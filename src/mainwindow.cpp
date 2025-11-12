#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "databasemanager.h"
#include "objectstab.h"
#include "sessionstab.h"
#include "camerastab.h"
#include "filtertypestab.h"
#include "filterstab.h"
#include "telescopestab.h"
#include "observationstab.h"
#include "objectstatstab.h"
#include "monthlystatstab.h"
#include "settingstab.h"
#include "abouttab.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_dbManager(std::make_unique<DatabaseManager>(this))
{
    ui->setupUi(this);

    // Initialize database
    if (!m_dbManager->initialize())
    {
        QMessageBox::critical(this, "Database Error",
                              "Failed to initialize database. Application may not function correctly.");
    }

    // Initialize all tabs
    initializeTabs();

    // Setup connections
    setupConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeTabs()
{
    // Create tab instances
    m_objectsTab = std::make_unique<ObjectsTab>(m_dbManager.get(), ui->objectsTab);
    m_sessionsTab = std::make_unique<SessionsTab>(m_dbManager.get(), ui->sessionsTab);
    m_camerasTab = std::make_unique<CamerasTab>(m_dbManager.get(), ui->camerasTab);
    m_filterTypesTab = std::make_unique<FilterTypesTab>(m_dbManager.get(), ui->filterTypesTab);
    m_filtersTab = std::make_unique<FiltersTab>(m_dbManager.get(), ui->filtersTab);
    m_telescopesTab = std::make_unique<TelescopesTab>(m_dbManager.get(), ui->telescopesTab);
    m_observationsTab = std::make_unique<ObservationsTab>(m_dbManager.get(), ui->observationsTab);
    m_objectStatsTab = std::make_unique<ObjectStatsTab>(m_dbManager.get(), ui->objectStatsTab);
    m_monthlyStatsTab = std::make_unique<MonthlyStatsTab>(m_dbManager.get(), ui->monthlyStatsTab);
    m_settingsTab = std::make_unique<SettingsTab>(m_dbManager.get(), ui->settingsTab);
    m_aboutTab = std::make_unique<AboutTab>(m_dbManager.get(), ui->aboutTab);

    // Initialize each tab
    m_objectsTab->initialize();
    m_sessionsTab->initialize();
    m_camerasTab->initialize();
    m_filterTypesTab->initialize();
    m_filtersTab->initialize();
    m_telescopesTab->initialize();
    m_observationsTab->initialize();
    m_objectStatsTab->initialize();
    m_monthlyStatsTab->initialize();
    m_settingsTab->initialize();
    m_aboutTab->initialize();
}

void MainWindow::setupConnections()
{
    // Connect database signals if needed
    connect(m_dbManager.get(), &DatabaseManager::errorOccurred, this, [this](const QString &error)
            { QMessageBox::warning(this, "Database Error", error); });
}