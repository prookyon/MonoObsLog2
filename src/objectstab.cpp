#include "objectstab.h"
#include "ui_objectstab.h"
#include "databasemanager.h"

ObjectsTab::ObjectsTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::ObjectsTab), m_dbManager(dbManager)
{
    ui->setupUi(this);
}

ObjectsTab::~ObjectsTab()
{
    delete ui;
}

void ObjectsTab::initialize()
{
    // Initialize the tab UI and data
    refreshData();
}

void ObjectsTab::refreshData()
{
    // Refresh data from database
    // To be implemented
}