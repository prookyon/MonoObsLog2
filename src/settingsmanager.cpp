#include "settingsmanager.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
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
      m_initialized(false)
{
}

SettingsManager::~SettingsManager()
{
}

bool SettingsManager::initialize()
{
    QString settingsDir = getSettingsDirectoryPath();
    QString settingsFile = getSettingsFilePath();

    // Check if directory exists, create if not
    QDir dir;
    if (!dir.exists(settingsDir))
    {
        if (!dir.mkpath(settingsDir))
        {
            emit errorOccurred(QString("Failed to create settings directory: %1").arg(settingsDir));
            return false;
        }
        qDebug() << "Created settings directory:" << settingsDir;
    }

    // Check if settings file exists
    bool fileExists = QFile::exists(settingsFile);

    if (!fileExists)
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

QString SettingsManager::getSettingsDirectoryPath() const
{
    QString homePath = QDir::homePath();
    return QDir(homePath).filePath(".MonoObsLog");
}

QString SettingsManager::getSettingsFilePath() const
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
    // Note: m_databasePath has no default - must be set by user

    // Create JSON object with default values
    QJsonObject jsonObj;
    jsonObj["moon_illumination_warning_percent"] = m_moonIlluminationWarningPercent;
    jsonObj["moon_angular_separation_warning_deg"] = m_moonAngularSeparationWarningDeg;
    jsonObj["latitude"] = m_latitude;
    jsonObj["longitude"] = m_longitude;
    // Do not add database_path to default settings

    // Write to file
    QJsonDocument doc(jsonObj);
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

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isObject())
    {
        qDebug() << "Invalid JSON in settings file";
        return false;
    }

    QJsonObject jsonObj = doc.object();

    // Load values with defaults if keys are missing
    m_moonIlluminationWarningPercent = jsonObj.value("moon_illumination_warning_percent").toInt(75);
    m_moonAngularSeparationWarningDeg = jsonObj.value("moon_angular_separation_warning_deg").toInt(60);
    m_latitude = jsonObj.value("latitude").toDouble(0.0);
    m_longitude = jsonObj.value("longitude").toDouble(0.0);
    // Load database_path - no default value, empty if not present
    m_databasePath = jsonObj.value("database_path").toString("");

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

    QJsonDocument doc(jsonObj);
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

// Setters
void SettingsManager::setMoonIlluminationWarningPercent(int value)
{
    if (m_moonIlluminationWarningPercent != value)
    {
        m_moonIlluminationWarningPercent = value;
    }
}

void SettingsManager::setMoonAngularSeparationWarningDeg(int value)
{
    if (m_moonAngularSeparationWarningDeg != value)
    {
        m_moonAngularSeparationWarningDeg = value;
    }
}

void SettingsManager::setLatitude(double value)
{
    if (m_latitude != value)
    {
        m_latitude = value;
    }
}

void SettingsManager::setLongitude(double value)
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