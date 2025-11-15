#ifndef DATABASEBACKUP_H
#define DATABASEBACKUP_H

#include <QString>
#include <QObject>

class DatabaseBackup : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseBackup(QObject *parent = nullptr);
    ~DatabaseBackup();

    // Check if backup is needed and create if necessary
    bool checkAndBackupIfNeeded(const QString &dbPath, QString &errorMessage);

    // Force create a backup
    bool createBackup(const QString &dbPath, QString &errorMessage);

signals:
    void backupCreated(const QString &backupPath);
    void backupCheckCompleted(bool backupCreated);

private:
    QString getBackupDirectory(const QString &dbPath);
    QString getLatestBackupPath(const QString &backupDir);
    bool isBackupNeeded(const QString &latestBackupPath);
    bool createZipBackup(const QString &dbPath, const QString &backupPath, QString &errorMessage);
};

#endif // DATABASEBACKUP_H