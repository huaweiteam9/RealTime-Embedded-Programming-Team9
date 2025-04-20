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

    // Add a row of data to the table, with the parameters corresponding to the contents of each column in turn.
    void addRow(const QString &category, const QString &count,
                const QString &putDate, const QString &expiryDate);
    // clear the form
    void clearTable();

signals:
         // Add page switching signals if required

public slots:
              // Add slot functions if needed

private:
    QLabel *temperatureLabel;  // Labels for displaying temperature
    QTableWidget *tableWidget; // For displaying tables

};


#endif // SECONDPAGE_H
