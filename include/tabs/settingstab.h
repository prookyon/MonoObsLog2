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
class SettingsManager;

class SettingsTab : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsTab(DatabaseManager *dbManager, SettingsManager *settingsManager, QWidget *parent = nullptr);
    ~SettingsTab();

    void initialize();
    void refreshData();

private slots:
    void onSaveButtonClicked();

private:
    void loadSettingsToUI();

    Ui::SettingsTab *ui;
    DatabaseManager *m_dbManager;
    SettingsManager *m_settingsManager;
};

#endif // SETTINGSTAB_H