#include "tabs/observationstab.h"
#include "ui_observations_tab.h"
#include "db/databasemanager.h"
#include "db/observationsrepository.h"
#include "numerictablewidgetitem.h"
#include "settingsmanager.h"
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
#include <QStringListModel>
#include <QMenu>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <OpenXLSX.hpp>
#include <QDesktopServices>

ObservationsTab::ObservationsTab(DatabaseManager *dbManager, SettingsManager *settingsManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::ObservationsTab), m_dbManager(dbManager), m_settingsManager(settingsManager),
      m_repository(nullptr), m_filterListModel(nullptr), m_exportMenu(nullptr), m_currentFilterObjectId(-1)
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

    m_rightClickMenu = new QMenu(ui->observationsTable);
    const auto openExplorerAction = m_rightClickMenu->addAction("Browse to Session folder...");
    connect(openExplorerAction, &QAction::triggered, this, &ObservationsTab::onExploreSessionsFolder);

    ui->observationsTable->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(ui->observationsTable,&QTableWidget::customContextMenuRequested,this,[this]{
        if (m_settingsManager->sessionsFolderTemplate().isEmpty())
            return;
        m_rightClickMenu->popup(QCursor::pos());
    });

    // Setup export menu
    m_exportMenu = new QMenu(this);
    const auto exportToHtmlAction = m_exportMenu->addAction("Export to HTML");
    const auto exportToExcelAction = m_exportMenu->addAction("Export to Excel");
    connect(exportToHtmlAction, &QAction::triggered, this, &ObservationsTab::onExportToHtmlClicked);
    connect(exportToExcelAction, &QAction::triggered, this, &ObservationsTab::onExportToExcelClicked);
    ui->exportObservationButton->setMenu(m_exportMenu);


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

void ObservationsTab::populateComboBoxes() const {
    // Save current selections
    // TODO: this is very naive, maybe use actual user data to restore selected item
    const int sessionIndex = ui->sessionNameComboBox->currentIndex();
    const int objectIndex = ui->objectComboBox->currentIndex();
    const int cameraIndex = ui->cameraComboBox->currentIndex();
    const int telescopeIndex = ui->telescopeComboBox->currentIndex();
    const int filterIndex = ui->filterComboBox->currentIndex();

    // Clear all combo boxes
    ui->sessionNameComboBox->clear();
    ui->objectComboBox->clear();
    ui->cameraComboBox->clear();
    ui->telescopeComboBox->clear();
    ui->filterComboBox->clear();

    QSqlQuery query(m_dbManager->database());

    // Populate Sessions
    if (query.exec("SELECT id, concat(name,' (', start_date, ')') as name FROM sessions ORDER BY start_date DESC"))
    {
        while (query.next())
        {
            const int id = query.value("id").toInt();
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
    if (query.exec("SELECT f.id, f.name FROM filters f LEFT JOIN filter_types ft on f.filter_type_id = ft.id ORDER BY ft.priority ASC, f.name ASC"))
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

void ObservationsTab::populateObjectFilter() const {
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
        const auto sessionItem = new QTableWidgetItem(obs.sessionName);
        sessionItem->setData(Qt::UserRole, obs.id); // Store observation ID
        ui->observationsTable->setItem(row, 0, sessionItem);

        // Date
        const auto dateItem = new QTableWidgetItem(obs.sessionDate);
        ui->observationsTable->setItem(row, 1, dateItem);

        // Object
        const auto objectItem = new QTableWidgetItem(obs.objectName);
        ui->observationsTable->setItem(row, 2, objectItem);

        // Camera
        const auto cameraItem = new QTableWidgetItem(obs.cameraName);
        ui->observationsTable->setItem(row, 3, cameraItem);

        // Telescope
        const auto telescopeItem = new QTableWidgetItem(obs.telescopeName);
        ui->observationsTable->setItem(row, 4, telescopeItem);

        // Filter
        const auto filterItem = new QTableWidgetItem(obs.filterName);
        ui->observationsTable->setItem(row, 5, filterItem);

        // Image Count
        const auto imageCountItem = new NumericTableWidgetItem(QString::number(obs.imageCount));
        imageCountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->observationsTable->setItem(row, 6, imageCountItem);

        // Exposure Length
        const auto exposureLengthItem = new NumericTableWidgetItem(QString::number(obs.exposureLength));
        exposureLengthItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->observationsTable->setItem(row, 7, exposureLengthItem);

        // Total Exposure
        const auto totalExposureItem = new NumericTableWidgetItem(QString::number(obs.totalExposure));
        totalExposureItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->observationsTable->setItem(row, 8, totalExposureItem);

        // Moon Illumination
        const auto moonIllumItem = new NumericTableWidgetItem(QString::number(obs.moonIllumination, 'f', 0));
        moonIllumItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        // Apply background color if moon illumination exceeds warning threshold
        if (m_settingsManager)
        {
            int warningThreshold = m_settingsManager->moonIlluminationWarningPercent();
            if (obs.moonIllumination > warningThreshold)
            {
                QColor warningColor = QColor::fromHsv(0, 40 * 255 / 100, 95 * 255 / 100);
                moonIllumItem->setBackground(warningColor);
            }
        }
        ui->observationsTable->setItem(row, 9, moonIllumItem);

        // Angular Separation
        if (obs.angularSeparation >= 0.0)
        {
            const auto angSepItem = new NumericTableWidgetItem(QString::number(obs.angularSeparation, 'f', 0));
            angSepItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            // Apply background color if angular separation is smaller than warning threshold
            if (m_settingsManager)
            {
                int warningThreshold = m_settingsManager->moonAngularSeparationWarningDeg();
                if (obs.angularSeparation < warningThreshold)
                {
                    QColor warningColor = QColor::fromHsv(0, 40 * 255 / 100, 95 * 255 / 100);
                    angSepItem->setBackground(warningColor);
                }
            }
            ui->observationsTable->setItem(row, 10, angSepItem);
        }
        else
        {
            const auto angSepItem = new QTableWidgetItem("");
            angSepItem->setTextAlignment(Qt::AlignCenter);
            ui->observationsTable->setItem(row, 10, angSepItem);
        }

        // Comments
        const auto commentsItem = new QTableWidgetItem(obs.comments);
        ui->observationsTable->setItem(row, 11, commentsItem);

        row++;
    }

    ui->observationsTable->setSortingEnabled(true);
    ui->observationsTable->resizeColumnsToContents();
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

    const auto formLayout = new QFormLayout(&dialog);

    // Session combo box
    const auto sessionCombo = new QComboBox(&dialog);
    QSqlQuery query(m_dbManager->database());
    if (query.exec("SELECT id, concat(name,' (', start_date, ')') as name FROM sessions ORDER BY start_date DESC"))
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
    const auto objectCombo = new QComboBox(&dialog);
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
    const auto cameraCombo = new QComboBox(&dialog);
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
    const auto telescopeCombo = new QComboBox(&dialog);
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
    const auto filterCombo = new QComboBox(&dialog);
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
    const auto imageCountSpin = new QSpinBox(&dialog);
    imageCountSpin->setMaximum(99999);
    imageCountSpin->setValue(imageCount);
    formLayout->addRow("Image Count:", imageCountSpin);

    // Exposure Length spin box
    const auto exposureLengthSpin = new QSpinBox(&dialog);
    exposureLengthSpin->setMaximum(99999);
    exposureLengthSpin->setValue(exposureLength);
    formLayout->addRow("Exposure Length (s):", exposureLengthSpin);

    // Comments field
    const auto commentsEdit = new QLineEdit(&dialog);
    commentsEdit->setText(comments);
    commentsEdit->setPlaceholderText("Enter comments (optional)");
    formLayout->addRow("Comments:", commentsEdit);

    // Buttons
    const auto buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formLayout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Validation on OK
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, [&dialog, sessionCombo,objectCombo,cameraCombo,telescopeCombo,filterCombo,imageCountSpin,exposureLengthSpin]()
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
    const int sessionId = ui->sessionNameComboBox->currentData().toInt();
    const int objectId = ui->objectComboBox->currentData().toInt();
    const int cameraId = ui->cameraComboBox->currentData().toInt();
    const int telescopeId = ui->telescopeComboBox->currentData().toInt();
    const int filterId = ui->filterComboBox->currentData().toInt();
    const int imageCount = ui->imageCountSpinBox->value();
    const int exposureLength = ui->exposureLengthSpinBox->value();
    const QString comments = ui->commentsLineEdit->text().trimmed();

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
    if (QString errorMessage; !m_repository->addObservation(imageCount, exposureLength, comments,
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
    const int currentRow = ui->observationsTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select an observation to edit.");
        return;
    }

    // Get observation ID from the table
    const QTableWidgetItem *sessionItem = ui->observationsTable->item(currentRow, 0);
    const int observationId = sessionItem->data(Qt::UserRole).toInt();

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
    const int currentRow = ui->observationsTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select an observation to delete.");
        return;
    }

    // Get observation data
    const QTableWidgetItem *sessionItem = ui->observationsTable->item(currentRow, 0);
    const int observationId = sessionItem->data(Qt::UserRole).toInt();
    QString sessionName = sessionItem->text();
    QString objectName = ui->observationsTable->item(currentRow, 2)->text();

    // Confirm deletion
    const QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Deletion",
        QString("Are you sure you want to delete the observation of '%1' in session '%2'?")
            .arg(objectName, sessionName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    // Delete from database
    if (QString errorMessage; !m_repository->deleteObservation(observationId, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to delete observation: %1").arg(errorMessage));
        return;
    }

    refreshData();
}

void ObservationsTab::onExportToHtmlClicked()
{
    // Ask user for save location
    const QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Observations to HTML",
        QDir::homePath() + "/observations_export.html",
        "HTML Files (*.html);;All Files (*)");

    if (fileName.isEmpty())
    {
        return; // User cancelled
    }

    // Read the template file
    QFile templateFile(":/templates/observations_export.html");
    if (!templateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Export Error",
                             "Failed to open template file: " + templateFile.errorString());
        return;
    }

    QString templateContent = QTextStream(&templateFile).readAll();
    templateFile.close();

    // Get current observations (respecting filter)
    QString errorMessage;
    QVector<ObservationData> observations;

    if (m_currentFilterObjectId == -1)
    {
        observations = m_repository->getAllObservations(errorMessage);
    }
    else
    {
        observations = m_repository->getObservationsByObject(m_currentFilterObjectId, errorMessage);
    }

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(this, "Export Error",
                             "Failed to load observations: " + errorMessage);
        return;
    }

    // Generate table rows HTML
    QString tableRowsHtml;
    const int moonWarningThreshold = m_settingsManager ? m_settingsManager->moonIlluminationWarningPercent() : 75;
    const int angularWarningThreshold = m_settingsManager ? m_settingsManager->moonAngularSeparationWarningDeg() : 60;

    for (const ObservationData &obs : observations)
    {
        tableRowsHtml += "                <tr>\n";
        tableRowsHtml += QString("                    <td>%1</td>\n").arg(obs.sessionName);
        tableRowsHtml += QString("                    <td>%1</td>\n").arg(obs.sessionDate);
        tableRowsHtml += QString("                    <td>%1</td>\n").arg(obs.objectName);
        tableRowsHtml += QString("                    <td>%1</td>\n").arg(obs.cameraName);
        tableRowsHtml += QString("                    <td>%1</td>\n").arg(obs.telescopeName);
        tableRowsHtml += QString("                    <td>%1</td>\n").arg(obs.filterName);
        tableRowsHtml += QString("                    <td>%1</td>\n").arg(obs.imageCount);
        tableRowsHtml += QString("                    <td>%1</td>\n").arg(obs.exposureLength);
        tableRowsHtml += QString("                    <td>%1</td>\n").arg(obs.totalExposure);

        // Moon illumination with warning class
        QString moonClass = obs.moonIllumination > moonWarningThreshold ? " class=\"moon-warning\"" : "";
        tableRowsHtml += QString("                    <td%1>%2</td>\n")
                             .arg(moonClass, QString::number(obs.moonIllumination, 'f', 0));

        // Angular separation with warning class
        if (obs.angularSeparation >= 0.0)
        {
            QString angularClass = obs.angularSeparation < angularWarningThreshold ? " class=\"angular-warning\"" : "";
            tableRowsHtml += QString("                    <td%1>%2</td>\n")
                                 .arg(angularClass, QString::number(obs.angularSeparation, 'f', 0));
        }
        else
        {
            tableRowsHtml += "                    <td></td>\n";
        }

        tableRowsHtml += QString("                    <td>%1</td>\n").arg(obs.comments);
        tableRowsHtml += "                </tr>\n";
    }

    // Determine filter name
    QString filterName = "All Objects";
    if (m_currentFilterObjectId != -1)
    {
        QModelIndex index = ui->observationFilterListView->currentIndex();
        if (index.isValid())
        {
            filterName = m_filterListModel->data(index, Qt::DisplayRole).toString();
        }
    }

    // Get current date/time
    const QString exportDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    // Replace placeholders
    templateContent.replace("{{filter_name}}", filterName);
    templateContent.replace("{{export_date}}", exportDateTime);
    templateContent.replace("{{total_records}}", QString::number(observations.size()));
    templateContent.replace("{{table_rows}}", tableRowsHtml);
    templateContent.replace("{{completion_date}}", exportDateTime);

    // Write to file
    QFile outputFile(fileName);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Export Error",
                             "Failed to write to file: " + outputFile.errorString());
        return;
    }

    QTextStream out(&outputFile);
    out << templateContent;
    outputFile.close();

    QMessageBox::information(this, "Export Successful",
                             QString("Exported %1 observations to:\n%2")
                                 .arg(observations.size())
                                 .arg(fileName));
}

void ObservationsTab::onExportToExcelClicked() {
    // Ask user for save location
    const QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Observations to Excel",
        QDir::homePath() + "/observations_export.xlsx",
        "Excel Files (*.xlsx);;All Files (*)");

    if (fileName.isEmpty())
    {
        return; // User cancelled
    }

    // Get current observations (respecting filter)
    QString errorMessage;
    QVector<ObservationData> observations;

    if (m_currentFilterObjectId == -1)
    {
        observations = m_repository->getAllObservations(errorMessage);
    }
    else
    {
        observations = m_repository->getObservationsByObject(m_currentFilterObjectId, errorMessage);
    }

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(this, "Export Error",
                             "Failed to load observations: " + errorMessage);
        return;
    }

    try
    {
        // Create a new Excel document
        OpenXLSX::XLDocument doc;
        doc.create(fileName.toStdString(), true);
        auto wks = doc.workbook().worksheet("Sheet1");

        // Set worksheet name
        wks.setName("Observations");

        // Determine filter name
        QString filterName = "All Objects";
        if (m_currentFilterObjectId != -1)
        {
            QModelIndex index = ui->observationFilterListView->currentIndex();
            if (index.isValid())
            {
                filterName = m_filterListModel->data(index, Qt::DisplayRole).toString();
            }
        }

        // Get current date/time
        const QString exportDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        // Add export information at the top
        wks.cell("A1").value() = "Observations Export";
        wks.cell("A2").value() = "Filter: " + filterName.toStdString();
        wks.cell("A3").value() = "Export Date: " + exportDateTime.toStdString();
        wks.cell("A4").value() = "Total Records: " + std::to_string(observations.size());

        // Add header row (starting at row 6)
        constexpr int headerRow = 6;
        wks.cell(headerRow, 1).value() = "Session Name";
        wks.cell(headerRow, 2).value() = "Date";
        wks.cell(headerRow, 3).value() = "Object";
        wks.cell(headerRow, 4).value() = "Camera";
        wks.cell(headerRow, 5).value() = "Telescope";
        wks.cell(headerRow, 6).value() = "Filter";
        wks.cell(headerRow, 7).value() = "Images";
        wks.cell(headerRow, 8).value() = "Exposure (s)";
        wks.cell(headerRow, 9).value() = "Total Exposure (s)";
        wks.cell(headerRow, 10).value() = "Moon Phase (%)";
        wks.cell(headerRow, 11).value() = "Angular Separation";
        wks.cell(headerRow, 12).value() = "Comments";

        // Add data rows
        int currentRow = headerRow + 1;
        for (const ObservationData &obs : observations)
        {
            wks.cell(currentRow, 1).value() = obs.sessionName.toStdString();
            wks.cell(currentRow, 2).value() = obs.sessionDate.toStdString();
            wks.cell(currentRow, 3).value() = obs.objectName.toStdString();
            wks.cell(currentRow, 4).value() = obs.cameraName.toStdString();
            wks.cell(currentRow, 5).value() = obs.telescopeName.toStdString();
            wks.cell(currentRow, 6).value() = obs.filterName.toStdString();
            wks.cell(currentRow, 7).value() = obs.imageCount;
            wks.cell(currentRow, 8).value() = obs.exposureLength;
            wks.cell(currentRow, 9).value() = obs.totalExposure;
            wks.cell(currentRow, 10).value() = obs.moonIllumination;
            
            // Angular separation (may be negative if not calculated)
            if (obs.angularSeparation >= 0.0)
            {
                wks.cell(currentRow, 11).value() = obs.angularSeparation;
            }
            else
            {
                wks.cell(currentRow, 11).value() = "";
            }
            
            wks.cell(currentRow, 12).value() = obs.comments.toStdString();

            currentRow++;
        }
        // Auto-fit columns (set reasonable widths)
        wks.column(1).setWidth(20);  // Session Name
        wks.column(2).setWidth(12);  // Date
        wks.column(3).setWidth(20);  // Object
        wks.column(4).setWidth(15);  // Camera
        wks.column(5).setWidth(15);  // Telescope
        wks.column(6).setWidth(12);  // Filter
        wks.column(7).setWidth(10);  // Images
        wks.column(8).setWidth(14);  // Exposure
        wks.column(9).setWidth(18);  // Total Exposure
        wks.column(10).setWidth(16); // Moon Phase
        wks.column(11).setWidth(18); // Angular Separation
        wks.column(12).setWidth(30); // Comments

        // Save the document
        doc.save();
        doc.close();

        QMessageBox::information(this, "Export Successful",
                                 QString("Exported %1 observations to:\n%2")
                                     .arg(observations.size())
                                     .arg(fileName));
    }
    catch (const std::exception &e)
    {
        QMessageBox::warning(this, "Export Error",
                             QString("Failed to create Excel file: %1").arg(e.what()));
    }
}

void ObservationsTab::onExploreSessionsFolder() {
    const int currentRow = ui->observationsTable->currentRow();
    const QString sessionId = ui->observationsTable->item(currentRow, 0)->text();
    QString folderTemplate = m_settingsManager->sessionsFolderTemplate();
    const QString finalPath = folderTemplate.replace("{{sessionid}}", sessionId);
    //Test path exists
    if (QFileInfo::exists(folderTemplate)) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(folderTemplate));
    }
    else {
        QMessageBox::warning(this, "Folder not found", QString("Session folder %1 does not exist").arg(folderTemplate));
    }
}
