#include "tabs/settingstab.h"
#include "ui_settings_tab.h"
#include "settingsmanager.h"
#include "db/databasemanager.h"
#include <QMessageBox>
#include <QPushButton>

SettingsTab::SettingsTab(DatabaseManager *dbManager, SettingsManager *settingsManager, QWidget *parent)
    : QWidget(parent), ui(new Ui::SettingsTab), m_dbManager(dbManager), m_settingsManager(settingsManager)
{
    ui->setupUi(this);
}

SettingsTab::~SettingsTab()
{
    delete ui;
}

void SettingsTab::initialize()
{
    // Connect Save button
    connect(ui->saveButton, &QPushButton::clicked, this, &SettingsTab::onSaveButtonClicked);

    // Initialize the tab UI and data
    refreshData();
}

void SettingsTab::refreshData()
{
    // Load settings from SettingsManager and populate UI
    loadSettingsToUI();
}

void SettingsTab::loadSettingsToUI()
{
    if (!m_settingsManager)
    {
        return;
    }

    // Load settings values into spinboxes
    ui->moonIlluminationSpinBox->setValue(m_settingsManager->moonIlluminationWarningPercent());
    ui->moonSeparationSpinBox->setValue(m_settingsManager->moonAngularSeparationWarningDeg());
    ui->latitudeSpinBox->setValue(m_settingsManager->latitude());
    ui->longitudeSpinBox->setValue(m_settingsManager->longitude());
}

void SettingsTab::onSaveButtonClicked()
{
    if (!m_settingsManager)
    {
        QMessageBox::warning(this, "Error", "Settings manager not available.");
        return;
    }

    // Get values from UI controls
    int moonIllumination = ui->moonIlluminationSpinBox->value();
    int moonSeparation = ui->moonSeparationSpinBox->value();
    double latitude = ui->latitudeSpinBox->value();
    double longitude = ui->longitudeSpinBox->value();

    // Update settings manager
    m_settingsManager->setMoonIlluminationWarningPercent(moonIllumination);
    m_settingsManager->setMoonAngularSeparationWarningDeg(moonSeparation);
    m_settingsManager->setLatitude(latitude);
    m_settingsManager->setLongitude(longitude);

    // Save to file
    if (!m_settingsManager->saveSettings())
    {
        QMessageBox::warning(this, "Error", "Failed to save settings.");
    }
}