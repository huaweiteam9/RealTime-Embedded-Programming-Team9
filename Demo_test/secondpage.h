#ifndef SECONDPAGE_H
#define SECONDPAGE_H

#include <QWidget>

class QLabel;
class QTableWidget;

class SecondPage : public QWidget
{
    Q_OBJECT
public:
    explicit SecondPage(QWidget *parent = nullptr);

public:
    // 添加一行数据到表格中，参数依次对应每列内容
    void addRow(const QString &category, const QString &count,
                const QString &putDate, const QString &expiryDate);


signals:
         // 如有需要，可添加页面切换信号

public slots:
              // 如有需要，可添加槽函数

private:
    QLabel *temperatureLabel;  // 用于显示温度的标签
    QTableWidget *tableWidget; // 用于显示表格

};


#endif // SECONDPAGE_H
