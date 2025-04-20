#include "GPIOSwitch.h"
#include <gpiod.h>
#include <QDebug>

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

    // Request both rising and falling edge events (door open and close)
    if (gpiod_line_request_both_edges_events(line, "GPIOSwitch") < 0) {
        qCritical() << "Failed to request edge events for GPIO line";
        gpiod_chip_close(chip);
        return;
    }

    qDebug() << "GPIOSwitch monitoring started on GPIO line" << m_lineNum;

    while (m_running) {
        struct timespec timeout = { 1, 0 };  // 1 second timeout to allow m_running check
        int ret = gpiod_line_event_wait(line, &timeout);

        if (ret < 0) {
            qCritical() << "Error waiting for GPIO event";
            break;
        } else if (ret == 0) {
            continue;  // timeout, re-check m_running
        }

        struct gpiod_line_event event;
        if (gpiod_line_event_read(line, &event) < 0) {
            qCritical() << "Failed to read GPIO event";
            break;
        }

        int value = gpiod_line_get_value(line);
        if (value < 0) {
            qCritical() << "Failed to get GPIO value";
            break;
        }

        // Emit signal when state changes
        bool isOpen = (value == 1);
        emit fridgeStateChanged(isOpen);
        qDebug() << "[GPIO] Door state changed:" << (isOpen ? "Open" : "Closed");
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
}