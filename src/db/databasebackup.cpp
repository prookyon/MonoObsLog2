#include "db/databasebackup.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>
#include <zip.h>

DatabaseBackup::DatabaseBackup(QObject *parent)
    : QObject(parent)
{
}

DatabaseBackup::~DatabaseBackup()
{
}

QString DatabaseBackup::getBackupDirectory(const QString &dbPath)
{
    QFileInfo fileInfo(dbPath);
    QDir dbDir = fileInfo.dir();
    QString backupDir = dbDir.filePath("ObsLogBackup");
    return backupDir;
}

QString DatabaseBackup::getLatestBackupPath(const QString &backupDir)
{
    QDir dir(backupDir);
    if (!dir.exists())
    {
        return QString();
    }

    // Filter for backup files matching pattern observations_backup_YYYY-MM-DD.zip
    QRegularExpression regex("^observations_backup_(\\d{4}-\\d{2}-\\d{2})\\.zip$");
    QStringList backups;

    QFileInfoList files = dir.entryInfoList(QDir::Files, QDir::Name | QDir::Reversed);
    for (const QFileInfo &fileInfo : files)
    {
        QRegularExpressionMatch match = regex.match(fileInfo.fileName());
        if (match.hasMatch())
        {
            backups.append(fileInfo.absoluteFilePath());
        }
    }

    if (backups.isEmpty())
    {
        return QString();
    }

    // Return the first one (most recent due to sorting)
    return backups.first();
}

bool DatabaseBackup::isBackupNeeded(const QString &latestBackupPath)
{
    if (latestBackupPath.isEmpty())
    {
        // No backups exist
        return true;
    }

    // Extract date from filename
    QFileInfo fileInfo(latestBackupPath);
    QString filename = fileInfo.fileName();

    QRegularExpression regex("^observations_backup_(\\d{4}-\\d{2}-\\d{2})\\.zip$");
    QRegularExpressionMatch match = regex.match(filename);

    if (!match.hasMatch())
    {
        return true;
    }

    QString dateString = match.captured(1);
    QDate backupDate = QDate::fromString(dateString, "yyyy-MM-dd");

    if (!backupDate.isValid())
    {
        return true;
    }

    // Check if backup is older than 7 days
    QDate currentDate = QDate::currentDate();
    int daysSinceBackup = backupDate.daysTo(currentDate);

    return daysSinceBackup >= 7;
}

bool DatabaseBackup::createZipBackup(const QString &dbPath, const QString &backupPath, QString &errorMessage)
{
    // Open the zip archive for writing
    int errorp;
    zip_t *archive = zip_open(backupPath.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &errorp);

    if (!archive)
    {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, errorp);
        errorMessage = QString("Failed to create zip archive: %1").arg(zip_error_strerror(&ziperror));
        zip_error_fini(&ziperror);
        return false;
    }

    // Open source database file
    QFile dbFile(dbPath);
    if (!dbFile.open(QIODevice::ReadOnly))
    {
        errorMessage = QString("Failed to open database file: %1").arg(dbFile.errorString());
        zip_close(archive);
        return false;
    }

    // Read all database content
    QByteArray dbData = dbFile.readAll();
    dbFile.close();

    if (dbData.isEmpty())
    {
        errorMessage = "Database file is empty";
        zip_close(archive);
        return false;
    }

    // Create a zip source from the database data
    QFileInfo dbFileInfo(dbPath);
    QString dbFileName = dbFileInfo.fileName();

    zip_source_t *source = zip_source_buffer(archive, dbData.constData(), dbData.size(), 0);
    if (!source)
    {
        errorMessage = QString("Failed to create zip source: %1").arg(zip_strerror(archive));
        zip_close(archive);
        return false;
    }

    // Add file to archive
    zip_int64_t index = zip_file_add(archive, dbFileName.toUtf8().constData(), source, ZIP_FL_ENC_UTF_8);
    if (index < 0)
    {
        errorMessage = QString("Failed to add file to archive: %1").arg(zip_strerror(archive));
        zip_source_free(source);
        zip_close(archive);
        return false;
    }

    // Close the archive
    if (zip_close(archive) < 0)
    {
        errorMessage = QString("Failed to finalize zip archive: %1").arg(zip_strerror(archive));
        return false;
    }

    return true;
}

bool DatabaseBackup::createBackup(const QString &dbPath, QString &errorMessage)
{
    // Verify database file exists
    if (!QFile::exists(dbPath))
    {
        errorMessage = "Database file does not exist";
        return false;
    }

    // Get backup directory path
    QString backupDir = getBackupDirectory(dbPath);

    // Create backup directory if it doesn't exist
    QDir dir;
    if (!dir.exists(backupDir))
    {
        if (!dir.mkpath(backupDir))
        {
            errorMessage = QString("Failed to create backup directory: %1").arg(backupDir);
            return false;
        }
    }

    // Generate backup filename with current date
    QString currentDate = QDate::currentDate().toString("yyyy-MM-dd");
    QString backupFileName = QString("observations_backup_%1.zip").arg(currentDate);
    QString backupPath = QDir(backupDir).filePath(backupFileName);

    // Create the zip backup
    if (!createZipBackup(dbPath, backupPath, errorMessage))
    {
        return false;
    }

    emit backupCreated(backupPath);
    qDebug() << "Database backup created:" << backupPath;

    return true;
}

bool DatabaseBackup::checkAndBackupIfNeeded(const QString &dbPath, QString &errorMessage)
{
    // Get backup directory
    QString backupDir = getBackupDirectory(dbPath);

    // Get latest backup file
    QString latestBackup = getLatestBackupPath(backupDir);

    // Check if backup is needed
    bool needsBackup = isBackupNeeded(latestBackup);

    if (needsBackup)
    {
        qDebug() << "Backup needed. Creating new backup...";
        if (!createBackup(dbPath, errorMessage))
        {
            emit backupCheckCompleted(false);
            return false;
        }
        emit backupCheckCompleted(true);
        return true;
    }
    else
    {
        qDebug() << "Recent backup exists, no backup needed";
        emit backupCheckCompleted(false);
        return true;
    }
}