#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsmanager.h"
#include "db/databasemanager.h"
#include "tabs/objectstab.h"
#include "tabs/sessionstab.h"
#include "tabs/camerastab.h"
#include "tabs/filtertypestab.h"
#include "tabs/filterstab.h"
#include "tabs/telescopestab.h"
#include "tabs/observationstab.h"
#include "tabs/objectstatstab.h"
#include "tabs/monthlystatstab.h"
#include "tabs/settingstab.h"
#include "tabs/abouttab.h"
#include <QMessageBox>
#include <QVBoxLayout>

MainWindow::MainWindow(const QString &dbPath, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      m_dbManager(std::make_unique<DatabaseManager>(this)),
      m_settingsManager(std::make_unique<SettingsManager>(this))
{
    ui->setupUi(this);

    // Initialize database with the provided path
    if (!m_dbManager->initialize(dbPath))
    {
        QMessageBox::critical(this, "Database Error",
                              "Failed to initialize database. Application may not function correctly.");
    }

    // Initialize settings
    if (!m_settingsManager->initialize())
    {
        QMessageBox::critical(this, "Settings Error",
                              "Failed to initialize settings. Application may not function correctly.");
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
    // Create tab widgets and add them to tabWidget
    auto *objectsTabWidget = new QWidget();
    // ReSharper disable once CppDFAMemoryLeak
    auto *objectsLayout = new QVBoxLayout(objectsTabWidget);
    objectsTabWidget->setLayout(objectsLayout);
    objectsLayout->setContentsMargins(0, 0, 0, 0);
    ui->tabWidget->addTab(objectsTabWidget, "Objects");

    auto *sessionsTabWidget = new QWidget();
    // ReSharper disable once CppDFAMemoryLeak
    auto *sessionsLayout = new QVBoxLayout(sessionsTabWidget);
    sessionsTabWidget->setLayout(sessionsLayout);
    sessionsLayout->setContentsMargins(0, 0, 0, 0);
    ui->tabWidget->addTab(sessionsTabWidget, "Sessions");

    auto *camerasTabWidget = new QWidget();
    // ReSharper disable once CppDFAMemoryLeak
    auto *camerasLayout = new QVBoxLayout(camerasTabWidget);
    camerasTabWidget->setLayout(camerasLayout);
    camerasLayout->setContentsMargins(0, 0, 0, 0);
    ui->tabWidget->addTab(camerasTabWidget, "Cameras");

    auto *filterTypesTabWidget = new QWidget();
    // ReSharper disable once CppDFAMemoryLeak
    auto *filterTypesLayout = new QVBoxLayout(filterTypesTabWidget);
    filterTypesTabWidget->setLayout(filterTypesLayout);
    filterTypesLayout->setContentsMargins(0, 0, 0, 0);
    ui->tabWidget->addTab(filterTypesTabWidget, "Filter Types");

    auto *filtersTabWidget = new QWidget();
    // ReSharper disable once CppDFAMemoryLeak
    auto *filtersLayout = new QVBoxLayout(filtersTabWidget);
    filtersTabWidget->setLayout(filtersLayout);
    filtersLayout->setContentsMargins(0, 0, 0, 0);
    ui->tabWidget->addTab(filtersTabWidget, "Filters");

    auto *telescopesTabWidget = new QWidget();
    // ReSharper disable once CppDFAMemoryLeak
    auto *telescopesLayout = new QVBoxLayout(telescopesTabWidget);
    telescopesTabWidget->setLayout(telescopesLayout);
    telescopesLayout->setContentsMargins(0, 0, 0, 0);
    ui->tabWidget->addTab(telescopesTabWidget, "Telescopes");

    auto *observationsTabWidget = new QWidget();
    // ReSharper disable once CppDFAMemoryLeak
    auto *observationsLayout = new QVBoxLayout(observationsTabWidget);
    observationsTabWidget->setLayout(observationsLayout);
    observationsLayout->setContentsMargins(0, 0, 0, 0);
    ui->tabWidget->addTab(observationsTabWidget, "Observations");

    auto *objectStatsTabWidget = new QWidget();
    // ReSharper disable once CppDFAMemoryLeak
    auto *objectStatsLayout = new QVBoxLayout(objectStatsTabWidget);
    objectStatsTabWidget->setLayout(objectStatsLayout);
    objectStatsLayout->setContentsMargins(0, 0, 0, 0);
    ui->tabWidget->addTab(objectStatsTabWidget, "Object Stats");

    auto *monthlyStatsTabWidget = new QWidget();
    // ReSharper disable once CppDFAMemoryLeak
    auto *monthlyStatsLayout = new QVBoxLayout(monthlyStatsTabWidget);
    monthlyStatsTabWidget->setLayout(monthlyStatsLayout);
    monthlyStatsLayout->setContentsMargins(0, 0, 0, 0);
    ui->tabWidget->addTab(monthlyStatsTabWidget, "Monthly Stats");

    auto *settingsTabWidget = new QWidget();
    // ReSharper disable once CppDFAMemoryLeak
    auto *settingsLayout = new QVBoxLayout(settingsTabWidget);
    settingsTabWidget->setLayout(settingsLayout);
    settingsLayout->setContentsMargins(0, 0, 0, 0);
    ui->tabWidget->addTab(settingsTabWidget, "Settings");

    auto *aboutTabWidget = new QWidget();
    // ReSharper disable once CppDFAMemoryLeak
    auto *aboutLayout = new QVBoxLayout(aboutTabWidget);
    aboutTabWidget->setLayout(aboutLayout);
    aboutLayout->setContentsMargins(0, 0, 0, 0);
    ui->tabWidget->addTab(aboutTabWidget, "About");

    // Create tab instances with the newly created widgets
    m_objectsTab = std::make_unique<ObjectsTab>(m_dbManager.get(), m_settingsManager.get(), objectsTabWidget);
    objectsLayout->addWidget(m_objectsTab.get());

    m_sessionsTab = std::make_unique<SessionsTab>(m_dbManager.get(), m_settingsManager.get(), sessionsTabWidget);
    sessionsLayout->addWidget(m_sessionsTab.get());

    m_camerasTab = std::make_unique<CamerasTab>(m_dbManager.get(), camerasTabWidget);
    camerasLayout->addWidget(m_camerasTab.get());

    m_filterTypesTab = std::make_unique<FilterTypesTab>(m_dbManager.get(), filterTypesTabWidget);
    filterTypesLayout->addWidget(m_filterTypesTab.get());

    m_filtersTab = std::make_unique<FiltersTab>(m_dbManager.get(), filtersTabWidget);
    filtersLayout->addWidget(m_filtersTab.get());

    m_telescopesTab = std::make_unique<TelescopesTab>(m_dbManager.get(), telescopesTabWidget);
    telescopesLayout->addWidget(m_telescopesTab.get());

    m_observationsTab = std::make_unique<ObservationsTab>(m_dbManager.get(), m_settingsManager.get(), observationsTabWidget);
    observationsLayout->addWidget(m_observationsTab.get());

    m_objectStatsTab = std::make_unique<ObjectStatsTab>(m_dbManager.get(), objectStatsTabWidget);
    objectStatsLayout->addWidget(m_objectStatsTab.get());

    m_monthlyStatsTab = std::make_unique<MonthlyStatsTab>(m_dbManager.get(), monthlyStatsTabWidget);
    monthlyStatsLayout->addWidget(m_monthlyStatsTab.get());

    m_settingsTab = std::make_unique<SettingsTab>(m_dbManager.get(), m_settingsManager.get(), settingsTabWidget);
    settingsLayout->addWidget(m_settingsTab.get());

    m_aboutTab = std::make_unique<AboutTab>(m_dbManager.get(), aboutTabWidget);
    aboutLayout->addWidget(m_aboutTab.get());

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

    // Connect tab switching to refresh data
    connect(ui->tabWidget, QOverload<int>::of(&QTabWidget::currentChanged), this, [this](const int index)
            {
        // Refresh Sessions tab when it's activated (index 1)
        if (index == 1 && m_sessionsTab)
        {
            m_sessionsTab->refreshData();
        }
        // Refresh Filters tab when it's activated (index 4)
        if (index == 4 && m_filtersTab)
        {
            m_filtersTab->refreshData();
        }
        // Refresh Observations tab when it's activated (index 6)
        else if (index == 6 && m_observationsTab)
        {
            m_observationsTab->refreshData();
        }
        // Refresh Object Stats tab when it's activated (index 7)
        else if (index == 7 && m_objectStatsTab)
        {
            m_objectStatsTab->refreshData();
        }
        // Refresh Monthly Stats tab when it's activated (index 8)
        else if (index == 8 && m_monthlyStatsTab)
        {
            m_monthlyStatsTab->refreshData();
        } });
}