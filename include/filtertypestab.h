#ifndef FILTERTYPESTAB_H
#define FILTERTYPESTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class FilterTypesTab;
}
QT_END_NAMESPACE

class DatabaseManager;

class FilterTypesTab : public QWidget
{
    Q_OBJECT

public:
    explicit FilterTypesTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~FilterTypesTab();

    void initialize();
    void refreshData();

private:
    Ui::FilterTypesTab *ui;
    DatabaseManager *m_dbManager;
};

#endif // FILTERTYPESTAB_H