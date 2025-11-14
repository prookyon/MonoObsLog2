#include "tabs/filterstab.h"
#include "ui_filters_tab.h"
#include "db/databasemanager.h"
#include "db/filtersrepository.h"
#include "db/filtertypesrepository.h"
#include <QDebug>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QTableWidgetItem>
#include <QPushButton>

FiltersTab::FiltersTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::FiltersTab), m_dbManager(dbManager), m_repository(nullptr), m_filterTypesRepository(nullptr)
{
    ui->setupUi(this);
    m_repository = new FiltersRepository(m_dbManager, this);
    m_filterTypesRepository = new FilterTypesRepository(m_dbManager, this);
}

FiltersTab::~FiltersTab()
{
    delete ui;
}

void FiltersTab::initialize()
{
    // Connect buttons to slots
    connect(ui->addFilterButton, &QPushButton::clicked, this, &FiltersTab::onAddFilterButtonClicked);
    connect(ui->editFilterButton, &QPushButton::clicked, this, &FiltersTab::onEditFilterButtonClicked);
    connect(ui->deleteFilterButton, &QPushButton::clicked, this, &FiltersTab::onDeleteFilterButtonClicked);

    // Configure table columns
    ui->filtersTable->setColumnWidth(0, 200); // Name
    ui->filtersTable->setColumnWidth(1, 200); // Type

    // Initialize the tab UI and data
    refreshData();
}

void FiltersTab::refreshData()
{
    // Refresh filter types first, then populate table
    populateFilterTypeComboBox();
    populateTable();
}

void FiltersTab::populateFilterTypeComboBox()
{
    ui->typeComboBox->clear();

    QString errorMessage;
    QVector<FilterTypeData> filterTypes = m_filterTypesRepository->getAllFilterTypes(errorMessage);

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to load filter types: %1").arg(errorMessage));
        return;
    }

    for (const FilterTypeData &filterType : filterTypes)
    {
        ui->typeComboBox->addItem(filterType.name, filterType.id);
    }
}

void FiltersTab::populateTable()
{
    ui->filtersTable->setSortingEnabled(false);
    ui->filtersTable->setRowCount(0);

    QString errorMessage;
    QVector<FilterData> filters = m_repository->getAllFilters(errorMessage);

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to load filters: %1").arg(errorMessage));
        return;
    }

    int row = 0;
    for (const FilterData &filter : filters)
    {
        ui->filtersTable->insertRow(row);

        // Name column
        QTableWidgetItem *nameItem = new QTableWidgetItem(filter.name);
        nameItem->setData(Qt::UserRole, filter.id); // Store ID as hidden data
        ui->filtersTable->setItem(row, 0, nameItem);

        // Type column
        QTableWidgetItem *typeItem = new QTableWidgetItem(filter.filterTypeName);
        typeItem->setData(Qt::UserRole, filter.filterTypeId); // Store filter type ID
        ui->filtersTable->setItem(row, 1, typeItem);

        row++;
    }

    ui->filtersTable->setSortingEnabled(true);
}

bool FiltersTab::showFilterDialog(const QString &title, QString &name, int &filterTypeId)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setMinimumWidth(400);

    QFormLayout *formLayout = new QFormLayout(&dialog);

    // Name field
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    nameEdit->setText(name);
    nameEdit->setPlaceholderText("Enter filter name");
    formLayout->addRow("Name:", nameEdit);

    // Filter Type field
    QComboBox *typeCombo = new QComboBox(&dialog);

    // Populate combo box with filter types
    QString errorMessage;
    QVector<FilterTypeData> filterTypes = m_filterTypesRepository->getAllFilterTypes(errorMessage);

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(&dialog, "Database Error",
                             QString("Failed to load filter types: %1").arg(errorMessage));
        return false;
    }

    int selectedIndex = 0;
    for (int i = 0; i < filterTypes.size(); ++i)
    {
        typeCombo->addItem(filterTypes[i].name, filterTypes[i].id);
        if (filterTypes[i].id == filterTypeId)
        {
            selectedIndex = i;
        }
    }
    typeCombo->setCurrentIndex(selectedIndex);
    formLayout->addRow("Filter Type:", typeCombo);

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
            QMessageBox::warning(&dialog, "Validation Error", "Filter name cannot be empty.");
            dialog.done(QDialog::Rejected);
        } });

    if (dialog.exec() == QDialog::Accepted)
    {
        name = nameEdit->text().trimmed();
        filterTypeId = typeCombo->currentData().toInt();

        return !name.isEmpty();
    }

    return false;
}

void FiltersTab::onAddFilterButtonClicked()
{
    QString name = ui->nameLineEdit->text().trimmed();
    int filterTypeId = ui->typeComboBox->currentData().toInt();

    // Validate inputs
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Validation Error", "Filter name cannot be empty.");
        return;
    }
    if (filterTypeId <= 0)
    {
        QMessageBox::warning(this, "Validation Error", "Please select a filter type.");
        return;
    }

    // Insert into database using repository
    QString errorMessage;
    if (!m_repository->addFilter(name, filterTypeId, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to add filter: %1").arg(errorMessage));
        return;
    }

    // Clear input fields
    ui->nameLineEdit->clear();
    ui->typeComboBox->setCurrentIndex(0);

    refreshData();
}

void FiltersTab::onEditFilterButtonClicked()
{
    // Check if a row is selected
    int currentRow = ui->filtersTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select a filter to edit.");
        return;
    }

    // Get current filter data (ID is stored in UserRole)
    QTableWidgetItem *nameItem = ui->filtersTable->item(currentRow, 0);
    int filterId = nameItem->data(Qt::UserRole).toInt();
    QString currentName = nameItem->text();

    QTableWidgetItem *typeItem = ui->filtersTable->item(currentRow, 1);
    int currentFilterTypeId = typeItem->data(Qt::UserRole).toInt();

    // Show dialog
    QString name = currentName;
    int filterTypeId = currentFilterTypeId;

    if (!showFilterDialog("Edit Filter", name, filterTypeId))
    {
        return;
    }

    // Update in database using repository
    QString errorMessage;
    if (!m_repository->updateFilter(filterId, name, filterTypeId, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to update filter: %1").arg(errorMessage));
        return;
    }

    refreshData();
}

void FiltersTab::onDeleteFilterButtonClicked()
{
    // Check if a row is selected
    int currentRow = ui->filtersTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select a filter to delete.");
        return;
    }

    // Get filter data
    QTableWidgetItem *nameItem = ui->filtersTable->item(currentRow, 0);
    int filterId = nameItem->data(Qt::UserRole).toInt();
    QString filterName = nameItem->text();

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Deletion",
        QString("Are you sure you want to delete the filter '%1'?").arg(filterName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    // Delete from database using repository
    QString errorMessage;
    if (!m_repository->deleteFilter(filterId, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to delete filter: %1").arg(errorMessage));
        return;
    }

    refreshData();
}
