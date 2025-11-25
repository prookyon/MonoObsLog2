#include "settingsmanager.h"
#include <QDir>
#include <QJsonObject>
#include <QDebug>
#include <QStandardPaths>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent),
      m_moonIlluminationWarningPercent(75),
      m_moonAngularSeparationWarningDeg(60),
      m_latitude(0.0),
      m_longitude(0.0),
      m_databasePath(""),
      m_style(""),
      m_colorScheme(Qt::ColorScheme::Unknown),
      m_initialized(false)
{
}

SettingsManager::~SettingsManager()
= default;

bool SettingsManager::initialize()
{
    const QString settingsDir = getSettingsDirectoryPath();
    const QString settingsFile = getSettingsFilePath();

    // Check if directory exists, create if not
    if (const QDir dir; !dir.exists(settingsDir))
    {
        if (!dir.mkpath(settingsDir))
        {
            emit errorOccurred(QString("Failed to create settings directory: %1").arg(settingsDir));
            return false;
        }
        qDebug() << "Created settings directory:" << settingsDir;
    }

    // Check if settings file exists

    if (const bool fileExists = QFile::exists(settingsFile); !fileExists)
    {
        // Create default settings file
        if (!createDefaultSettings())
        {
            emit errorOccurred("Failed to create default settings file");
            return false;
        }
        qDebug() << "Created default settings file:" << settingsFile;
    }
    else
    {
        // Load existing settings
        if (!loadSettings())
        {
            emit errorOccurred("Failed to load settings file");
            return false;
        }
        qDebug() << "Loaded settings from:" << settingsFile;
    }

    m_initialized = true;
    emit settingsInitialized();
    return true;
}

QString SettingsManager::getSettingsDirectoryPath() {
    const QString homePath = QDir::homePath();
    return QDir(homePath).filePath(".MonoObsLog");
}

QString SettingsManager::getSettingsFilePath()
{
    return QDir(getSettingsDirectoryPath()).filePath("settings.json");
}

bool SettingsManager::createDefaultSettings()
{
    // Set default values
    m_moonIlluminationWarningPercent = 75;
    m_moonAngularSeparationWarningDeg = 60;
    m_latitude = 0.0;
    m_longitude = 0.0;
    m_style = "";                             // use Qt default at first
    m_colorScheme = Qt::ColorScheme::Unknown; // use Qt default at first
    // Note: m_databasePath has no default - must be set by user

    // Create JSON object with default values
    QJsonObject jsonObj;
    jsonObj["moon_illumination_warning_percent"] = m_moonIlluminationWarningPercent;
    jsonObj["moon_angular_separation_warning_deg"] = m_moonAngularSeparationWarningDeg;
    jsonObj["latitude"] = m_latitude;
    jsonObj["longitude"] = m_longitude;
    jsonObj["style"] = m_style;
    jsonObj["color_scheme"] = static_cast<int>(m_colorScheme);
    // Do not add database_path to default settings

    // Write to file
    const QJsonDocument doc(jsonObj);
    QFile file(getSettingsFilePath());

    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open settings file for writing:" << file.errorString();
        return false;
    }

    if (file.write(doc.toJson(QJsonDocument::Indented)) == -1)
    {
        qDebug() << "Failed to write settings to file:" << file.errorString();
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool SettingsManager::loadSettings()
{
    QFile file(getSettingsFilePath());

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open settings file for reading:" << file.errorString();
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    const QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isObject())
    {
        qDebug() << "Invalid JSON in settings file";
        return false;
    }

    const QJsonObject jsonObj = doc.object();

    // Load values with defaults if keys are missing
    m_moonIlluminationWarningPercent = jsonObj.value("moon_illumination_warning_percent").toInt(75);
    m_moonAngularSeparationWarningDeg = jsonObj.value("moon_angular_separation_warning_deg").toInt(60);
    m_latitude = jsonObj.value("latitude").toDouble(0.0);
    m_longitude = jsonObj.value("longitude").toDouble(0.0);
    // Load database_path - no default value, empty if not present
    m_databasePath = jsonObj.value("database_path").toString("");
    m_style = jsonObj.value("style").toString("");
    m_colorScheme = static_cast<Qt::ColorScheme>(jsonObj.value("color_scheme").toInt(static_cast<int>(Qt::ColorScheme::Unknown)));

    return true;
}

bool SettingsManager::saveSettings()
{
    QJsonObject jsonObj;
    jsonObj["moon_illumination_warning_percent"] = m_moonIlluminationWarningPercent;
    jsonObj["moon_angular_separation_warning_deg"] = m_moonAngularSeparationWarningDeg;
    jsonObj["latitude"] = m_latitude;
    jsonObj["longitude"] = m_longitude;
    jsonObj["database_path"] = m_databasePath;
    jsonObj["style"] = m_style;
    jsonObj["color_scheme"] = static_cast<int>(m_colorScheme);

    const QJsonDocument doc(jsonObj);
    QFile file(getSettingsFilePath());

    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open settings file for writing:" << file.errorString();
        emit errorOccurred(QString("Failed to save settings: %1").arg(file.errorString()));
        return false;
    }

    if (file.write(doc.toJson(QJsonDocument::Indented)) == -1)
    {
        qDebug() << "Failed to write settings to file:" << file.errorString();
        emit errorOccurred(QString("Failed to write settings: %1").arg(file.errorString()));
        file.close();
        return false;
    }

    file.close();
    emit settingsChanged();
    return true;
}

// Getters
int SettingsManager::moonIlluminationWarningPercent() const
{
    return m_moonIlluminationWarningPercent;
}

int SettingsManager::moonAngularSeparationWarningDeg() const
{
    return m_moonAngularSeparationWarningDeg;
}

double SettingsManager::latitude() const
{
    return m_latitude;
}

double SettingsManager::longitude() const
{
    return m_longitude;
}

QString SettingsManager::databasePath() const
{
    return m_databasePath;
}

QString SettingsManager::style() const
{
    return m_style;
}

Qt::ColorScheme SettingsManager::colorScheme() const
{
    return m_colorScheme;
}

// Setters
void SettingsManager::setMoonIlluminationWarningPercent(const int value)
{
    if (m_moonIlluminationWarningPercent != value)
    {
        m_moonIlluminationWarningPercent = value;
    }
}

void SettingsManager::setMoonAngularSeparationWarningDeg(const int value)
{
    if (m_moonAngularSeparationWarningDeg != value)
    {
        m_moonAngularSeparationWarningDeg = value;
    }
}

void SettingsManager::setLatitude(const double value)
{
    if (m_latitude != value)
    {
        m_latitude = value;
    }
}

void SettingsManager::setLongitude(const double value)
{
    if (m_longitude != value)
    {
        m_longitude = value;
    }
}

void SettingsManager::setDatabasePath(const QString &value)
{
    if (m_databasePath != value)
    {
        m_databasePath = value;
    }
}

void SettingsManager::setStyle(const QString &value)
{
    if (m_style != value)
    {
        m_style = value;
    }
}

void SettingsManager::setColorScheme(const Qt::ColorScheme value)
{
    if (m_colorScheme != value)
    {
        m_colorScheme = value;
    }
}