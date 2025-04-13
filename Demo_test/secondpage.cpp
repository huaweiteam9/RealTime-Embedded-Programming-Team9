#include "secondpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>



void SecondPage::addRow(const QString &category, const QString &count,
                        const QString &putDate, const QString &expiryDate)
{
    // 获取当前行数
    int row = tableWidget->rowCount();
    // 插入一行
    tableWidget->insertRow(row);

    // 创建并设置每个单元格的内容
    tableWidget->setItem(row, 0, new QTableWidgetItem(category));
    tableWidget->setItem(row, 1, new QTableWidgetItem(count));
    tableWidget->setItem(row, 2, new QTableWidgetItem(putDate));
    tableWidget->setItem(row, 3, new QTableWidgetItem(expiryDate));
}

SecondPage::SecondPage(QWidget *parent) : QWidget(parent)
{
    if (parent) {
        setGeometry(parent->geometry());
    } else {
        resize(800, 600);
    }

    // Temperature label, initially showing "25 degC"
    temperatureLabel = new QLabel("25 ℃", this);
    QFont font = temperatureLabel->font();
    font.setPointSize(12);
    temperatureLabel->setFont(font);

    // Create a table with 4 columns
    tableWidget = new QTableWidget(0, 4, this);

    // Set the column headers
    QStringList headers;
    headers << "Category" << "Count" << "Put Date" << "Expiry Date";
    tableWidget->setHorizontalHeaderLabels(headers);

    // Make columns stretch to fill available width
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Layout
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addStretch();
    topLayout->addWidget(temperatureLabel);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(tableWidget);

    setLayout(mainLayout);
}
