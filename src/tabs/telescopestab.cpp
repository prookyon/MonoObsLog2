#include "tabs/telescopestab.h"
#include "ui_telescopes_tab.h"
#include "db/databasemanager.h"
#include "db/telescopesrepository.h"
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QTableWidgetItem>
#include <QPushButton>

TelescopesTab::TelescopesTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::TelescopesTab), m_dbManager(dbManager), m_repository(nullptr)
{
    ui->setupUi(this);
    m_repository = new TelescopesRepository(m_dbManager, this);
}

TelescopesTab::~TelescopesTab()
{
    delete ui;
}

void TelescopesTab::initialize()
{
    // Connect buttons to slots
    connect(ui->addTelescopeButton, &QPushButton::clicked, this, &TelescopesTab::onAddTelescopeButtonClicked);
    connect(ui->editTelescopeButton, &QPushButton::clicked, this, &TelescopesTab::onEditTelescopeButtonClicked);
    connect(ui->deleteTelescopeButton, &QPushButton::clicked, this, &TelescopesTab::onDeleteTelescopeButtonClicked);

    // Connect spin boxes for auto-calculation of f-ratio
    connect(ui->apertureSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TelescopesTab::onApertureOrFocalLengthChanged);
    connect(ui->focalLengthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TelescopesTab::onApertureOrFocalLengthChanged);

    // Configure table columns (without ID column)
    ui->telescopesTable->setColumnWidth(0, 150); // Name
    ui->telescopesTable->setColumnWidth(1, 100); // Aperture
    ui->telescopesTable->setColumnWidth(2, 80);  // F-ratio
    ui->telescopesTable->setColumnWidth(3, 120); // Focal Length

    // Initialize the tab UI and data
    refreshData();
}

void TelescopesTab::refreshData()
{
    // Refresh data from database
    populateTable();
}

void TelescopesTab::populateTable()
{
    ui->telescopesTable->setSortingEnabled(false);
    ui->telescopesTable->setRowCount(0);

    auto telescopesResult = m_repository->getAllTelescopes();
    if (!telescopesResult)
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to load telescopes: %1").arg(telescopesResult.error().errorMessage));
        return;
    }
    QVector<TelescopeData> telescopes = telescopesResult.value();

    int row = 0;
    for (const auto &[id, name, aperture, fRatio, focalLength] : telescopes)
    {
        ui->telescopesTable->insertRow(row);

        // Name column
        const auto nameItem = new QTableWidgetItem(name);
        nameItem->setData(Qt::UserRole, id); // Store ID as hidden data
        ui->telescopesTable->setItem(row, 0, nameItem);

        // Aperture column
        const auto apertureItem = new QTableWidgetItem(QString::number(aperture));
        apertureItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->telescopesTable->setItem(row, 1, apertureItem);

        // F-ratio column
        const auto fRatioItem = new QTableWidgetItem(QString::number(fRatio, 'f', 2));
        fRatioItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->telescopesTable->setItem(row, 2, fRatioItem);

        // Focal Length column
        const auto focalLengthItem = new QTableWidgetItem(QString::number(focalLength));
        focalLengthItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->telescopesTable->setItem(row, 3, focalLengthItem);

        row++;
    }

    ui->telescopesTable->setSortingEnabled(true);
}

bool TelescopesTab::showTelescopeDialog(const QString &title, QString &name, int &aperture, double &fRatio, int &focalLength)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setMinimumWidth(500);

    const auto formLayout = new QFormLayout(&dialog);

    // Name field
    const auto nameEdit = new QLineEdit(&dialog);
    nameEdit->setText(name);
    nameEdit->setPlaceholderText("Enter telescope name");
    formLayout->addRow("Name:", nameEdit);

    // Aperture field
    const auto apertureSpin = new QSpinBox(&dialog);
    apertureSpin->setMaximum(99999);
    apertureSpin->setValue(aperture);
    formLayout->addRow("Aperture (mm):", apertureSpin);

    // F-ratio field
    const auto fRatioSpin = new QDoubleSpinBox(&dialog);
    fRatioSpin->setMaximum(100.0);
    fRatioSpin->setDecimals(2);
    fRatioSpin->setSingleStep(0.1);
    fRatioSpin->setValue(fRatio);
    formLayout->addRow("F-ratio:", fRatioSpin);

    // Focal Length field
    const auto focalLengthSpin = new QSpinBox(&dialog);
    focalLengthSpin->setMaximum(99999);
    focalLengthSpin->setValue(focalLength);
    formLayout->addRow("Focal Length (mm):", focalLengthSpin);

    // Buttons
    const auto buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formLayout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Validate name on OK
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, [&dialog, nameEdit]
            {
        if (nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Validation Error", "Telescope name cannot be empty.");
            dialog.done(QDialog::Rejected);
        } });

    if (dialog.exec() == QDialog::Accepted)
    {
        name = nameEdit->text().trimmed();
        aperture = apertureSpin->value();
        fRatio = fRatioSpin->value();
        focalLength = focalLengthSpin->value();

        return !name.isEmpty();
    }

    return false;
}

void TelescopesTab::onAddTelescopeButtonClicked()
{
    const QString name = ui->nameLineEdit->text().trimmed();
    const int aperture = ui->apertureSpinBox->value();
    const double fRatio = ui->fRatioSpinBox->value();
    const int focalLength = ui->focalLengthSpinBox->value();

    // Validate inputs
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Validation Error", "Telescope name cannot be empty.");
        return;
    }

    // Insert into database using repository
    auto addResult = m_repository->addTelescope(name, aperture, fRatio, focalLength);
    if (!addResult)
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to add telescope: %1").arg(addResult.error().errorMessage));
        return;
    }

    // Clear input fields
    ui->nameLineEdit->clear();
    ui->apertureSpinBox->setValue(0);
    ui->fRatioSpinBox->setValue(0.0);
    ui->focalLengthSpinBox->setValue(0);

    refreshData();
}

void TelescopesTab::onEditTelescopeButtonClicked()
{
    // Check if a row is selected
    const int currentRow = ui->telescopesTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select a telescope to edit.");
        return;
    }

    // Get current telescope data (ID is stored in UserRole)
    const QTableWidgetItem *nameItem = ui->telescopesTable->item(currentRow, 0);
    const int telescopeId = nameItem->data(Qt::UserRole).toInt();
    const QString currentName = nameItem->text();
    const int currentAperture = ui->telescopesTable->item(currentRow, 1)->text().toInt();
    const double currentFRatio = ui->telescopesTable->item(currentRow, 2)->text().toDouble();
    const int currentFocalLength = ui->telescopesTable->item(currentRow, 3)->text().toInt();

    // Show dialog
    QString name = currentName;
    int aperture = currentAperture;
    double fRatio = currentFRatio;
    int focalLength = currentFocalLength;

    if (!showTelescopeDialog("Edit Telescope", name, aperture, fRatio, focalLength))
    {
        return;
    }

    // Update in database using repository
    auto updateResult = m_repository->updateTelescope(telescopeId, name, aperture, fRatio, focalLength);
    if (!updateResult)
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to update telescope: %1").arg(updateResult.error().errorMessage));
        return;
    }

    refreshData();
}

void TelescopesTab::onDeleteTelescopeButtonClicked()
{
    // Check if a row is selected
    const int currentRow = ui->telescopesTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select a telescope to delete.");
        return;
    }

    // Get telescope data
    const QTableWidgetItem *nameItem = ui->telescopesTable->item(currentRow, 0);
    const int telescopeId = nameItem->data(Qt::UserRole).toInt();
    const QString telescopeName = nameItem->text();

    // Confirm deletion
    const QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Deletion",
        QString("Are you sure you want to delete the telescope '%1'?").arg(telescopeName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    // Delete from database using repository
    auto deleteResult = m_repository->deleteTelescope(telescopeId);
    if (!deleteResult)
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to delete telescope: %1").arg(deleteResult.error().errorMessage));
        return;
    }

    refreshData();
}

void TelescopesTab::onApertureOrFocalLengthChanged() const {
    const int aperture = ui->apertureSpinBox->value();
    const int focalLength = ui->focalLengthSpinBox->value();

    if (aperture > 0 && focalLength > 0)
    {
        const double fRatio = static_cast<double>(focalLength) / aperture;
        ui->fRatioSpinBox->setValue(fRatio);
    }
}
