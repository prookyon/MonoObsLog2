#ifndef FILTERSTAB_H
#define FILTERSTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class FiltersTab;
}
QT_END_NAMESPACE

class DatabaseManager;

class FiltersTab : public QWidget
{
    Q_OBJECT

public:
    explicit FiltersTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~FiltersTab();

    void initialize();
    void refreshData();

private:
    Ui::FiltersTab *ui;
    DatabaseManager *m_dbManager;
};

#endif // FILTERSTAB_H