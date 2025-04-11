#include <gpiod.h>
#include <iostream>
#include <unistd.h>

#define CHIP_NAME "gpiochip0"
#define LINE_NUM 17  // BCM GPIO 17

int main() {
    gpiod_chip *chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip) {
        std::cerr << "cannot open GPIO : " << CHIP_NAME << std::endl;
        return 1;
    }

    gpiod_line *line = gpiod_chip_get_line(chip, LINE_NUM);
    if (!line) {
        std::cerr << "cannot obtain GPIO id: " << LINE_NUM << std::endl;
        gpiod_chip_close(chip);
        return 1;
    }

    // require input_mode and using pull_up resister
    if (gpiod_line_request_input_flags(line, "switch_monitor", GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP) < 0) {
        std::cerr << " GPIO input_mode failed" << std::endl;
        gpiod_chip_close(chip);
        return 1;
    }

    std::cout << "monitoring GPIO17, press Ctrl+C to exit." << std::endl;

    while (true) {
        int value = gpiod_line_get_value(line);
        if (value == 0) {
            std::cout << "switch on！" << std::endl;
        } else {
            std::cout << "switch off" << std::endl;
        }
        usleep(200000); // 200 ms
    }

    // clean
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 0;
}
