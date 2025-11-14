#ifndef MONTHLYSTATSTAB_H
#define MONTHLYSTATSTAB_H

#include <QWidget>

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
    Ui::MonthlyStatsTab *ui;
    DatabaseManager *m_dbManager;
};

#endif // MONTHLYSTATSTAB_H