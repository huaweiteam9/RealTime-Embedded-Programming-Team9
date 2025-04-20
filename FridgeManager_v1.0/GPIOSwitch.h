#ifndef GPIOSWITCH_H
#define GPIOSWITCH_H

#include <QObject>

class GPIOSwitch : public QObject
{
    Q_OBJECT
public:
    // Pass in the GPIO line number to be monitored during construction
    explicit GPIOSwitch(int lineNum, QObject *parent = nullptr);
    ~GPIOSwitch();

    //Stop monitoring cycle 
    void stop();

public slots:
    // Monitor main loop: turn on GPIO chip, fetch specified line, poll for status changes
    void process();

signals:
    // Signal when the switch state changes; parameter true means on (refrigerator door open), false means off
    void fridgeStateChanged(bool open);

private:
    int m_lineNum;
    bool m_running;
};

#endif // GPIOSWITCH_H
