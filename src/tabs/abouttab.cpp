#include "tabs/abouttab.h"
#include "ui_about_tab.h"
#include "db/databasemanager.h"

#include <QStringBuilder>

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
    ui->iconLabel->setPixmap(QPixmap(":/images/icon.png"));
    ui->linkLabel->setTextFormat(Qt::RichText);
    ui->linkLabel->setText("<a href=\"https://github.com/prookyon/MonoObsLog2\">GitHub Repository</a>");
    ui->linkLabel->setToolTip(QString("https://github.com/prookyon/MonoObsLog2"));
    QString version = QString("MonoObsLog version: %1\n© Rainer Ots 2025").arg(QApplication::instance()->applicationVersion());
    ui->versionLabel->setText(version);
    refreshData();
}

void AboutTab::refreshData()
{
    // Refresh data from database
    // To be implemented
}