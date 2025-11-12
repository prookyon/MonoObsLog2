#include "monthlystatstab.h"
#include "ui_monthlystatstab.h"
#include "databasemanager.h"

MonthlyStatsTab::MonthlyStatsTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::MonthlyStatsTab), m_dbManager(dbManager)
{
    ui->setupUi(this);
}

MonthlyStatsTab::~MonthlyStatsTab()
{
    delete ui;
}

void MonthlyStatsTab::initialize()
{
    // Initialize the tab UI and data
    refreshData();
}

void MonthlyStatsTab::refreshData()
{
    // Refresh data from database
    // To be implemented
}