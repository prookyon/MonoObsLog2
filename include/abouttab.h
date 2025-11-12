#ifndef ABOUTTAB_H
#define ABOUTTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class AboutTab;
}
QT_END_NAMESPACE

class DatabaseManager;

class AboutTab : public QWidget
{
    Q_OBJECT

public:
    explicit AboutTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~AboutTab();

    void initialize();
    void refreshData();

private:
    Ui::AboutTab *ui;
    DatabaseManager *m_dbManager;
};

#endif // ABOUTTAB_H