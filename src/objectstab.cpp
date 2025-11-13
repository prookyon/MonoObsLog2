#include "objectstab.h"
#include "ui_objectstab.h"
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QTableWidgetItem>

ObjectsTab::ObjectsTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::ObjectsTab), m_dbManager(dbManager)
{
    ui->setupUi(this);
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

    QSqlQuery query(m_dbManager->database());
    query.prepare("SELECT id, name, ra, dec FROM objects ORDER BY name");

    if (!query.exec())
    {
        qDebug() << "Failed to query objects:" << query.lastError().text();
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to load objects: %1").arg(query.lastError().text()));
        return;
    }

    int row = 0;
    while (query.next())
    {
        ui->objectsTable->insertRow(row);

        int objectId = query.value(0).toInt();

        // Name column
        QTableWidgetItem *nameItem = new QTableWidgetItem(query.value(1).toString());
        nameItem->setData(Qt::UserRole, objectId); // Store ID as hidden data
        ui->objectsTable->setItem(row, 0, nameItem);

        // RA column
        QString raText = query.value(2).isNull() ? "" : QString::number(query.value(2).toDouble(), 'f', 6);
        QTableWidgetItem *raItem = new QTableWidgetItem(raText);
        raItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->objectsTable->setItem(row, 1, raItem);

        // Dec column
        QString decText = query.value(3).isNull() ? "" : QString::number(query.value(3).toDouble(), 'f', 6);
        QTableWidgetItem *decItem = new QTableWidgetItem(decText);
        decItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->objectsTable->setItem(row, 2, decItem);

        row++;
    }

    ui->objectsTable->setSortingEnabled(true);
}

bool ObjectsTab::showObjectDialog(const QString &title, QString &name, QString &ra, QString &dec)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setMinimumWidth(400);

    QFormLayout *formLayout = new QFormLayout(&dialog);

    // Name field
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    nameEdit->setText(name);
    nameEdit->setPlaceholderText("Enter object name");
    formLayout->addRow("Name:", nameEdit);

    // RA field
    QLineEdit *raEdit = new QLineEdit(&dialog);
    raEdit->setText(ra);
    raEdit->setPlaceholderText("Enter RA in degrees (optional)");
    formLayout->addRow("RA (degrees):", raEdit);

    // Dec field
    QLineEdit *decEdit = new QLineEdit(&dialog);
    decEdit->setText(dec);
    decEdit->setPlaceholderText("Enter Dec in degrees (optional)");
    formLayout->addRow("Dec (degrees):", decEdit);

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

    if (dialog.exec() == QDialog::Accepted)
    {
        name = nameEdit->text().trimmed();
        ra = raEdit->text().trimmed();
        dec = decEdit->text().trimmed();
        return name.isEmpty() ? false : true;
    }

    return false;
}

void ObjectsTab::onAddButtonClicked()
{
    QString name, ra, dec;

    if (!showObjectDialog("Add Object", name, ra, dec))
    {
        return;
    }

    // Insert into database
    QSqlQuery query(m_dbManager->database());
    query.prepare("INSERT INTO objects (name, ra, dec) VALUES (:name, :ra, :dec)");
    query.bindValue(":name", name);

    // Handle RA
    bool raOk = false;
    double raValue = ra.toDouble(&raOk);
    query.bindValue(":ra", raOk ? QVariant(raValue) : QVariant());

    // Handle Dec
    bool decOk = false;
    double decValue = dec.toDouble(&decOk);
    query.bindValue(":dec", decOk ? QVariant(decValue) : QVariant());

    if (!query.exec())
    {
        qDebug() << "Failed to insert object:" << query.lastError().text();
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to add object: %1").arg(query.lastError().text()));
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

    // Update in database
    QSqlQuery query(m_dbManager->database());
    query.prepare("UPDATE objects SET name = :name, ra = :ra, dec = :dec WHERE id = :id");
    query.bindValue(":name", name);

    // Handle RA
    bool raOk = false;
    double raValue = ra.toDouble(&raOk);
    query.bindValue(":ra", raOk ? QVariant(raValue) : QVariant());

    // Handle Dec
    bool decOk = false;
    double decValue = dec.toDouble(&decOk);
    query.bindValue(":dec", decOk ? QVariant(decValue) : QVariant());

    query.bindValue(":id", objectId);

    if (!query.exec())
    {
        qDebug() << "Failed to update object:" << query.lastError().text();
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to update object: %1").arg(query.lastError().text()));
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

    // Delete from database
    QSqlQuery query(m_dbManager->database());
    query.prepare("DELETE FROM objects WHERE id = :id");
    query.bindValue(":id", objectId);

    if (!query.exec())
    {
        qDebug() << "Failed to delete object:" << query.lastError().text();
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to delete object: %1").arg(query.lastError().text()));
        return;
    }

    refreshData();
}