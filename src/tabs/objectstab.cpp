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
#include <QTableWidgetItem>
#include <QPushButton>
#include <QPointer>

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

    // Configure table columns (without ID column)
    ui->objectsTable->setColumnWidth(0, 200); // Name
    ui->objectsTable->setColumnWidth(1, 150); // RA
    ui->objectsTable->setColumnWidth(2, 150); // Dec

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

    double lat = m_settingsManager->latitude();
    double lon = m_settingsManager->longitude();
    int row = 0;
    for (const ObjectData &obj : objects)
    {
        ui->objectsTable->insertRow(row);

        // Name column
        QTableWidgetItem *nameItem = new QTableWidgetItem(obj.name);
        nameItem->setData(Qt::UserRole, obj.id); // Store ID as hidden data
        ui->objectsTable->setItem(row, 0, nameItem);

        // RA column
        QString raText = obj.ra.isNull() ? "" : QString::number(obj.ra.toDouble(), 'f', 6);
        NumericTableWidgetItem *raItem = new NumericTableWidgetItem(raText);
        raItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->objectsTable->setItem(row, 1, raItem);

        // Dec column
        QString decText = obj.dec.isNull() ? "" : QString::number(obj.dec.toDouble(), 'f', 6);
        NumericTableWidgetItem *decItem = new NumericTableWidgetItem(decText);
        decItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->objectsTable->setItem(row, 2, decItem);

        // Transit time column
        ObjectInfo info = AstroCalc::getObjectInfo(lat, lon, obj.ra.toDouble(), obj.dec.toDouble());
        QTableWidgetItem *transitItem = new QTableWidgetItem(info.transitTime.toLocalTime().toString("hh:mm"));
        transitItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->objectsTable->setItem(row, 3, transitItem);

        row++;
    }

    ui->objectsTable->setSortingEnabled(true);
}

bool ObjectsTab::showObjectDialog(const QString &title, QString &name, QString &ra, QString &dec)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setMinimumWidth(500);

    QFormLayout *formLayout = new QFormLayout(&dialog);

    // Name field
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    nameEdit->setText(name);
    nameEdit->setPlaceholderText("Enter object name");
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

    // Coordinates Lookup button
    QPushButton *lookupButton = new QPushButton("Coordinates Lookup", &dialog);
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
    QPointer<QPushButton> safeButtonPtr(lookupButton);

    // Connect lookup button
    connect(lookupButton, &QPushButton::clicked, [&, nameEdit, safeButtonPtr]()
            {
        QString objectName = nameEdit->text().trimmed();
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
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formLayout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Validate name on OK
    connect(buttonBox, &QDialogButtonBox::accepted, [&]()
            {
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
    connect(&dialog, &QDialog::destroyed, [conn1, conn2, this]()
            {
        disconnect(conn1);
        disconnect(conn2); });

    if (dialog.exec() == QDialog::Accepted)
    {
        name = nameEdit->text().trimmed();
        ra = m_dialogRaEdit->text().trimmed();
        dec = m_dialogDecEdit->text().trimmed();

        // Clear the dialog pointers
        m_dialogRaEdit = nullptr;
        m_dialogDecEdit = nullptr;

        return name.isEmpty() ? false : true;
    }

    // Clear the dialog pointers
    m_dialogRaEdit = nullptr;
    m_dialogDecEdit = nullptr;

    return false;
}

void ObjectsTab::onAddButtonClicked()
{
    QString name, ra, dec;

    if (!showObjectDialog("Add Object", name, ra, dec))
    {
        return;
    }

    // Convert string values to QVariant
    bool raOk = false;
    double raValue = ra.toDouble(&raOk);
    QVariant raVariant = raOk ? QVariant(raValue) : QVariant();

    bool decOk = false;
    double decValue = dec.toDouble(&decOk);
    QVariant decVariant = decOk ? QVariant(decValue) : QVariant();

    // Insert into database using repository
    QString errorMessage;
    if (!m_repository->addObject(name, raVariant, decVariant, errorMessage))
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
    int currentRow = ui->objectsTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select an object to edit.");
        return;
    }

    // Get current object data (ID is stored in UserRole)
    QTableWidgetItem *nameItem = ui->objectsTable->item(currentRow, 0);
    int objectId = nameItem->data(Qt::UserRole).toInt();
    QString currentName = nameItem->text();
    QString currentRa = ui->objectsTable->item(currentRow, 1)->text();
    QString currentDec = ui->objectsTable->item(currentRow, 2)->text();

    // Show dialog
    QString name = currentName;
    QString ra = currentRa;
    QString dec = currentDec;

    if (!showObjectDialog("Edit Object", name, ra, dec))
    {
        return;
    }

    // Convert string values to QVariant
    bool raOk = false;
    double raValue = ra.toDouble(&raOk);
    QVariant raVariant = raOk ? QVariant(raValue) : QVariant();

    bool decOk = false;
    double decValue = dec.toDouble(&decOk);
    QVariant decVariant = decOk ? QVariant(decValue) : QVariant();

    // Update in database using repository
    QString errorMessage;
    if (!m_repository->updateObject(objectId, name, raVariant, decVariant, errorMessage))
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
    int currentRow = ui->objectsTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select an object to delete.");
        return;
    }

    // Get object data
    QTableWidgetItem *nameItem = ui->objectsTable->item(currentRow, 0);
    int objectId = nameItem->data(Qt::UserRole).toInt();
    QString objectName = nameItem->text();

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
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
    QString errorMessage;
    if (!m_repository->deleteObject(objectId, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to delete object: %1").arg(errorMessage));
        return;
    }

    refreshData();
}

void ObjectsTab::onCoordinatesReceived(double ra, double dec, const QString &objectName)
{
    // Convert RA from degrees to hours (RA in degrees / 15 = RA in hours)
    double raHours = ra / 15.0;

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