#include "telescopestab.h"
#include "ui_telescopestab.h"
#include "databasemanager.h"

TelescopesTab::TelescopesTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::TelescopesTab), m_dbManager(dbManager)
{
    ui->setupUi(this);
}

TelescopesTab::~TelescopesTab()
{
    delete ui;
}

void TelescopesTab::initialize()
{
    // Initialize the tab UI and data
    refreshData();
}

void TelescopesTab::refreshData()
{
    // Refresh data from database
    // To be implemented
}