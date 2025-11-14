#include "db/filtertypesrepository.h"
#include "db/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

FilterTypesRepository::FilterTypesRepository(DatabaseManager *dbManager, QObject *parent)
    : QObject(parent), m_dbManager(dbManager)
{
}

QVector<FilterTypeData> FilterTypesRepository::getAllFilterTypes(QString &errorMessage)
{
    QVector<FilterTypeData> filterTypes;

    QSqlQuery query(m_dbManager->database());
    query.prepare("SELECT id, name, priority FROM filter_types ORDER BY priority ASC, name ASC");

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to query filter types:" << errorMessage;
        return filterTypes;
    }

    while (query.next())
    {
        FilterTypeData filterType;
        filterType.id = query.value(0).toInt();
        filterType.name = query.value(1).toString();
        filterType.priority = query.value(2).toInt();
        filterTypes.append(filterType);
    }

    return filterTypes;
}

bool FilterTypesRepository::addFilterType(const QString &name, int priority, QString &errorMessage)
{
    QSqlQuery query(m_dbManager->database());
    query.prepare("INSERT INTO filter_types (name, priority) VALUES (:name, :priority)");
    query.bindValue(":name", name);
    query.bindValue(":priority", priority);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to insert filter type:" << errorMessage;
        return false;
    }

    return true;
}

bool FilterTypesRepository::updateFilterType(int id, const QString &name, int priority, QString &errorMessage)
{
    QSqlQuery query(m_dbManager->database());
    query.prepare("UPDATE filter_types SET name = :name, priority = :priority WHERE id = :id");
    query.bindValue(":name", name);
    query.bindValue(":priority", priority);
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to update filter type:" << errorMessage;
        return false;
    }

    return true;
}

bool FilterTypesRepository::deleteFilterType(int id, QString &errorMessage)
{
    QSqlQuery query(m_dbManager->database());
    query.prepare("DELETE FROM filter_types WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to delete filter type:" << errorMessage;
        return false;
    }

    return true;
}