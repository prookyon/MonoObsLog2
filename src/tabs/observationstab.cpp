#include "tabs/observationstab.h"
#include "ui_observations_tab.h"
#include "db/databasemanager.h"

ObservationsTab::ObservationsTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::ObservationsTab), m_dbManager(dbManager)
{
    ui->setupUi(this);
}

ObservationsTab::~ObservationsTab()
{
    delete ui;
}

void ObservationsTab::initialize()
{
    // Initialize the tab UI and data
    refreshData();
}

void ObservationsTab::refreshData()
{
    // Refresh data from database
    // To be implemented
}