void GPIOSwitch::process()
{
    gpiod_chip* chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip) {
        qCritical() << "Cannot open GPIO chip:" << CHIP_NAME;
        return;
    }

    gpiod_line* line = gpiod_chip_get_line(chip, m_lineNum);
    if (!line) {
        qCritical() << "Cannot obtain GPIO line:" << m_lineNum;
        gpiod_chip_close(chip);
        return;
    }

    // Request to listen for both rising and falling edge events (i.e. door open & close)
    if (gpiod_line_request_both_edges_events(line, "GPIOSwitch") < 0) {
        qCritical() << "Failed to request edge events for GPIO line";
        gpiod_chip_close(chip);
        return;
    }

    qDebug() << "GPIOSwitch event monitoring started on line" << m_lineNum;

    // Event loop: blocks until GPIO state changes
    while (m_running) {
        struct timespec timeout = { 1, 0 };  // 1 second timeout to allow m_running check
        int ret = gpiod_line_event_wait(line, &timeout);

        if (ret < 0) {
            qCritical() << "Error waiting for GPIO event";
            break;
        }
        else if (ret == 0) {
            continue;  // Timeout, check m_running again
        }

        struct gpiod_line_event event;
        if (gpiod_line_event_read(line, &event) < 0) {
            qCritical() << "Failed to read GPIO event";
            break;
        }

        // Read current value (1=open, 0=closed)
        int value = gpiod_line_get_value(line);
        if (value < 0) {
            qCritical() << "Failed to read GPIO value";
            break;
        }

        emit fridgeStateChanged(value == 1);
        qDebug() << "GPIO state changed, door is now:" << (value == 1 ? "Open" : "Closed");
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
}
