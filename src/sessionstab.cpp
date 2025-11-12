#include "sessionstab.h"
#include "ui_sessionstab.h"
#include "databasemanager.h"

SessionsTab::SessionsTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::SessionsTab), m_dbManager(dbManager)
{
    ui->setupUi(this);
}

SessionsTab::~SessionsTab()
{
    delete ui;
}

void SessionsTab::initialize()
{
    // Initialize the tab UI and data
    refreshData();
}

void SessionsTab::refreshData()
{
    // Refresh data from database
    // To be implemented
}