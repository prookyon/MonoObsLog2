#ifndef OBSERVATIONSTAB_H
#define OBSERVATIONSTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ObservationsTab;
}
QT_END_NAMESPACE

class DatabaseManager;

class ObservationsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ObservationsTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~ObservationsTab();

    void initialize();
    void refreshData();

private:
    Ui::ObservationsTab *ui;
    DatabaseManager *m_dbManager;
};

#endif // OBSERVATIONSTAB_H