#include "mainwindow.h"
#include "settingsmanager.h"
#include "db/databasebackup.h"
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application information
    QApplication::setApplicationName("MonoObsLog");
    QApplication::setApplicationVersion("0.2.0");
    QApplication::setOrganizationName("Prookyon");
    QApplication::setWindowIcon(QIcon(":/images/icon.ico"));

    // Initialize settings manager early to check database path
    SettingsManager settingsManager;
    if (!settingsManager.initialize())
    {
        QMessageBox::critical(nullptr, "Settings Error",
                              "Failed to initialize settings. Application cannot continue.");
        return 1;
    }

    // Check if database path is set and valid
    QString dbPath = settingsManager.databasePath();
    bool needsNewPath = false;

    if (dbPath.isEmpty())
    {
        needsNewPath = true;
    }
    else
    {
        QFileInfo fileInfo(dbPath);
        // Check if file exists OR if parent directory exists (for new database creation)
        if (!fileInfo.exists() && !fileInfo.dir().exists())
        {
            QMessageBox::warning(nullptr, "Database Path Invalid",
                                 QString("The configured database path is invalid:\n%1\n\n"
                                         "Please select a valid location.")
                                     .arg(dbPath));
            needsNewPath = true;
        }
    }

    // Prompt user to select database file if needed
    if (needsNewPath)
    {
        QString selectedPath = QFileDialog::getSaveFileName(
            nullptr,
            "Select or Create Database File",
            QDir::homePath() + "/MonoObsLog.db",
            "SQLite Database (*.db);;All Files (*.*)");

        if (selectedPath.isEmpty())
        {
            // User cancelled - cannot proceed without database
            QMessageBox::critical(nullptr, "Database Required",
                                  "A database file is required to run the application.");
            return 1;
        }

        // Save the selected path to settings
        settingsManager.setDatabasePath(selectedPath);
        if (!settingsManager.saveSettings())
        {
            QMessageBox::warning(nullptr, "Settings Warning",
                                 "Failed to save database path to settings.\n"
                                 "The path will need to be selected again next time.");
        }
        dbPath = selectedPath;
    }

    // Check and create database backup if needed
    DatabaseBackup backupManager;
    QString backupErrorMessage;
    if (!backupManager.checkAndBackupIfNeeded(dbPath, backupErrorMessage))
    {
        // Backup failed, but we can still continue - just warn the user
        QMessageBox::warning(nullptr, "Backup Warning",
                             QString("Failed to create database backup:\n%1\n\n"
                                     "The application will continue, but your database may not be backed up.")
                                 .arg(backupErrorMessage));
    }

    MainWindow window(dbPath);
    window.show();

    return app.exec();
}