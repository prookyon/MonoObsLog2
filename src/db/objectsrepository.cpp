#include "db/objectsrepository.h"
#include "db/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ObjectsRepository::ObjectsRepository(DatabaseManager *dbManager, QObject *parent)
    : QObject(parent), m_dbManager(dbManager)
{
}

QVector<ObjectData> ObjectsRepository::getAllObjects(QString &errorMessage)
{
    QVector<ObjectData> objects;

    QSqlQuery query(m_dbManager->database());
    query.prepare("SELECT id, name, ra, dec FROM objects ORDER BY name");

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to query objects:" << errorMessage;
        return objects;
    }

    while (query.next())
    {
        ObjectData obj;
        obj.id = query.value(0).toInt();
        obj.name = query.value(1).toString();
        obj.ra = query.value(2);
        obj.dec = query.value(3);
        objects.append(obj);
    }

    return objects;
}

bool ObjectsRepository::addObject(const QString &name, const QVariant &ra, const QVariant &dec, QString &errorMessage)
{
    QSqlQuery query(m_dbManager->database());
    query.prepare("INSERT INTO objects (name, ra, dec) VALUES (:name, :ra, :dec)");
    query.bindValue(":name", name);
    query.bindValue(":ra", ra);
    query.bindValue(":dec", dec);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to insert object:" << errorMessage;
        return false;
    }

    return true;
}

bool ObjectsRepository::updateObject(int id, const QString &name, const QVariant &ra, const QVariant &dec, QString &errorMessage)
{
    QSqlQuery query(m_dbManager->database());
    query.prepare("UPDATE objects SET name = :name, ra = :ra, dec = :dec WHERE id = :id");
    query.bindValue(":name", name);
    query.bindValue(":ra", ra);
    query.bindValue(":dec", dec);
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to update object:" << errorMessage;
        return false;
    }

    return true;
}

bool ObjectsRepository::deleteObject(int id, QString &errorMessage)
{
    QSqlQuery query(m_dbManager->database());
    query.prepare("DELETE FROM objects WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to delete object:" << errorMessage;
        return false;
    }

    return true;
}