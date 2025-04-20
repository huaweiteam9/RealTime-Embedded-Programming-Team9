#include "secondpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>



void SecondPage::clearTable()
{
    tableWidget->setRowCount(0);
}


void SecondPage::addRow(const QString &category, const QString &count,
                        const QString &putDate, const QString &expiryDate)
{
    // Get current line number
    int row = tableWidget->rowCount();
    // Insert a line
    tableWidget->insertRow(row);

    // Create and set the contents of each cell
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
    temperatureLabel = new QLabel("", this);
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
