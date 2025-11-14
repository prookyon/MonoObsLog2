#include "tabs/observationstab.h"
#include "ui_observations_tab.h"
#include "db/databasemanager.h"
#include "db/observationsrepository.h"
#include "numerictablewidgetitem.h"
#include <QDebug>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringListModel>

ObservationsTab::ObservationsTab(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::ObservationsTab), m_dbManager(dbManager),
      m_repository(nullptr), m_filterListModel(nullptr), m_currentFilterObjectId(-1)
{
    ui->setupUi(this);
    m_repository = new ObservationsRepository(m_dbManager, this);
    m_filterListModel = new QStringListModel(this);
    ui->observationFilterListView->setModel(m_filterListModel);
}

ObservationsTab::~ObservationsTab()
{
    delete ui;
}

void ObservationsTab::initialize()
{
    // Connect buttons to slots
    connect(ui->addObservationButton, &QPushButton::clicked, this, &ObservationsTab::onAddObservationButtonClicked);
    connect(ui->editObservationButton, &QPushButton::clicked, this, &ObservationsTab::onEditObservationButtonClicked);
    connect(ui->deleteObservationButton, &QPushButton::clicked, this, &ObservationsTab::onDeleteObservationButtonClicked);

    // Connect filter list view
    connect(ui->observationFilterListView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ObservationsTab::onFilterSelectionChanged);

    // Configure table columns
    ui->observationsTable->setColumnWidth(0, 120);  // Session Name
    ui->observationsTable->setColumnWidth(1, 90);   // Date
    ui->observationsTable->setColumnWidth(2, 120);  // Object
    ui->observationsTable->setColumnWidth(3, 100);  // Camera
    ui->observationsTable->setColumnWidth(4, 100);  // Telescope
    ui->observationsTable->setColumnWidth(5, 80);   // Filter
    ui->observationsTable->setColumnWidth(6, 60);   // Images
    ui->observationsTable->setColumnWidth(7, 80);   // Exposure (s)
    ui->observationsTable->setColumnWidth(8, 100);  // Total Exposure (s)
    ui->observationsTable->setColumnWidth(9, 80);   // Moon Illumination
    ui->observationsTable->setColumnWidth(10, 80);  // Angular Separation
    ui->observationsTable->setColumnWidth(11, 200); // Comments

    // Initialize data
    refreshData();
}

void ObservationsTab::refreshData()
{
    // Refresh combo boxes with latest data
    populateComboBoxes();

    // Refresh object filter list
    populateObjectFilter();

    // Refresh observations table
    populateTable();
}

void ObservationsTab::populateComboBoxes()
{
    // Save current selections
    int sessionIndex = ui->sessionNameComboBox->currentIndex();
    int objectIndex = ui->objectComboBox->currentIndex();
    int cameraIndex = ui->cameraComboBox->currentIndex();
    int telescopeIndex = ui->telescopeComboBox->currentIndex();
    int filterIndex = ui->filterComboBox->currentIndex();

    // Clear all combo boxes
    ui->sessionNameComboBox->clear();
    ui->objectComboBox->clear();
    ui->cameraComboBox->clear();
    ui->telescopeComboBox->clear();
    ui->filterComboBox->clear();

    QSqlQuery query(m_dbManager->database());

    // Populate Sessions
    if (query.exec("SELECT id, name FROM sessions ORDER BY name"))
    {
        while (query.next())
        {
            int id = query.value("id").toInt();
            QString name = query.value("name").toString();
            ui->sessionNameComboBox->addItem(name, id);
        }
    }

    // Populate Objects
    if (query.exec("SELECT id, name FROM objects ORDER BY name"))
    {
        while (query.next())
        {
            int id = query.value("id").toInt();
            QString name = query.value("name").toString();
            ui->objectComboBox->addItem(name, id);
        }
    }

    // Populate Cameras
    if (query.exec("SELECT id, name FROM cameras ORDER BY name"))
    {
        while (query.next())
        {
            int id = query.value("id").toInt();
            QString name = query.value("name").toString();
            ui->cameraComboBox->addItem(name, id);
        }
    }

    // Populate Telescopes
    if (query.exec("SELECT id, name FROM telescopes ORDER BY name"))
    {
        while (query.next())
        {
            int id = query.value("id").toInt();
            QString name = query.value("name").toString();
            ui->telescopeComboBox->addItem(name, id);
        }
    }

    // Populate Filters
    if (query.exec("SELECT id, name FROM filters ORDER BY name"))
    {
        while (query.next())
        {
            int id = query.value("id").toInt();
            QString name = query.value("name").toString();
            ui->filterComboBox->addItem(name, id);
        }
    }

    // Restore selections if possible
    if (sessionIndex >= 0 && sessionIndex < ui->sessionNameComboBox->count())
        ui->sessionNameComboBox->setCurrentIndex(sessionIndex);
    if (objectIndex >= 0 && objectIndex < ui->objectComboBox->count())
        ui->objectComboBox->setCurrentIndex(objectIndex);
    if (cameraIndex >= 0 && cameraIndex < ui->cameraComboBox->count())
        ui->cameraComboBox->setCurrentIndex(cameraIndex);
    if (telescopeIndex >= 0 && telescopeIndex < ui->telescopeComboBox->count())
        ui->telescopeComboBox->setCurrentIndex(telescopeIndex);
    if (filterIndex >= 0 && filterIndex < ui->filterComboBox->count())
        ui->filterComboBox->setCurrentIndex(filterIndex);
}

void ObservationsTab::populateObjectFilter()
{
    QStringList objectNames;
    objectNames << "< All Objects >";

    QSqlQuery query(m_dbManager->database());
    if (query.exec("SELECT DISTINCT objects.name FROM objects INNER JOIN observations ON objects.id = observations.object_id ORDER BY objects.name"))
    {
        while (query.next())
        {
            objectNames << query.value(0).toString();
        }
    }

    m_filterListModel->setStringList(objectNames);

    // Select "All Objects" by default if nothing is selected
    if (!ui->observationFilterListView->currentIndex().isValid())
    {
        ui->observationFilterListView->setCurrentIndex(m_filterListModel->index(0, 0));
    }
}

void ObservationsTab::populateTable()
{
    ui->observationsTable->setSortingEnabled(false);
    ui->observationsTable->setRowCount(0);

    QString errorMessage;
    QVector<ObservationData> observations;

    // Get observations based on current filter
    if (m_currentFilterObjectId == -1)
    {
        // Show all observations
        observations = m_repository->getAllObservations(errorMessage);
    }
    else
    {
        // Show filtered observations
        observations = m_repository->getObservationsByObject(m_currentFilterObjectId, errorMessage);
    }

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to load observations: %1").arg(errorMessage));
        return;
    }

    int row = 0;
    for (const ObservationData &obs : observations)
    {
        ui->observationsTable->insertRow(row);

        // Session Name
        QTableWidgetItem *sessionItem = new QTableWidgetItem(obs.sessionName);
        sessionItem->setData(Qt::UserRole, obs.id); // Store observation ID
        ui->observationsTable->setItem(row, 0, sessionItem);

        // Date
        QTableWidgetItem *dateItem = new QTableWidgetItem(obs.sessionDate);
        ui->observationsTable->setItem(row, 1, dateItem);

        // Object
        QTableWidgetItem *objectItem = new QTableWidgetItem(obs.objectName);
        ui->observationsTable->setItem(row, 2, objectItem);

        // Camera
        QTableWidgetItem *cameraItem = new QTableWidgetItem(obs.cameraName);
        ui->observationsTable->setItem(row, 3, cameraItem);

        // Telescope
        QTableWidgetItem *telescopeItem = new QTableWidgetItem(obs.telescopeName);
        ui->observationsTable->setItem(row, 4, telescopeItem);

        // Filter
        QTableWidgetItem *filterItem = new QTableWidgetItem(obs.filterName);
        ui->observationsTable->setItem(row, 5, filterItem);

        // Image Count
        NumericTableWidgetItem *imageCountItem = new NumericTableWidgetItem(QString::number(obs.imageCount));
        imageCountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->observationsTable->setItem(row, 6, imageCountItem);

        // Exposure Length
        NumericTableWidgetItem *exposureLengthItem = new NumericTableWidgetItem(QString::number(obs.exposureLength));
        exposureLengthItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->observationsTable->setItem(row, 7, exposureLengthItem);

        // Total Exposure
        NumericTableWidgetItem *totalExposureItem = new NumericTableWidgetItem(QString::number(obs.totalExposure));
        totalExposureItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->observationsTable->setItem(row, 8, totalExposureItem);

        // Moon Illumination
        NumericTableWidgetItem *moonIllumItem = new NumericTableWidgetItem(QString::number(obs.moonIllumination, 'f', 1));
        moonIllumItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->observationsTable->setItem(row, 9, moonIllumItem);

        // Angular Separation
        if (obs.angularSeparation >= 0.0)
        {
            NumericTableWidgetItem *angSepItem = new NumericTableWidgetItem(QString::number(obs.angularSeparation, 'f', 1));
            angSepItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->observationsTable->setItem(row, 10, angSepItem);
        }
        else
        {
            QTableWidgetItem *angSepItem = new QTableWidgetItem("N/A");
            angSepItem->setTextAlignment(Qt::AlignCenter);
            ui->observationsTable->setItem(row, 10, angSepItem);
        }

        // Comments
        QTableWidgetItem *commentsItem = new QTableWidgetItem(obs.comments);
        ui->observationsTable->setItem(row, 11, commentsItem);

        row++;
    }

    ui->observationsTable->setSortingEnabled(true);
}

void ObservationsTab::onFilterSelectionChanged()
{
    QModelIndex index = ui->observationFilterListView->currentIndex();
    if (!index.isValid())
        return;

    QString selectedObject = m_filterListModel->data(index, Qt::DisplayRole).toString();

    if (selectedObject == "< All Objects >")
    {
        m_currentFilterObjectId = -1;
    }
    else
    {
        // Find the object ID by name
        QSqlQuery query(m_dbManager->database());
        query.prepare("SELECT id FROM objects WHERE name = :name");
        query.bindValue(":name", selectedObject);

        if (query.exec() && query.next())
        {
            m_currentFilterObjectId = query.value(0).toInt();
        }
        else
        {
            m_currentFilterObjectId = -1;
        }
    }

    // Refresh the table with the new filter
    populateTable();
}

bool ObservationsTab::showObservationDialog(const QString &title, int &sessionId, int &objectId,
                                            int &cameraId, int &telescopeId, int &filterId,
                                            int &imageCount, int &exposureLength, QString &comments)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setMinimumWidth(500);

    QFormLayout *formLayout = new QFormLayout(&dialog);

    // Session combo box
    QComboBox *sessionCombo = new QComboBox(&dialog);
    QSqlQuery query(m_dbManager->database());
    if (query.exec("SELECT id, name FROM sessions ORDER BY name"))
    {
        while (query.next())
        {
            sessionCombo->addItem(query.value("name").toString(), query.value("id").toInt());
        }
    }
    // Set current value
    int sessionIndex = sessionCombo->findData(sessionId);
    if (sessionIndex >= 0)
        sessionCombo->setCurrentIndex(sessionIndex);
    formLayout->addRow("Session:", sessionCombo);

    // Object combo box
    QComboBox *objectCombo = new QComboBox(&dialog);
    if (query.exec("SELECT id, name FROM objects ORDER BY name"))
    {
        while (query.next())
        {
            objectCombo->addItem(query.value("name").toString(), query.value("id").toInt());
        }
    }
    int objectIndex = objectCombo->findData(objectId);
    if (objectIndex >= 0)
        objectCombo->setCurrentIndex(objectIndex);
    formLayout->addRow("Object:", objectCombo);

    // Camera combo box
    QComboBox *cameraCombo = new QComboBox(&dialog);
    if (query.exec("SELECT id, name FROM cameras ORDER BY name"))
    {
        while (query.next())
        {
            cameraCombo->addItem(query.value("name").toString(), query.value("id").toInt());
        }
    }
    int cameraIndex = cameraCombo->findData(cameraId);
    if (cameraIndex >= 0)
        cameraCombo->setCurrentIndex(cameraIndex);
    formLayout->addRow("Camera:", cameraCombo);

    // Telescope combo box
    QComboBox *telescopeCombo = new QComboBox(&dialog);
    if (query.exec("SELECT id, name FROM telescopes ORDER BY name"))
    {
        while (query.next())
        {
            telescopeCombo->addItem(query.value("name").toString(), query.value("id").toInt());
        }
    }
    int telescopeIndex = telescopeCombo->findData(telescopeId);
    if (telescopeIndex >= 0)
        telescopeCombo->setCurrentIndex(telescopeIndex);
    formLayout->addRow("Telescope:", telescopeCombo);

    // Filter combo box
    QComboBox *filterCombo = new QComboBox(&dialog);
    if (query.exec("SELECT id, name FROM filters ORDER BY name"))
    {
        while (query.next())
        {
            filterCombo->addItem(query.value("name").toString(), query.value("id").toInt());
        }
    }
    int filterIndex = filterCombo->findData(filterId);
    if (filterIndex >= 0)
        filterCombo->setCurrentIndex(filterIndex);
    formLayout->addRow("Filter:", filterCombo);

    // Image Count spin box
    QSpinBox *imageCountSpin = new QSpinBox(&dialog);
    imageCountSpin->setMaximum(99999);
    imageCountSpin->setValue(imageCount);
    formLayout->addRow("Image Count:", imageCountSpin);

    // Exposure Length spin box
    QSpinBox *exposureLengthSpin = new QSpinBox(&dialog);
    exposureLengthSpin->setMaximum(99999);
    exposureLengthSpin->setValue(exposureLength);
    formLayout->addRow("Exposure Length (s):", exposureLengthSpin);

    // Comments field
    QLineEdit *commentsEdit = new QLineEdit(&dialog);
    commentsEdit->setText(comments);
    commentsEdit->setPlaceholderText("Enter comments (optional)");
    formLayout->addRow("Comments:", commentsEdit);

    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formLayout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Validation on OK
    connect(buttonBox, &QDialogButtonBox::accepted, [&]()
            {
        if (sessionCombo->currentIndex() < 0)
        {
            QMessageBox::warning(&dialog, "Validation Error", "Please select a session.");
            dialog.done(QDialog::Rejected);
        }
        else if (objectCombo->currentIndex() < 0)
        {
            QMessageBox::warning(&dialog, "Validation Error", "Please select an object.");
            dialog.done(QDialog::Rejected);
        }
        else if (cameraCombo->currentIndex() < 0)
        {
            QMessageBox::warning(&dialog, "Validation Error", "Please select a camera.");
            dialog.done(QDialog::Rejected);
        }
        else if (telescopeCombo->currentIndex() < 0)
        {
            QMessageBox::warning(&dialog, "Validation Error", "Please select a telescope.");
            dialog.done(QDialog::Rejected);
        }
        else if (filterCombo->currentIndex() < 0)
        {
            QMessageBox::warning(&dialog, "Validation Error", "Please select a filter.");
            dialog.done(QDialog::Rejected);
        }
        else if (imageCountSpin->value() <= 0)
        {
            QMessageBox::warning(&dialog, "Validation Error", "Image count must be greater than 0.");
            dialog.done(QDialog::Rejected);
        }
        else if (exposureLengthSpin->value() <= 0)
        {
            QMessageBox::warning(&dialog, "Validation Error", "Exposure length must be greater than 0.");
            dialog.done(QDialog::Rejected);
        } });

    if (dialog.exec() == QDialog::Accepted)
    {
        sessionId = sessionCombo->currentData().toInt();
        objectId = objectCombo->currentData().toInt();
        cameraId = cameraCombo->currentData().toInt();
        telescopeId = telescopeCombo->currentData().toInt();
        filterId = filterCombo->currentData().toInt();
        imageCount = imageCountSpin->value();
        exposureLength = exposureLengthSpin->value();
        comments = commentsEdit->text().trimmed();
        return true;
    }

    return false;
}

void ObservationsTab::onAddObservationButtonClicked()
{
    // Get values from input fields
    int sessionId = ui->sessionNameComboBox->currentData().toInt();
    int objectId = ui->objectComboBox->currentData().toInt();
    int cameraId = ui->cameraComboBox->currentData().toInt();
    int telescopeId = ui->telescopeComboBox->currentData().toInt();
    int filterId = ui->filterComboBox->currentData().toInt();
    int imageCount = ui->imageCountSpinBox->value();
    int exposureLength = ui->exposureLengthSpinBox->value();
    QString comments = ui->commentsLineEdit->text().trimmed();

    // Validate inputs
    if (ui->sessionNameComboBox->currentIndex() < 0)
    {
        QMessageBox::warning(this, "Validation Error", "Please select a session.");
        return;
    }
    if (ui->objectComboBox->currentIndex() < 0)
    {
        QMessageBox::warning(this, "Validation Error", "Please select an object.");
        return;
    }
    if (ui->cameraComboBox->currentIndex() < 0)
    {
        QMessageBox::warning(this, "Validation Error", "Please select a camera.");
        return;
    }
    if (ui->telescopeComboBox->currentIndex() < 0)
    {
        QMessageBox::warning(this, "Validation Error", "Please select a telescope.");
        return;
    }
    if (ui->filterComboBox->currentIndex() < 0)
    {
        QMessageBox::warning(this, "Validation Error", "Please select a filter.");
        return;
    }
    if (imageCount <= 0)
    {
        QMessageBox::warning(this, "Validation Error", "Image count must be greater than 0.");
        return;
    }
    if (exposureLength <= 0)
    {
        QMessageBox::warning(this, "Validation Error", "Exposure length must be greater than 0.");
        return;
    }

    // Add to database using repository
    QString errorMessage;
    if (!m_repository->addObservation(imageCount, exposureLength, comments,
                                      sessionId, objectId, cameraId, telescopeId, filterId,
                                      errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to add observation: %1").arg(errorMessage));
        return;
    }

    // Clear input fields
    ui->imageCountSpinBox->setValue(0);
    ui->exposureLengthSpinBox->setValue(0);
    ui->commentsLineEdit->clear();

    // Refresh data
    refreshData();
}

void ObservationsTab::onEditObservationButtonClicked()
{
    // Check if a row is selected
    int currentRow = ui->observationsTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select an observation to edit.");
        return;
    }

    // Get observation ID from the table
    QTableWidgetItem *sessionItem = ui->observationsTable->item(currentRow, 0);
    int observationId = sessionItem->data(Qt::UserRole).toInt();

    // Find the observation data
    QString errorMessage;
    QVector<ObservationData> allObs = m_repository->getAllObservations(errorMessage);

    ObservationData currentObs;
    bool found = false;
    for (const ObservationData &obs : allObs)
    {
        if (obs.id == observationId)
        {
            currentObs = obs;
            found = true;
            break;
        }
    }

    if (!found)
    {
        QMessageBox::warning(this, "Error", "Could not find observation data.");
        return;
    }

    // Show dialog with current values
    int sessionId = currentObs.sessionId;
    int objectId = currentObs.objectId;
    int cameraId = currentObs.cameraId;
    int telescopeId = currentObs.telescopeId;
    int filterId = currentObs.filterId;
    int imageCount = currentObs.imageCount;
    int exposureLength = currentObs.exposureLength;
    QString comments = currentObs.comments;

    if (!showObservationDialog("Edit Observation", sessionId, objectId, cameraId, telescopeId,
                               filterId, imageCount, exposureLength, comments))
    {
        return;
    }

    // Update in database
    if (!m_repository->updateObservation(observationId, imageCount, exposureLength, comments,
                                         sessionId, objectId, cameraId, telescopeId, filterId,
                                         errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to update observation: %1").arg(errorMessage));
        return;
    }

    refreshData();
}

void ObservationsTab::onDeleteObservationButtonClicked()
{
    // Check if a row is selected
    int currentRow = ui->observationsTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select an observation to delete.");
        return;
    }

    // Get observation data
    QTableWidgetItem *sessionItem = ui->observationsTable->item(currentRow, 0);
    int observationId = sessionItem->data(Qt::UserRole).toInt();
    QString sessionName = sessionItem->text();
    QString objectName = ui->observationsTable->item(currentRow, 2)->text();

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Deletion",
        QString("Are you sure you want to delete the observation of '%1' in session '%2'?")
            .arg(objectName)
            .arg(sessionName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    // Delete from database
    QString errorMessage;
    if (!m_repository->deleteObservation(observationId, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to delete observation: %1").arg(errorMessage));
        return;
    }

    refreshData();
}