#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QString>

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager() override;

    bool initialize();

    // Getters
    [[nodiscard]] int moonIlluminationWarningPercent() const;
    [[nodiscard]] int moonAngularSeparationWarningDeg() const;
    [[nodiscard]] double latitude() const;
    [[nodiscard]] double longitude() const;
    [[nodiscard]] QString databasePath() const;
    [[nodiscard]] QString style() const;
    [[nodiscard]] Qt::ColorScheme colorScheme() const;
    [[nodiscard]] QString sessionsFolderTemplate() const;

    // Setters
    void setMoonIlluminationWarningPercent(int value);
    void setMoonAngularSeparationWarningDeg(int value);
    void setLatitude(double value);
    void setLongitude(double value);
    void setDatabasePath(const QString &value);
    void setStyle(const QString &value);
    void setColorScheme(Qt::ColorScheme value);
    void setSessionsFolderTemplate(const QString &value);

    // Save settings to file
    bool saveSettings();

signals:
    void errorOccurred(const QString &error);
    void settingsInitialized();
    void settingsChanged();

private:
    bool loadSettings();
    bool createDefaultSettings();
    static QString getSettingsFilePath() ;

    static QString getSettingsDirectoryPath();

    // Settings values
    int m_moonIlluminationWarningPercent;
    int m_moonAngularSeparationWarningDeg;
    double m_latitude;
    double m_longitude;
    QString m_databasePath;
    QString m_style;
    Qt::ColorScheme m_colorScheme;
    QString m_sessionsFolderTemplate;

    bool m_initialized;
};

#endif // SETTINGSMANAGER_H