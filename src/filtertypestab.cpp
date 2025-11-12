#include "filtertypestab.h"
#include "ui_filtertypestab.h"
#include "databasemanager.h"

FilterTypesTab::FilterTypesTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::FilterTypesTab), m_dbManager(dbManager)
{
    ui->setupUi(this);
}

FilterTypesTab::~FilterTypesTab()
{
    delete ui;
}

void FilterTypesTab::initialize()
{
    // Initialize the tab UI and data
    refreshData();
}

void FilterTypesTab::refreshData()
{
    // Refresh data from database
    // To be implemented
}