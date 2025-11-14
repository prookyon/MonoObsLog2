#include "tabs/filtertypestab.h"
#include "ui_filtertypes_tab.h"
#include "db/databasemanager.h"
#include "db/filtertypesrepository.h"
#include <QDebug>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QTableWidgetItem>
#include <QPushButton>

FilterTypesTab::FilterTypesTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::FilterTypesTab), m_dbManager(dbManager), m_repository(nullptr)
{
    ui->setupUi(this);
    m_repository = new FilterTypesRepository(m_dbManager, this);
}

FilterTypesTab::~FilterTypesTab()
{
    delete ui;
}

void FilterTypesTab::initialize()
{
    // Connect buttons to slots
    connect(ui->addFilterTypeButton, &QPushButton::clicked, this, &FilterTypesTab::onAddFilterTypeButtonClicked);
    connect(ui->editFilterTypeButton, &QPushButton::clicked, this, &FilterTypesTab::onEditFilterTypeButtonClicked);
    connect(ui->deleteFilterTypeButton, &QPushButton::clicked, this, &FilterTypesTab::onDeleteFilterTypeButtonClicked);

    // Configure table columns
    ui->filterTypesTable->setColumnWidth(0, 200); // Name
    ui->filterTypesTable->setColumnWidth(1, 80);  // Priority

    // Initialize the tab UI and data
    refreshData();
}

void FilterTypesTab::refreshData()
{
    // Refresh data from database
    populateTable();
}

void FilterTypesTab::populateTable()
{
    ui->filterTypesTable->setSortingEnabled(false);
    ui->filterTypesTable->setRowCount(0);

    QString errorMessage;
    QVector<FilterTypeData> filterTypes = m_repository->getAllFilterTypes(errorMessage);

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to load filter types: %1").arg(errorMessage));
        return;
    }

    int row = 0;
    for (const FilterTypeData &filterType : filterTypes)
    {
        ui->filterTypesTable->insertRow(row);

        // Name column
        QTableWidgetItem *nameItem = new QTableWidgetItem(filterType.name);
        nameItem->setData(Qt::UserRole, filterType.id); // Store ID as hidden data
        ui->filterTypesTable->setItem(row, 0, nameItem);

        // Priority column
        QTableWidgetItem *priorityItem = new QTableWidgetItem(QString::number(filterType.priority));
        priorityItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->filterTypesTable->setItem(row, 1, priorityItem);

        row++;
    }

    ui->filterTypesTable->setSortingEnabled(true);
}

bool FilterTypesTab::showFilterTypeDialog(const QString &title, QString &name, int &priority)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setMinimumWidth(400);

    QFormLayout *formLayout = new QFormLayout(&dialog);

    // Name field
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    nameEdit->setText(name);
    nameEdit->setPlaceholderText("Enter filter type name");
    formLayout->addRow("Name:", nameEdit);

    // Priority field
    QSpinBox *prioritySpin = new QSpinBox(&dialog);
    prioritySpin->setMinimum(0);
    prioritySpin->setMaximum(100);
    prioritySpin->setValue(priority);
    formLayout->addRow("Priority:", prioritySpin);

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
            QMessageBox::warning(&dialog, "Validation Error", "Filter type name cannot be empty.");
            dialog.done(QDialog::Rejected);
        } });

    if (dialog.exec() == QDialog::Accepted)
    {
        name = nameEdit->text().trimmed();
        priority = prioritySpin->value();

        return name.isEmpty() ? false : true;
    }

    return false;
}

void FilterTypesTab::onAddFilterTypeButtonClicked()
{
    QString name = ui->nameLineEdit->text().trimmed();
    int priority = ui->prioritySpinBox->value();

    // Validate inputs
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Validation Error", "Filter type name cannot be empty.");
        return;
    }

    // Insert into database using repository
    QString errorMessage;
    if (!m_repository->addFilterType(name, priority, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to add filter type: %1").arg(errorMessage));
        return;
    }

    // Clear input fields
    ui->nameLineEdit->clear();
    ui->prioritySpinBox->setValue(0);

    refreshData();
}

void FilterTypesTab::onEditFilterTypeButtonClicked()
{
    // Check if a row is selected
    int currentRow = ui->filterTypesTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select a filter type to edit.");
        return;
    }

    // Get current filter type data (ID is stored in UserRole)
    QTableWidgetItem *nameItem = ui->filterTypesTable->item(currentRow, 0);
    int filterTypeId = nameItem->data(Qt::UserRole).toInt();
    QString currentName = nameItem->text();
    int currentPriority = ui->filterTypesTable->item(currentRow, 1)->text().toInt();

    // Show dialog
    QString name = currentName;
    int priority = currentPriority;

    if (!showFilterTypeDialog("Edit Filter Type", name, priority))
    {
        return;
    }

    // Update in database using repository
    QString errorMessage;
    if (!m_repository->updateFilterType(filterTypeId, name, priority, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to update filter type: %1").arg(errorMessage));
        return;
    }

    refreshData();
}

void FilterTypesTab::onDeleteFilterTypeButtonClicked()
{
    // Check if a row is selected
    int currentRow = ui->filterTypesTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select a filter type to delete.");
        return;
    }

    // Get filter type data
    QTableWidgetItem *nameItem = ui->filterTypesTable->item(currentRow, 0);
    int filterTypeId = nameItem->data(Qt::UserRole).toInt();
    QString filterTypeName = nameItem->text();

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Deletion",
        QString("Are you sure you want to delete the filter type '%1'?").arg(filterTypeName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    // Delete from database using repository
    QString errorMessage;
    if (!m_repository->deleteFilterType(filterTypeId, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to delete filter type: %1").arg(errorMessage));
        return;
    }

    refreshData();
}