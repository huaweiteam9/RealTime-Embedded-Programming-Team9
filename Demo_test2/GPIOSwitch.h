#ifndef GPIOSWITCH_H
#define GPIOSWITCH_H

#include <QObject>

class GPIOSwitch : public QObject
{
    Q_OBJECT
public:
    // 构造时传入要监控的 GPIO 线号（例如 17）
    explicit GPIOSwitch(int lineNum, QObject *parent = nullptr);
    ~GPIOSwitch();

    // 停止监控循环
    void stop();

public slots:
    // 监控主循环：打开 GPIO 芯片、获取指定线路，轮询状态变化
    void process();

signals:
    // 当开关状态改变时发出信号；参数 true 表示开（冰箱门打开），false 表示关
    void fridgeStateChanged(bool open);

private:
    int m_lineNum;
    bool m_running;
};

#endif // GPIOSWITCH_H
