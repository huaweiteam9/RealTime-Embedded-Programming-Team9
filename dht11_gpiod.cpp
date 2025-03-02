#include <iostream>
#include <vector>
#include <gpiod.h>
#include <unistd.h>
#include <chrono>

#define GPIO_CHIP "gpiochip0"  // Raspberry Pi's default GPIO controller
#define DHT_GPIO 17            // Using GPIO17 (physical pin 11)

// Function to read DHT11 data
bool readDHT(std::vector<int>& data) {
    struct gpiod_chip* chip;
    struct gpiod_line* line;
    int ret;

    // Open GPIO controller
    chip = gpiod_chip_open_by_name(GPIO_CHIP);
    if (!chip) {
        std::cerr << "Failed to open GPIO chip!" << std::endl;
        return false;
    }

    // Get GPIO line
    line = gpiod_chip_get_line(chip, DHT_GPIO);
    if (!line) {
        std::cerr << "Failed to get GPIO line!" << std::endl;
        gpiod_chip_close(chip);
        return false;
    }

    // Send start signal
    ret = gpiod_line_request_output(line, "dht11", 0);
    if (ret < 0) {
        std::cerr << "Failed to set output mode!" << std::endl;
        gpiod_chip_close(chip);
        return false;
    }

    // Pull low for 18ms
    gpiod_line_set_value(line, 0);
    usleep(18000);
    gpiod_line_set_value(line, 1);
    usleep(40);

    // Switch to input mode
    ret = gpiod_line_request_input(line, "dht11");
    if (ret < 0) {
        std::cerr << "Failed to set input mode!" << std::endl;
        gpiod_chip_close(chip);
        return false;
    }

    // Wait for sensor response
    auto start = std::chrono::steady_clock::now();
    while (gpiod_line_get_value(line) == 1) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::microseconds>(now - start).count() > 1000) {
            std::cerr << "Sensor response timeout!" << std::endl;
            gpiod_line_release(line);
            gpiod_chip_close(chip);
            return false;
        }
    }

    // Read 40 bits of data
    data.clear();
    for (int i = 0; i < 40; i++) {
        start = std::chrono::steady_clock::now();
        while (gpiod_line_get_value(line) == 0) {}  // Wait for high level start

        // Calculate high level duration
        auto t_start = std::chrono::steady_clock::now();
        while (gpiod_line_get_value(line) == 1) {}  // Wait for high level end
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - t_start
        ).count();

        data.push_back(duration > 30 ? 1 : 0);  // High level >30¦Ìs = logic 1
    }

    // Release resources
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return (data.size() == 40);
}

int main() {
    std::vector<int> data;
    if (!readDHT(data)) {
        std::cerr << "Failed to read data!" << std::endl;
        return 1;
    }

    // Parse data
    int humidity = 0, temp = 0, checksum = 0;
    for (int i = 0; i < 8; i++) {
        humidity = (humidity << 1) | data[i];
        temp = (temp << 1) | data[i + 16];
        checksum = (checksum << 1) | data[i + 32];
    }

    int sum = (humidity + temp + (data[8] << 8) + (data[24] << 8)) & 0xFF;
    if (sum != checksum) {
        std::cerr << "Checksum error!" << std::endl;
        return 1;
    }

    std::cout << "Humidity: " << humidity << "%\tTemperature: " << temp << "¡ãC" << std::endl;
    return 0;
}