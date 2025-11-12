#ifndef OBJECTSTAB_H
#define OBJECTSTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ObjectsTab;
}
QT_END_NAMESPACE

class DatabaseManager;

class ObjectsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectsTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~ObjectsTab();

    void initialize();
    void refreshData();

private:
    Ui::ObjectsTab *ui;
    DatabaseManager *m_dbManager;
};

#endif // OBJECTSTAB_H