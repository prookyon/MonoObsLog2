#include "tabs/monthlystatstab.h"
#include "ui_monthlystats_tab.h"
#include "db/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMap>
#include <QDate>
#include <QMessageBox>
#include <QGraphicsLayout>
#include <QwtText>
#include <utility>

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

    const QString sql = R"(
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
        const int totalSeconds = query.value("total_seconds").toInt();
        const double totalHours = totalSeconds / 3600.0; // Convert seconds to hours
        monthlyData[month] = totalHours;
    }

    if (monthlyData.isEmpty())
    {
        // No data to display - show empty chart
        ui->chartView->setTitle("Monthly Cumulative Exposure Time");
        return;
    }

    // Prepare categories (month labels) and values
    QStringList categories;
    QVector<QPointF> data;
    int index = 0;
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

        data << QPointF(index++, hours);
    }

    // Create bar chart
    const auto barChart = new QwtPlotBarChart();
    barChart->setSamples(data);
    //barChart->setBrush(QColor(52, 152, 219)); // Nice blue color
    barChart->attach(ui->chartView);

    // Custom scale draw for X axis labels
    class MonthScaleDraw : public QwtScaleDraw {
    public:
        explicit MonthScaleDraw(QStringList labels) : m_labels(std::move(labels)) {
        }

        QwtText label(double v) const override {
            int i = qRound(v);
            if (i >= 0 && i < m_labels.size())
                return m_labels[i];
            return {};
        }

    private:
        QStringList m_labels;
    };

    const auto scaleDraw = new MonthScaleDraw(categories);
    scaleDraw->enableComponent( QwtScaleDraw::Backbone, false );
    scaleDraw->enableComponent( QwtScaleDraw::Ticks, false );

    ui->chartView->setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);
    ui->chartView->setAxisTitle(QwtPlot::xBottom, "Month");
    ui->chartView->setAxisTitle(QwtPlot::yLeft, "Exposure Time (hours)");

    // Set Y axis range with some padding
    double maxHours = 0;
    for (const double hours : monthlyData.values())
    {
        if (hours > maxHours)
            maxHours = hours;
    }
    ui->chartView->setAxisScale(QwtPlot::yLeft, 0, maxHours * 1.05);

    ui->chartView->setTitle("Monthly Cumulative Exposure Time");

    // Set margins
    ui->chartView->setContentsMargins(0, 0, 0, 0);
    ui->verticalLayout->setContentsMargins(0, 0, 0, 0);

    ui->chartView->replot();
}