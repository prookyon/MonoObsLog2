#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include "ER.h"
#include <expected>

#define OBSLOGDBVERSION 3

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager() override;

    std::expected<void, ER> initialize(const QString &dbPath = "mgw_observations.db");
    [[nodiscard]] bool isOpen() const;
    QSqlDatabase &database();

    static int getSupportedDbVersion();
    [[nodiscard]] std::expected<int, ER> getActualDbVersion() const;


signals:
    void errorOccurred(const QString &error);
    void databaseInitialized();

private:
    [[nodiscard]] bool createTables() const;
    [[nodiscard]] bool tableExists(const QString &tableName) const;
    std::expected<void, ER> runMigrations(int fromVersion, int toVersion) const;


    QSqlDatabase m_database;
    QString m_dbPath;
    bool m_initialized;
};

#endif // DATABASEMANAGER_H