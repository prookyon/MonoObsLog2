#ifndef MONTHLYSTATSTAB_H
#define MONTHLYSTATSTAB_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MonthlyStatsTab;
}
QT_END_NAMESPACE

class DatabaseManager;

class MonthlyStatsTab : public QWidget
{
    Q_OBJECT

public:
    explicit MonthlyStatsTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~MonthlyStatsTab();

    void initialize();
    void refreshData();

private:
    void createChart();

    Ui::MonthlyStatsTab *ui;
    DatabaseManager *m_dbManager;
};

#endif // MONTHLYSTATSTAB_H