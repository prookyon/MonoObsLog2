#include "tabs/telescopestab.h"
#include "ui_telescopes_tab.h"
#include "db/databasemanager.h"
#include "db/telescopesrepository.h"
#include <QDebug>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
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

    QString errorMessage;
    QVector<TelescopeData> telescopes = m_repository->getAllTelescopes(errorMessage);

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to load telescopes: %1").arg(errorMessage));
        return;
    }

    int row = 0;
    for (const TelescopeData &tel : telescopes)
    {
        ui->telescopesTable->insertRow(row);

        // Name column
        QTableWidgetItem *nameItem = new QTableWidgetItem(tel.name);
        nameItem->setData(Qt::UserRole, tel.id); // Store ID as hidden data
        ui->telescopesTable->setItem(row, 0, nameItem);

        // Aperture column
        QTableWidgetItem *apertureItem = new QTableWidgetItem(QString::number(tel.aperture));
        apertureItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->telescopesTable->setItem(row, 1, apertureItem);

        // F-ratio column
        QTableWidgetItem *fRatioItem = new QTableWidgetItem(QString::number(tel.fRatio, 'f', 2));
        fRatioItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->telescopesTable->setItem(row, 2, fRatioItem);

        // Focal Length column
        QTableWidgetItem *focalLengthItem = new QTableWidgetItem(QString::number(tel.focalLength));
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

    QFormLayout *formLayout = new QFormLayout(&dialog);

    // Name field
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    nameEdit->setText(name);
    nameEdit->setPlaceholderText("Enter telescope name");
    formLayout->addRow("Name:", nameEdit);

    // Aperture field
    QSpinBox *apertureSpin = new QSpinBox(&dialog);
    apertureSpin->setMaximum(99999);
    apertureSpin->setValue(aperture);
    formLayout->addRow("Aperture (mm):", apertureSpin);

    // F-ratio field
    QDoubleSpinBox *fRatioSpin = new QDoubleSpinBox(&dialog);
    fRatioSpin->setMaximum(100.0);
    fRatioSpin->setDecimals(2);
    fRatioSpin->setSingleStep(0.1);
    fRatioSpin->setValue(fRatio);
    formLayout->addRow("F-ratio:", fRatioSpin);

    // Focal Length field
    QSpinBox *focalLengthSpin = new QSpinBox(&dialog);
    focalLengthSpin->setMaximum(99999);
    focalLengthSpin->setValue(focalLength);
    formLayout->addRow("Focal Length (mm):", focalLengthSpin);

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
            QMessageBox::warning(&dialog, "Validation Error", "Telescope name cannot be empty.");
            dialog.done(QDialog::Rejected);
        } });

    if (dialog.exec() == QDialog::Accepted)
    {
        name = nameEdit->text().trimmed();
        aperture = apertureSpin->value();
        fRatio = fRatioSpin->value();
        focalLength = focalLengthSpin->value();

        return name.isEmpty() ? false : true;
    }

    return false;
}

void TelescopesTab::onAddTelescopeButtonClicked()
{
    QString name = ui->nameLineEdit->text().trimmed();
    int aperture = ui->apertureSpinBox->value();
    double fRatio = ui->fRatioSpinBox->value();
    int focalLength = ui->focalLengthSpinBox->value();

    // Validate inputs
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Validation Error", "Telescope name cannot be empty.");
        return;
    }

    // Insert into database using repository
    QString errorMessage;
    if (!m_repository->addTelescope(name, aperture, fRatio, focalLength, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to add telescope: %1").arg(errorMessage));
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
    int currentRow = ui->telescopesTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select a telescope to edit.");
        return;
    }

    // Get current telescope data (ID is stored in UserRole)
    QTableWidgetItem *nameItem = ui->telescopesTable->item(currentRow, 0);
    int telescopeId = nameItem->data(Qt::UserRole).toInt();
    QString currentName = nameItem->text();
    int currentAperture = ui->telescopesTable->item(currentRow, 1)->text().toInt();
    double currentFRatio = ui->telescopesTable->item(currentRow, 2)->text().toDouble();
    int currentFocalLength = ui->telescopesTable->item(currentRow, 3)->text().toInt();

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
    QString errorMessage;
    if (!m_repository->updateTelescope(telescopeId, name, aperture, fRatio, focalLength, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to update telescope: %1").arg(errorMessage));
        return;
    }

    refreshData();
}

void TelescopesTab::onDeleteTelescopeButtonClicked()
{
    // Check if a row is selected
    int currentRow = ui->telescopesTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select a telescope to delete.");
        return;
    }

    // Get telescope data
    QTableWidgetItem *nameItem = ui->telescopesTable->item(currentRow, 0);
    int telescopeId = nameItem->data(Qt::UserRole).toInt();
    QString telescopeName = nameItem->text();

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
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
    QString errorMessage;
    if (!m_repository->deleteTelescope(telescopeId, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to delete telescope: %1").arg(errorMessage));
        return;
    }

    refreshData();
}