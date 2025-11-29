#ifndef OBJECTSTATSTAB_H
#define OBJECTSTATSTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ObjectStatsTab;
}
QT_END_NAMESPACE

class DatabaseManager;

class ObjectStatsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectStatsTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~ObjectStatsTab() override;

    void initialize();
    void refreshData();

private:
    void applyConditionalFormatting(const QVector<double> &totals) const;

    Ui::ObjectStatsTab *ui;
    DatabaseManager *m_dbManager;
};

#endif // OBJECTSTATSTAB_H