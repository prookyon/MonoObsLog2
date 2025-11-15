#include "tabs/sessionstab.h"
#include "ui_sessions_tab.h"
#include "settingsmanager.h"
#include "db/databasemanager.h"
#include "db/sessionsrepository.h"
#include "numerictablewidgetitem.h"
#include <QDebug>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QColor>

SessionsTab::SessionsTab(DatabaseManager *dbManager, SettingsManager *settingsManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::SessionsTab), m_dbManager(dbManager), m_settingsManager(settingsManager), m_repository(nullptr)
{
    ui->setupUi(this);
    m_repository = new SessionsRepository(m_dbManager, this);
}

SessionsTab::~SessionsTab()
{
    delete ui;
}

void SessionsTab::initialize()
{
    // Connect buttons to slots
    connect(ui->addSessionButton, &QPushButton::clicked, this, &SessionsTab::onAddButtonClicked);
    connect(ui->editSessionButton, &QPushButton::clicked, this, &SessionsTab::onEditButtonClicked);
    connect(ui->deleteSessionButton, &QPushButton::clicked, this, &SessionsTab::onDeleteButtonClicked);

    // Configure table columns
    ui->sessionsTable->setColumnWidth(0, 150); // Session Name
    ui->sessionsTable->setColumnWidth(1, 120); // Start Date
    ui->sessionsTable->setColumnWidth(2, 150); // Moon Illumination
    ui->sessionsTable->setColumnWidth(3, 100); // Moon RA
    ui->sessionsTable->setColumnWidth(4, 100); // Moon Dec
    ui->sessionsTable->setColumnWidth(5, 150); // Exposure Total (h)
    ui->sessionsTable->setColumnWidth(6, 200); // Comments

    // Initialize the tab UI and data
    ui->startDateEdit->setDate(QDate::currentDate());
    refreshData();
}

void SessionsTab::refreshData()
{
    // Refresh data from database
    populateTable();
}

void SessionsTab::populateTable()
{
    ui->sessionsTable->setSortingEnabled(false);
    ui->sessionsTable->setRowCount(0);

    QString errorMessage;
    QVector<SessionData> sessions = m_repository->getAllSessions(errorMessage);

    if (!errorMessage.isEmpty())
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to load sessions: %1").arg(errorMessage));
        return;
    }

    int row = 0;
    for (const SessionData &session : sessions)
    {
        ui->sessionsTable->insertRow(row);

        // Session Name column
        QTableWidgetItem *nameItem = new QTableWidgetItem(session.name);
        nameItem->setData(Qt::UserRole, session.id); // Store ID as hidden data
        ui->sessionsTable->setItem(row, 0, nameItem);

        // Start Date column
        QString dateText = session.startDate.toString("yyyy-MM-dd");
        QTableWidgetItem *dateItem = new QTableWidgetItem(dateText);
        ui->sessionsTable->setItem(row, 1, dateItem);

        // Moon Illumination column
        QString moonIllumText = session.moonIllumination.isNull() ? "" : QString::number(session.moonIllumination.toDouble(), 'f', 0);
        NumericTableWidgetItem *moonIllumItem = new NumericTableWidgetItem(moonIllumText);
        moonIllumItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        // Apply background color if moon illumination exceeds warning threshold
        if (!session.moonIllumination.isNull() && m_settingsManager)
        {
            double illumination = session.moonIllumination.toDouble();
            int warningThreshold = m_settingsManager->moonIlluminationWarningPercent();
            if (illumination > warningThreshold)
            {
                QColor warningColor = QColor::fromHsv(0, 40 * 255 / 100, 95 * 255 / 100);
                moonIllumItem->setBackground(warningColor);
            }
        }

        ui->sessionsTable->setItem(row, 2, moonIllumItem);

        // Moon RA column
        QString moonRaText = session.moonRa.isNull() ? "" : QString::number(session.moonRa.toDouble(), 'f', 2);
        NumericTableWidgetItem *moonRaItem = new NumericTableWidgetItem(moonRaText);
        moonRaItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->sessionsTable->setItem(row, 3, moonRaItem);

        // Moon Dec column
        QString moonDecText = session.moonDec.isNull() ? "" : QString::number(session.moonDec.toDouble(), 'f', 2);
        NumericTableWidgetItem *moonDecItem = new NumericTableWidgetItem(moonDecText);
        moonDecItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->sessionsTable->setItem(row, 4, moonDecItem);

        // Exposure Total (h) column
        QString exposureText = session.exposureTotal.isNull() ? "0" : QString::number(session.exposureTotal.toDouble(), 'f', 1);
        NumericTableWidgetItem *exposureItem = new NumericTableWidgetItem(exposureText);
        exposureItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->sessionsTable->setItem(row, 5, exposureItem);

        // Comments column
        QTableWidgetItem *commentsItem = new QTableWidgetItem(session.comments);
        ui->sessionsTable->setItem(row, 6, commentsItem);

        row++;
    }

    ui->sessionsTable->setSortingEnabled(true);
}

bool SessionsTab::showSessionDialog(const QString &title, QString &name, QString &startDate, QString &comments)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setMinimumWidth(500);

    QFormLayout *formLayout = new QFormLayout(&dialog);

    // Name field
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    nameEdit->setText(name);
    nameEdit->setPlaceholderText("Enter session name (unique)");
    formLayout->addRow("Session Name:", nameEdit);

    // Start Date field
    QDateEdit *dateEdit = new QDateEdit(&dialog);
    if (!startDate.isEmpty())
    {
        dateEdit->setDate(QDate::fromString(startDate, "yyyy-MM-dd"));
    }
    else
    {
        dateEdit->setDate(QDate::currentDate());
    }
    dateEdit->setCalendarPopup(true);
    formLayout->addRow("Start Date:", dateEdit);

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

    // Validate name on OK
    connect(buttonBox, &QDialogButtonBox::accepted, [&]()
            {
        if (nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Validation Error", "Session name cannot be empty.");
            dialog.done(QDialog::Rejected);
        } });

    if (dialog.exec() == QDialog::Accepted)
    {
        name = nameEdit->text().trimmed();
        startDate = dateEdit->date().toString("yyyy-MM-dd");
        comments = commentsEdit->text().trimmed();

        return !name.isEmpty();
    }

    return false;
}

void SessionsTab::onAddButtonClicked()
{
    QString name = ui->sessionNameLineEdit->text().trimmed();
    QString startDate = ui->startDateEdit->date().toString("yyyy-MM-dd");
    QString comments = ui->sessionCommentsLineEdit->text().trimmed();

    // Insert into database using repository
    QString errorMessage;
    if (!m_repository->addSession(name, QDate::fromString(startDate, "yyyy-MM-dd"), comments, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to add session: %1").arg(errorMessage));
        return;
    }

    // Clear input fields
    ui->sessionNameLineEdit->clear();
    ui->startDateEdit->setDate(QDate::currentDate());
    ui->sessionCommentsLineEdit->clear();

    refreshData();
}

void SessionsTab::onEditButtonClicked()
{
    // Check if a row is selected
    int currentRow = ui->sessionsTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select a session to edit.");
        return;
    }

    // Get current session data (ID is stored in UserRole)
    QTableWidgetItem *nameItem = ui->sessionsTable->item(currentRow, 0);
    int sessionId = nameItem->data(Qt::UserRole).toInt();
    QString currentName = nameItem->text();
    QString currentDate = ui->sessionsTable->item(currentRow, 1)->text();
    QString currentComments = ui->sessionsTable->item(currentRow, 6)->text();

    // Show dialog
    QString name = currentName;
    QString startDate = currentDate;
    QString comments = currentComments;

    if (!showSessionDialog("Edit Session", name, startDate, comments))
    {
        return;
    }

    // Update in database using repository
    QString errorMessage;
    if (!m_repository->updateSession(sessionId, name, QDate::fromString(startDate, "yyyy-MM-dd"), comments, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to update session: %1").arg(errorMessage));
        return;
    }

    refreshData();
}

void SessionsTab::onDeleteButtonClicked()
{
    // Check if a row is selected
    int currentRow = ui->sessionsTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::information(this, "No Selection", "Please select a session to delete.");
        return;
    }

    // Get session data
    QTableWidgetItem *nameItem = ui->sessionsTable->item(currentRow, 0);
    int sessionId = nameItem->data(Qt::UserRole).toInt();
    QString sessionName = nameItem->text();

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Deletion",
        QString("Are you sure you want to delete the session '%1'?").arg(sessionName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    // Delete from database using repository
    QString errorMessage;
    if (!m_repository->deleteSession(sessionId, errorMessage))
    {
        QMessageBox::warning(this, "Database Error",
                             QString("Failed to delete session: %1").arg(errorMessage));
        return;
    }

    refreshData();
}
