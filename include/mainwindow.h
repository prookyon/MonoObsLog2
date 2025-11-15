#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class DatabaseManager;
class SettingsManager;
class ObjectsTab;
class SessionsTab;
class CamerasTab;
class FilterTypesTab;
class FiltersTab;
class TelescopesTab;
class ObservationsTab;
class ObjectStatsTab;
class MonthlyStatsTab;
class SettingsTab;
class AboutTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initializeTabs();
    void setupConnections();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<DatabaseManager> m_dbManager;
    std::unique_ptr<SettingsManager> m_settingsManager;

    // Tab managers
    std::unique_ptr<ObjectsTab> m_objectsTab;
    std::unique_ptr<SessionsTab> m_sessionsTab;
    std::unique_ptr<CamerasTab> m_camerasTab;
    std::unique_ptr<FilterTypesTab> m_filterTypesTab;
    std::unique_ptr<FiltersTab> m_filtersTab;
    std::unique_ptr<TelescopesTab> m_telescopesTab;
    std::unique_ptr<ObservationsTab> m_observationsTab;
    std::unique_ptr<ObjectStatsTab> m_objectStatsTab;
    std::unique_ptr<MonthlyStatsTab> m_monthlyStatsTab;
    std::unique_ptr<SettingsTab> m_settingsTab;
    std::unique_ptr<AboutTab> m_aboutTab;
};

#endif // MAINWINDOW_H