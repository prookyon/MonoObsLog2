#include "tabs/camerastab.h"
#include "ui_cameras_tab.h"
#include "db/databasemanager.h"
#include "db/camerasrepository.h"
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

CamerasTab::CamerasTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::CamerasTab), m_dbManager(dbManager), m_repository(nullptr)
{
    ui->setupUi(this);
    m_repository = new CamerasRepository(m_dbManager, this);
}

CamerasTab::~CamerasTab()
{
    delete ui;
}

void CamerasTab::initialize()
{
    // Connect buttons to slots
    connect(ui->addCameraButton, &QPushButton::clicked, this, &CamerasTab::onAddCameraButtonClicked);
    connect(ui->editCameraButton, &QPushButton::clicked, this, &CamerasTab::onEditCameraButtonClicked);
    connect(ui->deleteCameraButton, &QPushButton::clicked, this, &CamerasTab::onDeleteCameraButtonClicked);

    // Configure table columns (without ID column)
    ui->camerasTable->setColumnWidth(0, 150); // Name
    ui->camerasTable->setColumnWidth(1, 150); // Sensor
    ui->camerasTable->setColumnWidth(2, 100); // Pixel Size
    ui->camerasTable->setColumnWidth(3, 80);  // Width
    ui->camerasTable->setColumnWidth(4, 80);  // Height

    // Initialize the tab UI and data
    refreshData();
}

void CamerasTab::refreshData()
{
    // Refresh data from database
    populateTable();
}

void CamerasTab::populateTable()
{
    ui->camerasTable->setSortingEnabled(false);
    ui->camerasTable->setRowCount(0);

    QString errorMessage;
    QVector<CameraData> cameras = m_repository->getAllCameras(errorMessage);

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to load cameras: %1").arg(errorMessage));
        return;
    }

    int row = 0;
    for (const CameraData &cam : cameras)
    {
        ui->camerasTable->insertRow(row);

        // Name column
        QTableWidgetItem *nameItem = new QTableWidgetItem(cam.name);
        nameItem->setData(Qt::UserRole, cam.id); // Store ID as hidden data
        ui->camerasTable->setItem(row, 0, nameItem);

        // Sensor column
        QTableWidgetItem *sensorItem = new QTableWidgetItem(cam.sensor);
        ui->camerasTable->setItem(row, 1, sensorItem);

        // Pixel Size column
        QTableWidgetItem *pixelSizeItem = new QTableWidgetItem(QString::number(cam.pixelSize, 'f', 2));
        pixelSizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->camerasTable->setItem(row, 2, pixelSizeItem);

        // Width column
        QTableWidgetItem *widthItem = new QTableWidgetItem(QString::number(cam.width));
        widthItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->camerasTable->setItem(row, 3, widthItem);

        // Height column
        QTableWidgetItem *heightItem = new QTableWidgetItem(QString::number(cam.height));
        heightItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->camerasTable->setItem(row, 4, heightItem);

        row++;
    }

    ui->camerasTable->setSortingEnabled(true);
}

bool CamerasTab::showCameraDialog(const QString &title, QString &name, QString &sensor, double &pixelSize, int &width, int &height)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setMinimumWidth(500);

    QFormLayout *formLayout = new QFormLayout(&dialog);

    // Name field
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    nameEdit->setText(name);
    nameEdit->setPlaceholderText("Enter camera name");
    formLayout->addRow("Name:", nameEdit);

    // Sensor field
    QLineEdit *sensorEdit = new QLineEdit(&dialog);
    sensorEdit->setText(sensor);
    sensorEdit->setPlaceholderText("Enter sensor type");
    formLayout->addRow("Sensor:", sensorEdit);

    // Pixel Size field
    QDoubleSpinBox *pixelSizeSpin = new QDoubleSpinBox(&dialog);
    pixelSizeSpin->setMaximum(100.0);
    pixelSizeSpin->setDecimals(2);
    pixelSizeSpin->setSingleStep(0.1);
    pixelSizeSpin->setValue(pixelSize);
    formLayout->addRow("Pixel Size (μm):", pixelSizeSpin);

    // Width field
    QSpinBox *widthSpin = new QSpinBox(&dialog);
    widthSpin->setMaximum(99999);
    widthSpin->setValue(width);
    formLayout->addRow("Width (px):", widthSpin);

    // Height field
    QSpinBox *heightSpin = new QSpinBox(&dialog);
    heightSpin->setMaximum(99999);
    heightSpin->setValue(height);
    formLayout->addRow("Height (px):", heightSpin);

    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formLayout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Validate name and sensor on OK
    connect(buttonBox, &QDialogButtonBox::accepted, [&]()
            {
        if (nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Validation Error", "Camera name cannot be empty.");
            dialog.done(QDialog::Rejected);
        }
        if (sensorEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Validation Error", "Sensor type cannot be empty.");
            dialog.done(QDialog::Rejected);
        } });

    if (dialog.exec() == QDialog::Accepted)
    {
        name = nameEdit->text().trimmed();
        sensor = sensorEdit->text().trimmed();
        pixelSize = pixelSizeSpin->value();
        width = widthSpin->value();
        height = heightSpin->value();

        return name.isEmpty() ? false : true;
    }

    return false;
}

void CamerasTab::onAddCameraButtonClicked()
{
    QString name = ui->nameLineEdit->text().trimmed();
    QString sensor = ui->sensorLineEdit->text().trimmed();
    double pixelSize = ui->pixelSizeSpinBox->value();
    int width = ui->widthSpinBox->value();
    int height = ui->heightSpinBox->value();

    // Validate inputs
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Validation Error", "Camera name cannot be empty.");
        return;
    }
    if (sensor.isEmpty())
    {
        QMessageBox::warning(this, "Validation Error", "Sensor type cannot be empty.");
        return;
    }

    // Insert into database using repository
    QString errorMessage;
    if (!m_repository->addCamera(name, sensor, pixelSize, width, height, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to add camera: %1").arg(errorMessage));
        return;
    }

    // Clear input fields
    ui->nameLineEdit->clear();
    ui->sensorLineEdit->clear();
    ui->pixelSizeSpinBox->setValue(0.0);
    ui->widthSpinBox->setValue(0);
    ui->heightSpinBox->setValue(0);

    refreshData();
}

void CamerasTab::onEditCameraButtonClicked()
{
    // Check if a row is selected
    int currentRow = ui->camerasTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select a camera to edit.");
        return;
    }

    // Get current camera data (ID is stored in UserRole)
    QTableWidgetItem *nameItem = ui->camerasTable->item(currentRow, 0);
    int cameraId = nameItem->data(Qt::UserRole).toInt();
    QString currentName = nameItem->text();
    QString currentSensor = ui->camerasTable->item(currentRow, 1)->text();
    double currentPixelSize = ui->camerasTable->item(currentRow, 2)->text().toDouble();
    int currentWidth = ui->camerasTable->item(currentRow, 3)->text().toInt();
    int currentHeight = ui->camerasTable->item(currentRow, 4)->text().toInt();

    // Show dialog
    QString name = currentName;
    QString sensor = currentSensor;
    double pixelSize = currentPixelSize;
    int width = currentWidth;
    int height = currentHeight;

    if (!showCameraDialog("Edit Camera", name, sensor, pixelSize, width, height))
    {
        return;
    }

    // Update in database using repository
    QString errorMessage;
    if (!m_repository->updateCamera(cameraId, name, sensor, pixelSize, width, height, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to update camera: %1").arg(errorMessage));
        return;
    }

    refreshData();
}

void CamerasTab::onDeleteCameraButtonClicked()
{
    // Check if a row is selected
    int currentRow = ui->camerasTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select a camera to delete.");
        return;
    }

    // Get camera data
    QTableWidgetItem *nameItem = ui->camerasTable->item(currentRow, 0);
    int cameraId = nameItem->data(Qt::UserRole).toInt();
    QString cameraName = nameItem->text();

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Deletion",
        QString("Are you sure you want to delete the camera '%1'?").arg(cameraName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    // Delete from database using repository
    QString errorMessage;
    if (!m_repository->deleteCamera(cameraId, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to delete camera: %1").arg(errorMessage));
        return;
    }

    refreshData();
}