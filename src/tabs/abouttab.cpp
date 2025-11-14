#include "tabs/abouttab.h"
#include "ui_about_tab.h"
#include "db/databasemanager.h"

AboutTab::AboutTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::AboutTab), m_dbManager(dbManager)
{
    ui->setupUi(this);
}

AboutTab::~AboutTab()
{
    delete ui;
}

void AboutTab::initialize()
{
    // Initialize the tab UI and data
    refreshData();
}

void AboutTab::refreshData()
{
    // Refresh data from database
    // To be implemented
}