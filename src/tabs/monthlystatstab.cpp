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
#include <QwtPlotMarker>
#include <qwt_column_symbol.h>
#include <qwt_scale_engine.h>
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
    ui->chartView->detachItems();

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

    // test for extreme amount of data
    /*for (int i = 1; i < 24; i++) {
        auto month = QDateTime::currentDateTime().addMonths(i);
        auto key = month.toString("yyyy-MM");
        auto value = static_cast<double>( rand()%100) + 0.1;
        monthlyData[key] = value;
    }*/

    if (monthlyData.isEmpty())
    {
        // No data to display - show empty chart
        ui->chartView->setTitle("Monthly Cumulative Exposure Time");
        return;
    }

    // Prepare categories (month labels) and values
    QStringList categories;
    QVector<QPointF> data;
    double maxHours = 0;
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
        if (hours > maxHours)
            maxHours = hours;
    }

    // Create bar chart
    const auto barChart = new QwtPlotBarChart();
    barChart->setSamples(data);
    const auto symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
    symbol->setPalette(QColor(63, 148, 224));
    symbol->setFrameStyle(QwtColumnSymbol::Plain);
    symbol->setLineWidth(0);
    barChart->setSymbol(symbol);
    barChart->attach(ui->chartView);

    // Add value labels on top of each bar
    for (auto i : data) {
        double x = i.x();
        double y = i.y();
        QString labelText = QString::number(y, 'f', 1);
        const auto marker = new QwtPlotMarker();
        marker->setLabel(labelText);
        marker->setLabelAlignment(Qt::AlignCenter);
        marker->setValue(x, y + 0.03 * maxHours);
        marker->attach(ui->chartView);
    }

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

    // Custom Scale Engine
    class MyScaleEngine : public QwtLinearScaleEngine {
      public:
        void autoScale(int maxNumSteps, double &x1, double &x2, double &stepSize) const override {
            if (x2 < 24)
                stepSize = 1.0;
            else if (x2 < 48)
                stepSize = 3.0;
            else
                stepSize = 6.0;
        };
    };

    const auto scaleDraw = new MonthScaleDraw(categories);
    scaleDraw->enableComponent( QwtScaleDraw::Backbone, false );
    //scaleDraw->enableComponent( QwtScaleDraw::Ticks, false );
    ui->chartView->setAxisMaxMinor(QwtPlot::xBottom, 0);
    ui->chartView->setAxisMaxMinor(QwtPlot::yLeft, 0);
    ui->chartView->setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);
    ui->chartView->setAxisTitle(QwtPlot::xBottom, "Month");
    ui->chartView->setAxisTitle(QwtPlot::yLeft, "Exposure Time (hours)");
    ui->chartView->canvas()->setContentsMargins(0,0,0,0);
    const auto scaleEngine = new MyScaleEngine();
    scaleEngine->setAttribute(QwtScaleEngine::Floating);
    ui->chartView->setAxisScaleEngine(QwtPlot::xBottom,scaleEngine);

    // Set Y axis range with some padding
    ui->chartView->setAxisScale(QwtPlot::yLeft, 0, maxHours * 1.05);

    ui->chartView->setTitle("Monthly Cumulative Exposure Time");

    // Set margins
    ui->chartView->setContentsMargins(0, 0, 20, 0);
    ui->verticalLayout->setContentsMargins(0, 0, 0, 0);

    ui->chartView->replot();
}