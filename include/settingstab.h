#ifndef SETTINGSTAB_H
#define SETTINGSTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class SettingsTab;
}
QT_END_NAMESPACE

class DatabaseManager;

class SettingsTab : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsTab(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~SettingsTab();

    void initialize();
    void refreshData();

private:
    Ui::SettingsTab *ui;
    DatabaseManager *m_dbManager;
};

#endif // SETTINGSTAB_H