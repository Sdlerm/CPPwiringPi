// ============================================================================
// File: pi500_gpio_logger.cpp (Raspberry Pi 500 Direct C++ GPIO Logger)
// Target: Raspberry Pi 500 / Pi 5 / Pi 4 (Linux ARM64)
// Description: Reads analog potentiometer voltage dividers via SPI ADC (MCP3008)
//              or digital GPIOs directly on Pi 500 header pins, logging 
//              data directly into the shared 'sensor_log.db' SQLite database.
//
// Prerequisite: sudo apt install libsqlite3-dev   (provides sqlite3.h)
// Build: g++ -O2 pi500_gpio_logger.cpp -o pi500_gpio_logger -lsqlite3
// Usage: ./pi500_gpio_logger /dev/spidev0.0 sensor_log.db
// ============================================================================

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <cstdint>
#include <sqlite3.h> // Requires: sudo apt install libsqlite3-dev (then reload the editor/IntelliSense)

const char* SENSOR_ID = "PI500_POT_ADC";
const float ALPHA = 0.1f; // EMA smoothing factor

// Initialize SQLite database table with WAL mode for concurrent multi-logger access
bool initDatabase(sqlite3* db) {
    // Enable Write-Ahead Logging (WAL) so Arduino host_logger and Pi 500 logger write simultaneously
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

// Read Channel 0 from MCP3008 10-bit SPI ADC connected to Pi 500 SPI pins
int readMCP3008(int spiFd, int channel) {
    if (spiFd < 0) return -1;

    uint8_t tx[] = { 1, static_cast<uint8_t>((8 + channel) << 4), 0 };
    uint8_t rx[3] = { 0 };

    struct spi_ioc_transfer tr = {};
    tr.tx_buf = (unsigned long)tx;
    tr.rx_buf = (unsigned long)rx;
    tr.len = 3;
    tr.speed_hz = 1000000;
    tr.bits_per_word = 8; // Linux SPI API uses bits_per_word, not bits_per_byte

    if (ioctl(spiFd, SPI_IOC_MESSAGE(1), &tr) < 1) {
        return -1;
    }

    return ((rx[1] & 3) << 8) + rx[2]; // 10-bit value (0 - 1023)
}

int main(int argc, char* argv[]) {
    std::string spiDevice = "/dev/spidev0.0";
    std::string dbPath = "sensor_log.db";

    if (argc >= 2) spiDevice = argv[1];
    if (argc >= 3) dbPath = argv[2];

    std::cout << "[INFO] Starting Raspberry Pi 500 Direct GPIO/SPI C++ Logger..." << std::endl;
    std::cout << "[INFO] SPI Device: " << spiDevice << " | Database: " << dbPath << std::endl;

    // Open SQLite DB
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "[DB ERROR] Cannot open " << dbPath << ": " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    if (!initDatabase(db)) {
        sqlite3_close(db);
        return 1;
    }

    // Try opening SPI device
    int spiFd = open(spiDevice.c_str(), O_RDWR);
    bool useHardwareSpi = (spiFd >= 0);
    if (!useHardwareSpi) {
        std::cout << "[WARN] Could not open " << spiDevice << ". Running in simulated GPIO ADC mode for testing." << std::endl;
    }

    // Prepared SQL Statement
    const char* insertSQL = "INSERT INTO adc_readings (sensor_id, raw_adc, filtered_val, voltage_mv, btn_event) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);

    float emaFiltered = 512.0f;
    int sampleCount = 0;
    int simVal = 512;
    int simDir = 5;

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    while (true) {
        int rawAdc = 0;
        if (useHardwareSpi) {
            rawAdc = readMCP3008(spiFd, 0); // Read channel 0
            if (rawAdc < 0) rawAdc = 0;
        } else {
            // Simulation curve for Pi 500 demo
            simVal += simDir;
            if (simVal >= 900 || simVal <= 100) simDir = -simDir;
            rawAdc = simVal;
        }

        // Apply EMA Filter
        emaFiltered = (ALPHA * rawAdc) + ((1.0f - ALPHA) * emaFiltered);

        // Convert 10-bit reading to 3.3V (3300 mV) logic for Pi 500 GPIO/ADC
        float voltageMv = (emaFiltered * 3300.0f) / 1023.0f;
        int btnEvent = (sampleCount % 40 == 0) ? 1 : 0;

        // Bind SQL parameters
        sqlite3_bind_text(stmt, 1, SENSOR_ID, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, rawAdc);
        sqlite3_bind_double(stmt, 3, emaFiltered);
        sqlite3_bind_double(stmt, 4, voltageMv);
        sqlite3_bind_int(stmt, 5, btnEvent);

        sqlite3_step(stmt);
        sqlite3_reset(stmt);

        sampleCount++;
        std::cout << "[Pi500 Logged #" << sampleCount << "] " << SENSOR_ID 
                  << " -> Raw: " << rawAdc << " | Volts: " << voltageMv << " mV" << std::endl;

        if (sampleCount % 50 == 0) {
            sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
            sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // 50 Hz
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    if (spiFd >= 0) close(spiFd);
    return 0;
}
