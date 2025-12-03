#include "db/objectsrepository.h"
#include "db/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ObjectsRepository::ObjectsRepository(DatabaseManager *dbManager, QObject *parent)
    : QObject(parent), m_dbManager(dbManager)
{
}

std::expected<QVector<ObjectData>, ER> ObjectsRepository::getAllObjects() const {
    QVector<ObjectData> objects;

    QSqlQuery query(m_dbManager->database());
    query.prepare("SELECT id, name, ra, dec, comments FROM objects ORDER BY name");

    if (!query.exec())
    {
        QString errorMessage = query.lastError().text();
        qDebug() << "Failed to query objects:" << errorMessage;
        return std::unexpected(ER::Error(errorMessage));
    }

    while (query.next())
    {
        ObjectData obj;
        obj.id = query.value(0).toInt();
        obj.name = query.value(1).toString();
        obj.ra = query.value(2);
        obj.dec = query.value(3);
        obj.comments = query.value(4).toString();
        objects.append(obj);
    }

    return objects;
}

std::expected<void, ER> ObjectsRepository::addObject(const QString &name, const QVariant &ra, const QVariant &dec, const QString &comments) const {
    QSqlQuery query(m_dbManager->database());
    query.prepare("INSERT INTO objects (name, ra, dec, comments) VALUES (:name, :ra, :dec, :comments)");
    query.bindValue(":name", name);
    query.bindValue(":ra", ra);
    query.bindValue(":dec", dec);
    query.bindValue(":comments", comments);

    if (!query.exec())
    {
        QString errorMessage = query.lastError().text();
        qDebug() << "Failed to insert object:" << errorMessage;
        return std::unexpected(ER::Error(errorMessage));
    }

    return {};
}

std::expected<void, ER> ObjectsRepository::updateObject(int id, const QString &name, const QVariant &ra, const QVariant &dec, const QString &comments) const {
    QSqlQuery query(m_dbManager->database());
    query.prepare("UPDATE objects SET name = :name, ra = :ra, dec = :dec, comments = :comments WHERE id = :id");
    query.bindValue(":name", name);
    query.bindValue(":ra", ra);
    query.bindValue(":dec", dec);
    query.bindValue(":id", id);
    query.bindValue(":comments", comments);

    if (!query.exec())
    {
        QString errorMessage = query.lastError().text();
        qDebug() << "Failed to update object:" << errorMessage;
        return std::unexpected(ER::Error(errorMessage));
    }

    return {};
}

std::expected<void, ER> ObjectsRepository::deleteObject(int id) const {
    QSqlQuery query(m_dbManager->database());
    query.prepare("DELETE FROM objects WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec())
    {
        QString errorMessage = query.lastError().text();
        qDebug() << "Failed to delete object:" << errorMessage;
        return std::unexpected(ER::Error(errorMessage));
    }

    return {};
}
