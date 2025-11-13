#include "camerastab.h"
#include "ui_cameras_tab.h"
#include "databasemanager.h"

CamerasTab::CamerasTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::CamerasTab), m_dbManager(dbManager)
{
    ui->setupUi(this);
}

CamerasTab::~CamerasTab()
{
    delete ui;
}

void CamerasTab::initialize()
{
    // Initialize the tab UI and data
    refreshData();
}

void CamerasTab::refreshData()
{
    // Refresh data from database
    // To be implemented
}