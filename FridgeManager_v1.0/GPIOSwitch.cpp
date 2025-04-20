#include "GPIOSwitch.h"
#include <gpiod.h>
#include <QDebug>
#include <unistd.h>

#define CHIP_NAME "gpiochip0"

GPIOSwitch::GPIOSwitch(int lineNum, QObject *parent)
    : QObject(parent), m_lineNum(lineNum), m_running(true)
{
}

GPIOSwitch::~GPIOSwitch()
{
}

void GPIOSwitch::stop()
{
    m_running = false;
}

void GPIOSwitch::process()
{
    gpiod_chip *chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip) {
        qCritical() << "Cannot open GPIO chip:" << CHIP_NAME;
        return;
    }
    gpiod_line *line = gpiod_chip_get_line(chip, m_lineNum);
    if (!line) {
        qCritical() << "Cannot obtain GPIO line:" << m_lineNum;
        gpiod_chip_close(chip);
        return;
    }
    // Request input mode with pull-up resistor (bias pull-up)
    if (gpiod_line_request_input_flags(line, "GPIOSwitch", GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP) < 0) {
        qCritical() << "Failed to request GPIO input mode for line" << m_lineNum;
        gpiod_chip_close(chip);
        return;
    }
    int prev_value = gpiod_line_get_value(line);
    qDebug() << "Initial fridge state:" << (prev_value == 1 ? "Open" : "Closed");

    while(m_running) {
        int value = gpiod_line_get_value(line);
        if (value != prev_value) {
            bool open = (value == 1);
            qDebug() << "Fridge state changed to:" << (open ? "Open" : "Closed");
            emit fridgeStateChanged(open);
            prev_value = value;
        }
        usleep(200000);  // heck interval 200ms
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
}
