#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QString>

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager();

    bool initialize();

    // Getters
    int moonIlluminationWarningPercent() const;
    int moonAngularSeparationWarningDeg() const;
    double latitude() const;
    double longitude() const;

    // Setters
    void setMoonIlluminationWarningPercent(int value);
    void setMoonAngularSeparationWarningDeg(int value);
    void setLatitude(double value);
    void setLongitude(double value);

    // Save settings to file
    bool saveSettings();

signals:
    void errorOccurred(const QString &error);
    void settingsInitialized();
    void settingsChanged();

private:
    bool loadSettings();
    bool createDefaultSettings();
    QString getSettingsFilePath() const;
    QString getSettingsDirectoryPath() const;

    // Settings values
    int m_moonIlluminationWarningPercent;
    int m_moonAngularSeparationWarningDeg;
    double m_latitude;
    double m_longitude;

    bool m_initialized;
};

#endif // SETTINGSMANAGER_H