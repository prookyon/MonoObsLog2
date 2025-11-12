#include "settingstab.h"
#include "ui_settingstab.h"
#include "databasemanager.h"

SettingsTab::SettingsTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::SettingsTab), m_dbManager(dbManager)
{
    ui->setupUi(this);
}

SettingsTab::~SettingsTab()
{
    delete ui;
}

void SettingsTab::initialize()
{
    // Initialize the tab UI and data
    refreshData();
}

void SettingsTab::refreshData()
{
    // Refresh data from database
    // To be implemented
}