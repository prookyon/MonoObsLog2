#ifndef TELESCOPESTAB_H
#define TELESCOPESTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class TelescopesTab;
}
QT_END_NAMESPACE

class DatabaseManager;

class TelescopesTab : public QWidget
{
    Q_OBJECT

public:
    explicit TelescopesTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~TelescopesTab();

    void initialize();
    void refreshData();

private:
    Ui::TelescopesTab *ui;
    DatabaseManager *m_dbManager;
};

#endif // TELESCOPESTAB_H