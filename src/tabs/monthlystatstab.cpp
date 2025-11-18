#include "tabs/monthlystatstab.h"
#include "ui_monthlystats_tab.h"
#include "db/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMap>
#include <QDate>
#include <QMessageBox>
#include <QGraphicsLayout>

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
    // Refresh data from database and recreate the chart
    createChart();
}

void MonthlyStatsTab::createChart()
{
    // Query database to get monthly exposure times
    QSqlQuery query(m_dbManager->database());

    QString sql = R"(
        SELECT 
            strftime('%Y-%m', s.start_date) AS month,
            SUM(o.total_exposure) AS total_seconds
        FROM observations o
        INNER JOIN sessions s ON o.session_id = s.id
        GROUP BY month
        ORDER BY month ASC
    )";

    if (!query.exec(sql))
    {
        QMessageBox::warning(this, "Error",
                             QString("Failed to query monthly stats: %1").arg(query.lastError().text()));
        return;
    }

    // Collect data: month -> total hours
    QMap<QString, double> monthlyData;
    while (query.next())
    {
        QString month = query.value("month").toString();
        int totalSeconds = query.value("total_seconds").toInt();
        double totalHours = totalSeconds / 3600.0; // Convert seconds to hours
        monthlyData[month] = totalHours;
    }

    if (monthlyData.isEmpty())
    {
        // No data to display - show empty chart
        QChart *chart = new QChart();
        chart->setTitle("Monthly Cumulative Exposure Time");
        ui->chartView->setChart(chart);
        ui->chartView->setRenderHint(QPainter::Antialiasing);
        return;
    }

    // Create bar set for the data
    QBarSet *barSet = new QBarSet("Exposure Hours");
    barSet->setColor(QColor(52, 152, 219)); // Nice blue color

    // Prepare categories (month labels) and values
    QStringList categories;
    for (auto it = monthlyData.constBegin(); it != monthlyData.constEnd(); ++it)
    {
        QString monthStr = it.key();
        double hours = it.value();

        // Format month as "MMM YYYY" (e.g., "Jan 2024")
        QDate date = QDate::fromString(monthStr + "-01", "yyyy-MM-dd");
        if (date.isValid())
        {
            categories << date.toString("MM/yyyy");
        }
        else
        {
            categories << monthStr;
        }

        *barSet << hours;
    }

    // Create bar series and add the bar set
    QBarSeries *series = new QBarSeries();
    series->append(barSet);

    // Create chart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Create X axis with month categories
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("Month");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Create Y axis for hours
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Exposure Time (hours)");
    axisY->setLabelFormat("%.0f");
    axisY->setTickInterval(10.0);

    // Set Y axis range with some padding
    double maxHours = 0;
    for (double hours : monthlyData.values())
    {
        if (hours > maxHours)
            maxHours = hours;
    }
    axisY->setRange(0, maxHours * 1.05); // Add padding at top

    chart->addAxis(axisY, Qt::AlignLeft);
    series->setLabelsFormat("@value h");
    series->setLabelsPrecision(3);
    series->setLabelsVisible(true);
    series->attachAxis(axisY);

    chart->legend()->setVisible(false);
    chart->setBackgroundRoundness(0);

    // Set chart to view
    ui->chartView->setFrameShape(QFrame::NoFrame);
    ui->chartView->setChart(chart);
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setBackgroundRoundness(0);
    ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
    ui->chartView->setRenderHint(QPainter::Antialiasing);
}