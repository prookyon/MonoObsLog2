#include "db/filtersrepository.h"
#include "db/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

FiltersRepository::FiltersRepository(DatabaseManager *dbManager, QObject *parent)
    : QObject(parent), m_dbManager(dbManager)
{
}

QVector<FilterData> FiltersRepository::getAllFilters(QString &errorMessage)
{
    QVector<FilterData> filters;

    QSqlQuery query(m_dbManager->database());
    query.prepare("SELECT f.id, f.name, f.filter_type_id, ft.name FROM filters f "
                  "JOIN filter_types ft ON f.filter_type_id = ft.id "
                  "ORDER BY f.name");

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to query filters:" << errorMessage;
        return filters;
    }

    while (query.next())
    {
        FilterData filter;
        filter.id = query.value(0).toInt();
        filter.name = query.value(1).toString();
        filter.filterTypeId = query.value(2).toInt();
        filter.filterTypeName = query.value(3).toString();
        filters.append(filter);
    }

    return filters;
}

bool FiltersRepository::addFilter(const QString &name, int filterTypeId, QString &errorMessage)
{
    QSqlQuery query(m_dbManager->database());
    query.prepare("INSERT INTO filters (name, filter_type_id) VALUES (:name, :filter_type_id)");
    query.bindValue(":name", name);
    query.bindValue(":filter_type_id", filterTypeId);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to insert filter:" << errorMessage;
        return false;
    }

    return true;
}

bool FiltersRepository::updateFilter(int id, const QString &name, int filterTypeId, QString &errorMessage)
{
    QSqlQuery query(m_dbManager->database());
    query.prepare("UPDATE filters SET name = :name, filter_type_id = :filter_type_id WHERE id = :id");
    query.bindValue(":name", name);
    query.bindValue(":filter_type_id", filterTypeId);
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to update filter:" << errorMessage;
        return false;
    }

    return true;
}

bool FiltersRepository::deleteFilter(int id, QString &errorMessage)
{
    QSqlQuery query(m_dbManager->database());
    query.prepare("DELETE FROM filters WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to delete filter:" << errorMessage;
        return false;
    }

    return true;
}
