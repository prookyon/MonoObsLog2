#include "tabs/settingstab.h"
#include "ui_settings_tab.h"
#include "settingsmanager.h"
#include <QMessageBox>
#include <QPushButton>
#include <QStyleFactory>
#include <QstyleHints>

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

    ui->styleComboBox->clear();
    const auto styles = QStyleFactory::keys();
    ui->styleComboBox->addItems(styles);
    const auto currentStyle = qApp->style()->objectName();
    if (const auto styleIndex = ui->styleComboBox->findText(currentStyle, Qt::MatchFixedString); styleIndex != -1)
    {
        ui->styleComboBox->setCurrentIndex(styleIndex);
    }
    connect(ui->styleComboBox, &QComboBox::currentTextChanged, this, [](const QString &style)
            { qApp->setStyle(style); });

    ui->colorSchemeComboBox->clear();
    ui->colorSchemeComboBox->addItem(QString("System Default"), QVariant::fromValue<Qt::ColorScheme>(Qt::ColorScheme::Unknown));
    ui->colorSchemeComboBox->addItem(QString("Light"), QVariant::fromValue<Qt::ColorScheme>(Qt::ColorScheme::Light));
    ui->colorSchemeComboBox->addItem(QString("Dark"), QVariant::fromValue<Qt::ColorScheme>(Qt::ColorScheme::Dark));
    const auto currentColorScheme = qApp->styleHints()->colorScheme();
    if (const auto colorSchemeIndex = ui->colorSchemeComboBox->findData(QVariant::fromValue(currentColorScheme)); colorSchemeIndex != -1)
    {
        ui->colorSchemeComboBox->setCurrentIndex(colorSchemeIndex);
    }
    connect(ui->colorSchemeComboBox, &QComboBox::currentIndexChanged, this, [this](const int index)
            {
                const auto colorScheme = ui->colorSchemeComboBox->itemData(index).value<Qt::ColorScheme>();
                qApp->styleHints()->setColorScheme(colorScheme); });

    ui->sessionsFolderEdit->setText(m_settingsManager->sessionsFolderTemplate());
}

void SettingsTab::onSaveButtonClicked()
{
    if (!m_settingsManager)
    {
        QMessageBox::warning(this, "Error", "Settings manager not available.");
        return;
    }

    // Get values from UI controls
    const int moonIllumination = ui->moonIlluminationSpinBox->value();
    const int moonSeparation = ui->moonSeparationSpinBox->value();
    const double latitude = ui->latitudeSpinBox->value();
    const double longitude = ui->longitudeSpinBox->value();
    const QString style = ui->styleComboBox->currentText();
    const auto colorScheme = ui->colorSchemeComboBox->currentData().value<Qt::ColorScheme>();
    const QString sessionsFolderTemplate = ui->sessionsFolderEdit->text();

    // Update settings manager
    m_settingsManager->setMoonIlluminationWarningPercent(moonIllumination);
    m_settingsManager->setMoonAngularSeparationWarningDeg(moonSeparation);
    m_settingsManager->setLatitude(latitude);
    m_settingsManager->setLongitude(longitude);
    m_settingsManager->setStyle(style);
    m_settingsManager->setColorScheme(colorScheme);
    m_settingsManager->setSessionsFolderTemplate(sessionsFolderTemplate);

    // Save to file
    if (!m_settingsManager->saveSettings())
    {
        QMessageBox::warning(this, "Error", "Failed to save settings.");
    }
}