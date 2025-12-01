#ifndef MONTHLYSTATSTAB_H
#define MONTHLYSTATSTAB_H

#include <QWidget>
#include <qwt_plot.h>
#include <qwt_plot_barchart.h>
#include <qwt_scale_draw.h>

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
    ~MonthlyStatsTab() override;

    void initialize();
    void refreshData();

private:
    void createChart();

    Ui::MonthlyStatsTab *ui;
    DatabaseManager *m_dbManager;
};

#endif // MONTHLYSTATSTAB_H