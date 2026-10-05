#include <iostream>
#include <sstream>
#include <string>
#include <sqlite3.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

constexpr const char* DATABASE = "electronics_lab.db";
constexpr const char* SERIAL_PORT = "/dev/ttyUSB0";
constexpr speed_t BAUD_RATE = B9600;

int main()
{
    // ------------------------------------------------------------
    // Open SQLite database
    // ------------------------------------------------------------

    sqlite3* db = nullptr;

    if (sqlite3_open(DATABASE, &db) != SQLITE_OK)
    {
        std::cerr << "Cannot open database: "
                  << sqlite3_errmsg(db) << '\n';

        sqlite3_close(db);
        return 1;
    }

    const char* createTable = R"(
        CREATE TABLE IF NOT EXISTS sensor_readings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            seconds REAL NOT NULL,
            adc INTEGER NOT NULL,
            voltage REAL NOT NULL,
            current_mA REAL NOT NULL,
            resistance_ohms REAL NOT NULL,
            logged_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        );
    )";

    char* errorMessage = nullptr;

    if (sqlite3_exec(db, createTable, nullptr, nullptr, &errorMessage)
        != SQLITE_OK)
    {
        std::cerr << "Failed to create table: "
                  << errorMessage << '\n';

        sqlite3_free(errorMessage);
        sqlite3_close(db);
        return 1;
    }

    // ------------------------------------------------------------
    // Open Arduino serial port
    // ------------------------------------------------------------

    int serial = open(SERIAL_PORT, O_RDONLY | O_NOCTTY);

    if (serial < 0)
    {
        std::cerr << "Could not open " << SERIAL_PORT << '\n';
        sqlite3_close(db);
        return 1;
    }

    termios tty{};

    if (tcgetattr(serial, &tty) != 0)
    {
        std::cerr << "Could not configure serial port\n";

        close(serial);
        sqlite3_close(db);
        return 1;
    }

    cfsetispeed(&tty, BAUD_RATE);
    cfsetospeed(&tty, BAUD_RATE);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_oflag &= ~OPOST;

    tcsetattr(serial, TCSANOW, &tty);

    // ------------------------------------------------------------
    // Prepare SQLite INSERT
    // ------------------------------------------------------------

    const char* insertSQL = R"(
        INSERT INTO sensor_readings
        (seconds, adc, voltage, current_mA, resistance_ohms)
        VALUES (?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(
            db,
            insertSQL,
            -1,
            &statement,
            nullptr) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare INSERT statement\n";

        close(serial);
        sqlite3_close(db);
        return 1;
    }

    // ------------------------------------------------------------
    // Read Arduino continuously
    // ------------------------------------------------------------

    std::string buffer;
    char character;

    std::cout << "Logging Arduino data...\n";
    std::cout << "Press Ctrl+C to stop.\n";

    while (true)
    {
        ssize_t bytesRead = read(serial, &character, 1);

        if (bytesRead <= 0)
            continue;

        if (character == '\n')
        {
            // Remove carriage return if Arduino sends \r\n.
            if (!buffer.empty() && buffer.back() == '\r')
                buffer.pop_back();

            std::stringstream stream(buffer);

            std::string field;
            double seconds;
            int adc;
            double voltage;
            double current_mA;
            double resistance;

            try
            {
                std::getline(stream, field, ',');
                seconds = std::stod(field);

                std::getline(stream, field, ',');
                adc = std::stoi(field);

                std::getline(stream, field, ',');
                voltage = std::stod(field);

                std::getline(stream, field, ',');
                current_mA = std::stod(field);

                std::getline(stream, field, ',');
                resistance = std::stod(field);
            }
            catch (...)
            {
                // Ignore headers or malformed lines.
                buffer.clear();
                continue;
            }

            // ----------------------------------------------------
            // Insert into SQLite
            // ----------------------------------------------------

            sqlite3_reset(statement);
            sqlite3_clear_bindings(statement);

            sqlite3_bind_double(statement, 1, seconds);
            sqlite3_bind_int(statement, 2, adc);
            sqlite3_bind_double(statement, 3, voltage);
            sqlite3_bind_double(statement, 4, current_mA);
            sqlite3_bind_double(statement, 5, resistance);

            if (sqlite3_step(statement) == SQLITE_DONE)
            {
                std::cout
                    << "Saved: "
                    << seconds << " s, "
                    << adc << ", "
                    << voltage << " V, "
                    << current_mA << " mA, "
                    << resistance << " ohms\n";
            }
            else
            {
                std::cerr << "SQLite INSERT failed: "
                          << sqlite3_errmsg(db) << '\n';
            }

            buffer.clear();
        }
        else
        {
            buffer += character;
        }
    }

    // Technically unreachable without adding signal handling,
    // but kept for completeness.

    sqlite3_finalize(statement);
    close(serial);
    sqlite3_close(db);

    return 0;
}