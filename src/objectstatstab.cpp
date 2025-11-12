#include "objectstatstab.h"
#include "ui_objectstatstab.h"
#include "databasemanager.h"

ObjectStatsTab::ObjectStatsTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::ObjectStatsTab), m_dbManager(dbManager)
{
    ui->setupUi(this);
}

ObjectStatsTab::~ObjectStatsTab()
{
    delete ui;
}

void ObjectStatsTab::initialize()
{
    // Initialize the tab UI and data
    refreshData();
}

void ObjectStatsTab::refreshData()
{
    // Refresh data from database
    // To be implemented
}