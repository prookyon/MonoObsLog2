#ifndef SESSIONSTAB_H
#define SESSIONSTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class SessionsTab;
}
QT_END_NAMESPACE

class DatabaseManager;

class SessionsTab : public QWidget
{
    Q_OBJECT

public:
    explicit SessionsTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~SessionsTab();

    void initialize();
    void refreshData();

private:
    Ui::SessionsTab *ui;
    DatabaseManager *m_dbManager;
};

#endif // SESSIONSTAB_H