#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <sqlite3.h>
#include <string>

class I2C 
{
    int fd;
public:
    explicit I2C(int addr) {
        fd = wiringPiI2CSetup(addr);
    }
    ~I2C() {
        if (fd >= 0) close(fd);
    }
    int readByte() {
        return wiringPiI2CRead(fd);
    }
    int writeByte(uint8_t v) {
        return wiringPiI2CWrite(fd, v);
    }
    int readReg8(uint8_t reg) {
        return wiringPiI2CReadReg8(fd, reg);
    }
    bool isValid() const {
        return fd >= 0;
    }
};

// Initialize SQLite Database with WAL mode for multi-process safety
bool initDatabase(sqlite3* db) {
    // Enable Write-Ahead Logging so this Pi program and Arduino host_logger can write concurrently
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    const char* createTableSQL = 
        "CREATE TABLE IF NOT EXISTS adc_readings ("
        "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "   sensor_id TEXT NOT NULL,"
        "   raw_adc INTEGER NOT NULL,"
        "   filtered_val REAL NOT NULL,"
        "   voltage_mv REAL NOT NULL,"
        "   btn_event INTEGER NOT NULL"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "[DB ERROR] Table creation failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    std::string dbPath = (argc >= 2) ? argv[1] : "sensor_log.db";
    const char* SENSOR_ID = "PI500_ADS7830_CH0";
    const float ALPHA = 0.1f; // EMA smoothing factor

    // 1. Initialize WiringPi
    if (wiringPiSetup() == -1) {
        std::cerr << "[ERROR] WiringPi setup failed!" << std::endl;
        return 1;
    }

    // 2. Open ADS7830 I2C Device (Address 0x4B)
    I2C adc(0x4b);
    if (!adc.isValid()) {
        std::cerr << "[ERROR] Could not open ADS7830 at address 0x4B" << std::endl;
        return 1;
    }

    // 3. Open SQLite Database
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "[DB ERROR] Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    if (!initDatabase(db)) {
        sqlite3_close(db);
        return 1;
    }

    // 4. Prepare SQL Insert Statement
    const char* insertSQL = "INSERT INTO adc_readings (sensor_id, raw_adc, filtered_val, voltage_mv, btn_event) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DB ERROR] Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }

    std::cout << "[INFO] Logging ADS7830 (0x4B) readings to " << dbPath << "..." << std::endl;

    float emaFiltered = 0.0f;
    int sampleCount = 0;
    
    // Initial read to set baseline
    adc.writeByte(0x84); // Single-ended Channel 0 command
    usleep(1000);
    emaFiltered = static_cast<float>(adc.readByte());

    // Begin first transaction batch
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    while (true) {
        // --- Read ADS7830 Channel 0 ---
        adc.writeByte(0x84);
        int rawVal = adc.readByte();

        if (rawVal < 0) {
            std::cerr << "[WARN] I2C Read Error" << std::endl;
            usleep(1000000);
            continue;
        }

        // --- Exponential Moving Average (EMA) Filter ---
        emaFiltered = (ALPHA * rawVal) + ((1.0f - ALPHA) * emaFiltered);

        // --- Calculate Millivolts (8-bit resolution: 0 - 255 -> 0 - 3300 mV) ---
        float voltageMv = (emaFiltered * 3300.0f) / 255.0f;
        int btnEvent = 0; // Set to 1 if digital pin button event triggered

        // --- Bind Values to SQL Statement ---
        sqlite3_bind_text(stmt, 1, SENSOR_ID, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, rawVal);
        sqlite3_bind_double(stmt, 3, emaFiltered);
        sqlite3_bind_double(stmt, 4, voltageMv);
        sqlite3_bind_int(stmt, 5, btnEvent);

        // Execute SQL statement
        sqlite3_step(stmt);
        sqlite3_reset(stmt);

        sampleCount++;
        std::cout << "[Sample #" << sampleCount << "] Raw: " << rawVal 
                  << " | Filtered: " << emaFiltered 
                  << " | Volts: " << voltageMv << " mV" << std::endl;

        // Commit batch every 50 samples to prevent SD card wear
        if (sampleCount % 50 == 0) {
            sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
            sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
        }

        usleep(1000000); // 50 Hz sampling rate (20 ms delay)
    }

    // Cleanup
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}