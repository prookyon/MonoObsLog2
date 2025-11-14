#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool initialize(const QString &dbPath = "mgw_observations.db");
    bool isOpen() const;
    QSqlDatabase &database();

signals:
    void errorOccurred(const QString &error);
    void databaseInitialized();

private:
    bool createTables();
    bool tableExists(const QString &tableName);

    QSqlDatabase m_database;
    QString m_dbPath;
    bool m_initialized;
};

#endif // DATABASEMANAGER_H