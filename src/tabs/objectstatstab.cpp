#include "tabs/objectstatstab.h"
#include "ui_objectstats_tab.h"
#include "db/databasemanager.h"
#include "numerictablewidgetitem.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QMap>
#include <QHeaderView>
#include <QColor>
#include <algorithm>

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
    // Clear the table
    ui->objectStatsTable->clear();
    ui->objectStatsTable->setRowCount(0);
    ui->objectStatsTable->setColumnCount(0);

    const QSqlDatabase &db = m_dbManager->database();

    // First, get all filter types ordered by priority
    QSqlQuery filterTypesQuery(db);
    filterTypesQuery.prepare(R"(
        SELECT id, name, priority
        FROM filter_types
        ORDER BY priority
    )");

    if (!filterTypesQuery.exec())
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to fetch filter types: %1").arg(filterTypesQuery.lastError().text()));
        return;
    }

    QVector<int> filterTypeIds;
    QVector<QString> filterTypeNames;

    while (filterTypesQuery.next())
    {
        filterTypeIds.append(filterTypesQuery.value(0).toInt());
        filterTypeNames.append(filterTypesQuery.value(1).toString());
    }

    // Get all objects that have observations
    QSqlQuery objectsQuery(db);
    objectsQuery.prepare(R"(
        SELECT DISTINCT o.id, o.name
        FROM objects o
        INNER JOIN observations obs ON obs.object_id = o.id
        ORDER BY o.name
    )");

    if (!objectsQuery.exec())
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to fetch objects: %1").arg(objectsQuery.lastError().text()));
        return;
    }

    QVector<int> objectIds;
    QVector<QString> objectNames;

    while (objectsQuery.next())
    {
        objectIds.append(objectsQuery.value(0).toInt());
        objectNames.append(objectsQuery.value(1).toString());
    }

    // If no objects or filter types, show empty table
    if (objectIds.isEmpty() || filterTypeIds.isEmpty())
    {
        return;
    }

    // Query exposure times grouped by object and filter type
    QSqlQuery exposureQuery(db);
    exposureQuery.prepare(R"(
        SELECT
            o.id as object_id,
            ft.id as filter_type_id,
            SUM(obs.total_exposure) as total_exposure
        FROM observations obs
        INNER JOIN objects o ON obs.object_id = o.id
        INNER JOIN filters f ON obs.filter_id = f.id
        INNER JOIN filter_types ft ON f.filter_type_id = ft.id
        GROUP BY o.id, ft.id
    )");

    if (!exposureQuery.exec())
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to fetch exposure data: %1").arg(exposureQuery.lastError().text()));
        return;
    }

    // Store exposure times in a map: (objectId, filterTypeId) -> totalExposure
    QMap<QPair<int, int>, int> exposureMap;

    while (exposureQuery.next())
    {
        int objectId = exposureQuery.value(0).toInt();
        int filterTypeId = exposureQuery.value(1).toInt();
        const int totalExposure = exposureQuery.value(2).toInt();
        exposureMap[qMakePair(objectId, filterTypeId)] = totalExposure;
    }

    // Setup table: columns = Object Name + Filter Types + Total
    const int numCols = static_cast<int>( 1 + filterTypeNames.size() + 1); // Object Name column + filter type columns + Total column
    const int numRows = static_cast<int>( objectNames.size());

    ui->objectStatsTable->setRowCount(numRows);
    ui->objectStatsTable->setColumnCount(numCols);

    // Set header labels
    QStringList headers;
    headers << "Object Name";
    headers << filterTypeNames;
    headers << "Total";
    ui->objectStatsTable->setHorizontalHeaderLabels(headers);

    // Populate the table and collect totals
    QVector<double> totals;
    totals.reserve(objectNames.size());

    for (int row = 0; row < objectNames.size(); ++row)
    {
        int objectId = objectIds[row];
        QString objectName = objectNames[row];

        // First column: Object name
        const auto objectItem = new QTableWidgetItem(objectName);
        objectItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui->objectStatsTable->setItem(row, 0, objectItem);

        int rowTotal = 0;

        // Filter type columns
        for (int col = 0; col < filterTypeNames.size(); ++col)
        {
            int filterTypeId = filterTypeIds[col];
            const int exposure = exposureMap.value(qMakePair(objectId, filterTypeId), 0);

            rowTotal += exposure;

            // Format exposure time in seconds
            QString exposureText = exposure > 0 ? QString::number(exposure) : "";
            const auto item = new NumericTableWidgetItem(exposureText);
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->objectStatsTable->setItem(row, col + 1, item);
        }

        // Last column: Row total
        const auto totalItem = new NumericTableWidgetItem(QString::number(rowTotal));
        totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QFont boldFont = totalItem->font();
        boldFont.setBold(true);
        totalItem->setFont(boldFont);
        ui->objectStatsTable->setItem(row, numCols - 1, totalItem);

        // Collect the total value for conditional formatting
        totals.append(static_cast<double>(rowTotal));
    }

    // Resize columns to content
    ui->objectStatsTable->resizeColumnToContents(0);

    // Apply conditional formatting to the Total column
    applyConditionalFormatting(totals);
}

void ObjectStatsTab::applyConditionalFormatting(const QVector<double> &totals) const {
    if (totals.isEmpty())
    {
        return;
    }

    // Find min and max values
    const double minVal = *std::ranges::min_element(totals);
    const double maxVal = *std::ranges::max_element(totals);

    if (maxVal / minVal <= 2.0)
    {
        // All values are quite close, don't apply color
        return;
    }

    const int totalColIdx = ui->objectStatsTable->columnCount() - 1; // Last column is Total

    // Apply color gradient from red (min) to green (max)
    for (int row = 0; row < totals.size(); ++row)
    {
        const double value = totals[row];

        // Normalize value between 0 and 1
        const double normalized = (value - minVal) / (maxVal - minVal);

        // Map to hue values: Red -> Yellow -> Green
        // 0.0 -> Red (0°), 0.5 -> Yellow (60°), 1.0 -> Green (120°)
        int hue;
        if (normalized <= 0.5)
        {
            // Red to Yellow
            hue = static_cast<int>(normalized * 2 * 60); // 0° -> 60°
        }
        else
        {
            // Yellow to Green
            hue = static_cast<int>(60 + (normalized - 0.5) * 2 * 60); // 60° -> 120°
        }

        // For pastel: high value, low-to-medium saturation
        constexpr int saturation = 40 * 255 / 100; // 0-255, lower = more pastel
        constexpr int valueHsv = 95 * 255 / 100;   // 0-255, brightness

        QColor color = QColor::fromHsv(hue, saturation, valueHsv);

        if (QTableWidgetItem *item = ui->objectStatsTable->item(row, totalColIdx))
        {
            item->setBackground(color);
        }
    }
}
