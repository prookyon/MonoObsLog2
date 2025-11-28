#ifndef DATABASEBACKUP_H
#define DATABASEBACKUP_H

#include <QString>
#include <QObject>

class DatabaseBackup : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseBackup(QObject *parent = nullptr);
    ~DatabaseBackup() override;

    // Check if backup is needed and create if necessary
    static bool checkAndBackupIfNeeded(const QString &dbPath, QString &errorMessage);

    // Force create a backup
    static bool createBackup(const QString &dbPath, QString &errorMessage);

private:
    static QString getBackupDirectory(const QString &dbPath);
    static QString getLatestBackupPath(const QString &backupDir);
    static bool isBackupNeeded(const QString &latestBackupPath);
    static bool createZipBackup(const QString &dbPath, const QString &backupPath, QString &errorMessage);
};

#endif // DATABASEBACKUP_H