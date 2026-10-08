#include <iostream>
#include <iomanip>
#include <csignal>
#include <cstdlib>

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <sqlite3.h>


// ================================================================
// Configuration
// ================================================================

constexpr int ADS7830_ADDRESS = 0x4B;
constexpr int ADC_CHANNEL = 0;

constexpr double VCC = 3.3;
constexpr double ADC_MAX = 255.0;

constexpr double POT_RESISTANCE = 10000.0;  // 10 kΩ

constexpr int SAMPLE_DELAY_MS = 100;


// ================================================================
// Globals
// ================================================================

int adcFd = -1;

sqlite3* db = nullptr;
sqlite3_stmt* insertStatement = nullptr;

volatile sig_atomic_t running = 1;


// ================================================================
// Ctrl+C handler
// ================================================================

void handleSignal(int signal)
{
    if (signal == SIGINT)
    {
        running = 0;
    }
}


// ================================================================
// ADS7830
// ================================================================

int readADS7830(int channel)
{
    if (channel < 0 || channel > 7)
    {
        return -1;
    }


    /*
        ADS7830 command byte:

        0x84 = single-ended mode
               + internal reference / ADC configuration

        Freenove's channel-selection formula:

        0x84 | (((channel << 2 | channel >> 1) & 0x07) << 4)
    */

    int command =
        0x84 |
        (((channel << 2 | channel >> 1) & 0x07) << 4);


    // Send command and read ADC result
    int value =
        wiringPiI2CReadReg8(
            adcFd,
            command
        );


    // WiringPi returns negative values on error
    if (value < 0)
    {
        return -1;
    }


    return value & 0xFF;
}


// ================================================================
// SQLite database setup
// ================================================================

bool setupDatabase()
{
    int rc =
        sqlite3_open(
            "sensor_log.db",
            &db
        );


    if (rc != SQLITE_OK)
    {
        std::cerr
            << "Failed to open SQLite database: "
            << sqlite3_errmsg(db)
            << '\n';

        return false;
    }


    const char* sql = R"SQL(

        CREATE TABLE IF NOT EXISTS potentiometer_log
        (
            id INTEGER PRIMARY KEY AUTOINCREMENT,

            timestamp DATETIME
                DEFAULT CURRENT_TIMESTAMP,

            adc_value INTEGER NOT NULL,

            voltage REAL NOT NULL,

            resistance REAL NOT NULL,

            current REAL NOT NULL
        );

    )SQL";


    char* errorMessage = nullptr;


    rc =
        sqlite3_exec(
            db,
            sql,
            nullptr,
            nullptr,
            &errorMessage
        );


    if (rc != SQLITE_OK)
    {
        std::cerr
            << "SQLite error creating table: "
            << errorMessage
            << '\n';

        sqlite3_free(errorMessage);

        return false;
    }


    return true;
}


// ================================================================
// Prepare INSERT statement
// ================================================================

bool prepareInsert()
{
    const char* sql = R"SQL(

        INSERT INTO potentiometer_log
        (
            adc_value,
            voltage,
            resistance,
            current
        )
        VALUES
        (
            ?,
            ?,
            ?,
            ?
        );

    )SQL";


    int rc =
        sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &insertStatement,
            nullptr
        );


    if (rc != SQLITE_OK)
    {
        std::cerr
            << "Failed to prepare INSERT statement: "
            << sqlite3_errmsg(db)
            << '\n';

        return false;
    }


    return true;
}


// ================================================================
// Insert one measurement
// ================================================================

bool logMeasurement(
    int adcValue,
    double voltage,
    double resistance,
    double current
)
{
    sqlite3_reset(insertStatement);

    sqlite3_clear_bindings(insertStatement);


    sqlite3_bind_int(
        insertStatement,
        1,
        adcValue
    );


    sqlite3_bind_double(
        insertStatement,
        2,
        voltage
    );


    sqlite3_bind_double(
        insertStatement,
        3,
        resistance
    );


    sqlite3_bind_double(
        insertStatement,
        4,
        current
    );


    int rc =
        sqlite3_step(
            insertStatement
        );


    if (rc != SQLITE_DONE)
    {
        std::cerr
            << "SQLite INSERT failed: "
            << sqlite3_errmsg(db)
            << '\n';

        return false;
    }


    return true;
}


// ================================================================
// Cleanup
// ================================================================

void cleanup()
{
    if (insertStatement != nullptr)
    {
        sqlite3_finalize(
            insertStatement
        );

        insertStatement = nullptr;
    }


    if (db != nullptr)
    {
        sqlite3_close(db);
        db = nullptr;
    }


    if (adcFd >= 0)
    {
        close(adcFd);
        adcFd = -1;
    }
}


// ================================================================
// Main
// ================================================================

int main()
{
    std::cout
        << "ADS7830 Potentiometer Logger\n"
        << "============================\n";


    // ------------------------------------------------------------
    // Ctrl+C
    // ------------------------------------------------------------

    std::signal(
        SIGINT,
        handleSignal
    );


    // ------------------------------------------------------------
    // WiringPi
    // ------------------------------------------------------------

    if (wiringPiSetup() == -1)
    {
        std::cerr
            << "Failed to initialize WiringPi.\n";

        return EXIT_FAILURE;
    }


    // ------------------------------------------------------------
    // ADS7830
    // ------------------------------------------------------------

    adcFd =
        wiringPiI2CSetup(
            ADS7830_ADDRESS
        );


    if (adcFd < 0)
    {
        std::cerr
            << "Failed to connect to ADS7830 "
            << "at address 0x4B.\n"
            << "Check:\n"
            << "  - I2C is enabled\n"
            << "  - ADS7830 is connected\n"
            << "  - i2cdetect -y 1 shows 0x4B\n";

        return EXIT_FAILURE;
    }


    std::cout
        << "ADS7830 connected at 0x4B\n";


    // ------------------------------------------------------------
    // SQLite
    // ------------------------------------------------------------

    if (!setupDatabase())
    {
        cleanup();
        return EXIT_FAILURE;
    }


    if (!prepareInsert())
    {
        cleanup();
        return EXIT_FAILURE;
    }


    std::cout
        << "SQLite database: sensor_log.db\n";


    // ------------------------------------------------------------
    // Potentiometer current
    //
    // The entire 10k potentiometer is across 3.3 V.
    //
    // I = V / R
    //
    // I = 3.3 / 10000
    //   = 0.00033 A
    //   = 0.330 mA
    // ------------------------------------------------------------

    const double current =
        VCC / POT_RESISTANCE;


    std::cout
        << std::fixed
        << std::setprecision(3);


    std::cout
        << "Potentiometer current: "
        << current * 1000.0
        << " mA\n";


    std::cout
        << "\nLogging channel A0...\n"
        << "Press Ctrl+C to stop.\n\n";


    // ------------------------------------------------------------
    // Main loop
    // ------------------------------------------------------------

    while (running)
    {
        // --------------------------------------------------------
        // Read ADS7830
        // --------------------------------------------------------

        int adcValue =
            readADS7830(
                ADC_CHANNEL
            );


        if (adcValue < 0)
        {
            std::cerr
                << "ADS7830 read failed.\n";

            break;
        }


        // --------------------------------------------------------
        // ADC → voltage
        // --------------------------------------------------------

        double voltage =
            static_cast<double>(adcValue)
            / ADC_MAX
            * VCC;


        // --------------------------------------------------------
        // Voltage → potentiometer resistance
        //
        // R = Rpot × V / VCC
        // --------------------------------------------------------

        double resistance =
            POT_RESISTANCE
            * voltage
            / VCC;


        // --------------------------------------------------------
        // Display
        // --------------------------------------------------------

        std::cout
            << "ADC: "
            << std::setw(3)
            << adcValue

            << " | Voltage: "
            << std::setw(5)
            << voltage
            << " V"

            << " | Resistance: "
            << std::setw(7)
            << resistance
            << " Ω"

            << " | Current: "
            << std::fixed
            << std::setprecision(3)
            << current * 1000.0
            << " mA"

            << '\n';


        // --------------------------------------------------------
        // SQLite
        // --------------------------------------------------------

        if (!logMeasurement(
                adcValue,
                voltage,
                resistance,
                current
            ))
        {
            break;
        }


        // --------------------------------------------------------
        // Wait 100 ms
        // --------------------------------------------------------

        delay(
            SAMPLE_DELAY_MS
        );
    }


    // ------------------------------------------------------------
    // Cleanup
    // ------------------------------------------------------------

    std::cout
        << "\nStopping logger...\n";


    cleanup();


    std::cout
        << "Database closed.\n";


    return EXIT_SUCCESS;
}