#include "tabs/objectstab.h"
#include "ui_objects_tab.h"
#include "db/databasemanager.h"
#include "db/objectsrepository.h"
#include "simbadquery.h"
#include "numerictablewidgetitem.h"
#include "astrocalc.h"
#include "settingsmanager.h"
#include <QDebug>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFile>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QPointer>
#include <qwt/qwt_polar_grid.h>
#include <qwt/qwt_polar_marker.h>
#include <qwt/qwt_polar_curve.h>
#include <qwt/qwt_point_polar.h>
#include <qwt/qwt_series_data.h>
#include <qwt/qwt_text.h>
#include <qwt/qwt_symbol.h>
#include <qwt/qwt_polar_panner.h>
#include <qwt/qwt_polar_magnifier.h>
#include <qwt/qwt_scale_div.h>
#include <qwt/qwt_polar_canvas.h>

ObjectsTab::ObjectsTab(DatabaseManager *dbManager, SettingsManager *settingsManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::ObjectsTab), m_dbManager(dbManager), m_settingsManager(settingsManager), m_repository(nullptr), m_simbadQuery(nullptr), m_dialogRaEdit(nullptr), m_dialogDecEdit(nullptr)
{
    ui->setupUi(this);
    m_repository = new ObjectsRepository(m_dbManager, this);
}

ObjectsTab::~ObjectsTab()
{
    delete ui;
}

void ObjectsTab::initialize()
{
    // Connect buttons to slots
    connect(ui->addButton, &QPushButton::clicked, this, &ObjectsTab::onAddButtonClicked);
    connect(ui->editButton, &QPushButton::clicked, this, &ObjectsTab::onEditButtonClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &ObjectsTab::onDeleteButtonClicked);
    connect(ui->plotButton, &QPushButton::clicked, this, &ObjectsTab::populateTable);

    // Configure table columns (without ID column)
    ui->objectsTable->setColumnWidth(0, 200); // Name
    ui->objectsTable->setColumnWidth(1, 150); // RA
    ui->objectsTable->setColumnWidth(2, 150); // Dec
    //Comments is already set to stretch

    // Initialize polar plot
    ui->polarPlot->setPlotBackground(QColor(0,66,124,255));

    // Configure axes
    ui->polarPlot->setScale(QwtPolar::Azimuth, 0.0, 360.0, 30.0);
    ui->polarPlot->setScale(QwtPolar::Radius, 90.0, 0.0, 30.0);

    // Add grid
    auto *grid = new QwtPolarGrid();
    grid->setPen( QPen( Qt::white ) );
    grid->setAxisPen(QwtPolar::AxisAzimuth, QPen(Qt::black));
    grid->showAxis( QwtPolar::AxisAzimuth, true );
    grid->showAxis( QwtPolar::AxisLeft, false );
    grid->showAxis( QwtPolar::AxisRight, false );
    grid->showAxis( QwtPolar::AxisTop, false );
    grid->showAxis( QwtPolar::AxisBottom, true );
    grid->attach(ui->polarPlot);

    // Set azimuth origin to North (top) and direction clockwise
    ui->polarPlot->setAzimuthOrigin(90.0*M_PI/180.0);
    ui->polarPlot->setScaleMaxMinor(QwtPolar::Azimuth, 0);
    ui->polarPlot->setScaleMaxMinor(QwtPolar::Radius, 0);

    m_panner = new QwtPolarPanner( ui->polarPlot->canvas() );
    m_panner->setEnabled( true );
    ui->polarPlot->canvas()->setCursor(QCursor(Qt::SizeAllCursor));

    m_zoomer = new QwtPolarMagnifier( ui->polarPlot->canvas() );
    m_zoomer->setEnabled( true );

    LoadConstellations();

    // Initialize the tab UI and data
    refreshData();
}

void ObjectsTab::refreshData()
{
    // Refresh data from database
    populateTable();
}

void ObjectsTab::populateTable()
{
    ui->objectsTable->setSortingEnabled(false);
    ui->objectsTable->setRowCount(0);

    QString errorMessage;
    QVector<ObjectData> objects = m_repository->getAllObjects(errorMessage);

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to load objects: %1").arg(errorMessage));
        return;
    }

    // Clear existing markers and curves from polar plot
    ui->polarPlot->detachItems(QwtPolarItem::Rtti_PolarMarker);
    ui->polarPlot->detachItems(QwtPolarItem::Rtti_PolarCurve);

    double lat = m_settingsManager->latitude();
    double lon = m_settingsManager->longitude();
    int row = 0;
    for (const ObjectData &obj : objects)
    {
        ui->objectsTable->insertRow(row);

        // Name column
        auto nameItem = new QTableWidgetItem(obj.name);
        nameItem->setData(Qt::UserRole, obj.id); // Store ID as hidden data
        ui->objectsTable->setItem(row, 0, nameItem);

        // RA column
        QString raText = obj.ra.isNull() ? "" : QString::number(obj.ra.toDouble(), 'f', 6);
        auto raItem = new NumericTableWidgetItem(raText);
        raItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->objectsTable->setItem(row, 1, raItem);

        // Dec column
        QString decText = obj.dec.isNull() ? "" : QString::number(obj.dec.toDouble(), 'f', 6);
        auto decItem = new NumericTableWidgetItem(decText);
        decItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->objectsTable->setItem(row, 2, decItem);

        // Get object info for transit time and position
        if (!obj.ra.isNull() && !obj.dec.isNull())
        {
            ObjectInfo info = AstroCalc::getObjectInfo(lat, lon, obj.ra.toDouble(), obj.dec.toDouble());
            
            // Transit time column
            auto transitItem = new QTableWidgetItem(info.transitTime.toLocalTime().toString("hh:mm"));
            transitItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->objectsTable->setItem(row, 3, transitItem);

            // Add marker to polar plot if altitude is positive (object is above horizon)
            if (info.altitude > 0)
            {
                auto marker = new QwtPolarMarker();
                
                // Set position: azimuth (angular) and altitude
                QwtPointPolar position(info.azimuth, info.altitude);
                marker->setPosition(position);
                
                // Set label with object name
                QwtText label(obj.name);
                label.setColor(Qt::white);
                label.setBackgroundBrush(QBrush(QColor(0, 0, 0, 128))); // Semi-transparent black background
                marker->setLabel(label);
                marker->setLabelAlignment(Qt::AlignTop | Qt::AlignHCenter);
                
                // Set marker symbol
                marker->setSymbol(new QwtSymbol(QwtSymbol::Ellipse,
                                               QBrush(Qt::yellow),
                                               QPen(Qt::white),
                                               QSize(4, 4)));
                
                marker->attach(ui->polarPlot);
            }
        }
        else
        {
            // Empty transit time if coordinates are missing
            auto transitItem = new QTableWidgetItem("");
            transitItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->objectsTable->setItem(row, 3, transitItem);
        }

        //Comments
        auto commentsItem = new QTableWidgetItem(obj.comments);
        ui->objectsTable->setItem(row, 4, commentsItem);

        row++;
    }

    ui->objectsTable->resizeColumnsToContents();

    // Draw constellation lines
    QwtPolarCurve *curve = nullptr;

    for (const ConstellationLine &line : m_constellations) {
        auto first = AstroCalc::getObjectInfo(lat, lon, line.ra1/15.0, line.dec1);
        auto second = AstroCalc::getObjectInfo(lat, lon, line.ra2/15.0, line.dec2);

        // Skip if both points are below horizon
        if (first.altitude < 0 && second.altitude < 0)
            continue;

        curve = new QwtPolarCurve();
        curve->setPen(QPen(Qt::cyan, 1));
        curve->setSymbol( new QwtSymbol( QwtSymbol::Ellipse,
                QBrush( QColor(220,220,220,255)), QPen(  QColor(220,220,220,255) ), QSize( 2, 2 ) ) );
        QVector<QwtPointPolar> points;

        // Add points to current curve (clamp altitude to 0 if below horizon)
        points.append(QwtPointPolar(first.azimuth, qMax(0.0, first.altitude)));
        points.append(QwtPointPolar(second.azimuth, qMax(0.0, second.altitude)));
        curve->setData(new QwtArraySeriesData(points));
        curve->attach(ui->polarPlot);
    }

    ui->objectsTable->setSortingEnabled(true);
    ui->polarPlot->setTitle(QDateTime::currentDateTime().toString(  Qt::ISODate));
    ui->polarPlot->replot();
}

bool ObjectsTab::showObjectDialog(const QString &title, QString &name, QString &ra, QString &dec, QString &comments)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setMinimumWidth(500);

    const auto formLayout = new QFormLayout(&dialog);

    // Name field
    auto nameEdit = new QLineEdit(&dialog);
    nameEdit->setText(name);
    nameEdit->setPlaceholderText("Enter object name (duplicate name only allowed with different comment)");
    formLayout->addRow("Name:", nameEdit);

    // RA field
    m_dialogRaEdit = new QLineEdit(&dialog);
    m_dialogRaEdit->setText(ra);
    m_dialogRaEdit->setPlaceholderText("Enter RA in hours (optional, decimal format only)");
    formLayout->addRow("RA (hours):", m_dialogRaEdit);

    // Dec field
    m_dialogDecEdit = new QLineEdit(&dialog);
    m_dialogDecEdit->setText(dec);
    m_dialogDecEdit->setPlaceholderText("Enter Dec in degrees (optional)");
    formLayout->addRow("Dec (degrees):", m_dialogDecEdit);

    // Comments field
    auto commentsEdit = new QLineEdit(&dialog);
    commentsEdit->setText(comments);
    commentsEdit->setPlaceholderText("Enter comment (optional)");
    formLayout->addRow("Comments:", commentsEdit);

    // Coordinates Lookup button
    const auto lookupButton = new QPushButton("Coordinates Lookup", &dialog);
    lookupButton->setToolTip("Query SIMBAD database for coordinates based on object name");
    formLayout->addRow("", lookupButton);

    // Create SIMBAD query object if not exists
    if (!m_simbadQuery)
    {
        m_simbadQuery = new SimbadQuery(this);
        connect(m_simbadQuery, &SimbadQuery::coordinatesReceived,
                this, &ObjectsTab::onCoordinatesReceived);
        connect(m_simbadQuery, &SimbadQuery::errorOccurred,
                this, &ObjectsTab::onSimbadError);
    }

    // Use QPointer to safely track the button
    QPointer safeButtonPtr(lookupButton);

    // Connect lookup button
    connect(lookupButton, &QPushButton::clicked, &dialog, [&dialog, nameEdit, safeButtonPtr, this] {
        const QString objectName = nameEdit->text().trimmed();
        if (objectName.isEmpty()) {
            QMessageBox::warning(&dialog, "Input Error", "Please enter an object name first.");
            return;
        }
        if (safeButtonPtr) {
            safeButtonPtr->setEnabled(false);
            safeButtonPtr->setText("Querying SIMBAD...");
        }
        m_simbadQuery->queryObject(objectName); });

    // Buttons
    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formLayout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Validate name on OK
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, [&dialog, nameEdit] {
        if (nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Validation Error", "Object name cannot be empty.");
            dialog.done(QDialog::Rejected);
        } });

    // Re-enable lookup button when query completes (use QPointer for safety)
    QMetaObject::Connection conn1 = connect(m_simbadQuery, &SimbadQuery::coordinatesReceived,
                                            [safeButtonPtr](double, double, const QString &)
                                            {
        if (safeButtonPtr) {
            safeButtonPtr->setEnabled(true);
            safeButtonPtr->setText("Coordinates Lookup");
        } });

    QMetaObject::Connection conn2 = connect(m_simbadQuery, &SimbadQuery::errorOccurred,
                                            [safeButtonPtr](const QString &)
                                            {
        if (safeButtonPtr) {
            safeButtonPtr->setEnabled(true);
            safeButtonPtr->setText("Coordinates Lookup");
        } });

    // Disconnect these specific connections when dialog is destroyed
    connect(&dialog, &QDialog::destroyed, [conn1, conn2] {
        disconnect(conn1);
        disconnect(conn2);
    });

    if (dialog.exec() == QDialog::Accepted)
    {
        name = nameEdit->text().trimmed();
        ra = m_dialogRaEdit->text().trimmed();
        dec = m_dialogDecEdit->text().trimmed();
        comments = commentsEdit->text().trimmed();

        // Clear the dialog pointers
        m_dialogRaEdit = nullptr;
        m_dialogDecEdit = nullptr;

        return !name.isEmpty();
    }

    // Clear the dialog pointers
    m_dialogRaEdit = nullptr;
    m_dialogDecEdit = nullptr;

    return false;
}

void ObjectsTab::onAddButtonClicked()
{
    QString name, ra, dec, comments;

    if (!showObjectDialog("Add Object", name, ra, dec, comments))
    {
        return;
    }

    // Convert string values to QVariant
    bool raOk = false;
    const double raValue = ra.toDouble(&raOk);
    const QVariant raVariant = raOk ? QVariant(raValue) : QVariant();

    bool decOk = false;
    const double decValue = dec.toDouble(&decOk);
    const QVariant decVariant = decOk ? QVariant(decValue) : QVariant();

    // Insert into database using repository
    if (QString errorMessage; !m_repository->addObject(name, raVariant, decVariant, comments, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to add object: %1").arg(errorMessage));
        return;
    }

    refreshData();
}

void ObjectsTab::onEditButtonClicked()
{
    // Check if a row is selected
    const int currentRow = ui->objectsTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select an object to edit.");
        return;
    }

    // Get current object data (ID is stored in UserRole)
    const QTableWidgetItem *nameItem = ui->objectsTable->item(currentRow, 0);
    const int objectId = nameItem->data(Qt::UserRole).toInt();
    const QString currentName = nameItem->text();
    const QString currentRa = ui->objectsTable->item(currentRow, 1)->text();
    const QString currentDec = ui->objectsTable->item(currentRow, 2)->text();
    const QString currentComments = ui->objectsTable->item(currentRow, 4)->text();

    // Show dialog
    QString name = currentName;
    QString ra = currentRa;
    QString dec = currentDec;
    QString comments = currentComments;

    if (!showObjectDialog("Edit Object", name, ra, dec, comments))
    {
        return;
    }

    // Convert string values to QVariant
    bool raOk = false;
    const double raValue = ra.toDouble(&raOk);
    const QVariant raVariant = raOk ? QVariant(raValue) : QVariant();

    bool decOk = false;
    const double decValue = dec.toDouble(&decOk);
    const QVariant decVariant = decOk ? QVariant(decValue) : QVariant();

    // Update in database using repository
    if (QString errorMessage; !m_repository->updateObject(objectId, name, raVariant, decVariant,comments, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to update object: %1").arg(errorMessage));
        return;
    }

    refreshData();
}

void ObjectsTab::onDeleteButtonClicked()
{
    // Check if a row is selected
    const int currentRow = ui->objectsTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select an object to delete.");
        return;
    }

    // Get object data
    const QTableWidgetItem *nameItem = ui->objectsTable->item(currentRow, 0);
    const int objectId = nameItem->data(Qt::UserRole).toInt();
    const QString objectName = nameItem->text();

    // Confirm deletion
    const QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Deletion",
        QString("Are you sure you want to delete the object '%1'?").arg(objectName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    // Delete from database using repository
    if (QString errorMessage; !m_repository->deleteObject(objectId, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to delete object: %1").arg(errorMessage));
        return;
    }

    refreshData();
}

void ObjectsTab::onCoordinatesReceived(const double ra, const double dec, const QString &objectName) const {
    // Convert RA from degrees to hours (RA in degrees / 15 = RA in hours)
    const double raHours = ra / 15.0;

    qDebug() << "Received coordinates for" << objectName;
    qDebug() << "RA:" << ra << "degrees =" << raHours << "hours";
    qDebug() << "DEC:" << dec << "degrees";

    // Update the dialog fields if they exist
    if (m_dialogRaEdit)
    {
        m_dialogRaEdit->setText(QString::number(raHours, 'f', 6));
    }
    if (m_dialogDecEdit)
    {
        m_dialogDecEdit->setText(QString::number(dec, 'f', 6));
    }
}

void ObjectsTab::onSimbadError(const QString &error)
{
    qDebug() << "SIMBAD error:" << error;
    QMessageBox::warning(nullptr, "SIMBAD Query Error",
                         QString("Failed to retrieve coordinates:\n\n%1").arg(error));
}

void ObjectsTab::LoadConstellations() {
    // Read the template file
    QFile conCsv(":/data/constellations.csv");
    if (!conCsv.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error",
                              "Failed to open constellations file: " + conCsv.errorString());
        return;
    }

    m_constellations.clear();
    QTextStream stream(&conCsv);
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (QStringList parts = line.split(','); parts.size() == 5) {
            ConstellationLine cl;
            cl.name = parts[0];
            bool ok1, ok2, ok3, ok4;
            cl.ra1 = parts[1].toDouble(&ok1);
            cl.dec1 = parts[2].toDouble(&ok2);
            cl.ra2 = parts[3].toDouble(&ok3);
            cl.dec2 = parts[4].toDouble(&ok4);
            if (ok1 && ok2 && ok3 && ok4) {
                m_constellations.append(cl);
            }
        }
    }
    conCsv.close();
}