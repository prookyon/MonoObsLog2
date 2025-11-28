#include "db/telescopesrepository.h"
#include "db/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

TelescopesRepository::TelescopesRepository(DatabaseManager *dbManager, QObject *parent)
    : QObject(parent), m_dbManager(dbManager)
{
}

QVector<TelescopeData> TelescopesRepository::getAllTelescopes(QString &errorMessage) const {
    QVector<TelescopeData> telescopes;

    QSqlQuery query(m_dbManager->database());
    query.prepare("SELECT id, name, aperture, f_ratio, focal_length FROM telescopes ORDER BY name");

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to query telescopes:" << errorMessage;
        return telescopes;
    }

    while (query.next())
    {
        TelescopeData tel;
        tel.id = query.value(0).toInt();
        tel.name = query.value(1).toString();
        tel.aperture = query.value(2).toInt();
        tel.fRatio = query.value(3).toDouble();
        tel.focalLength = query.value(4).toInt();
        telescopes.append(tel);
    }

    return telescopes;
}

bool TelescopesRepository::addTelescope(const QString &name, int aperture, double fRatio, int focalLength, QString &errorMessage) const {
    QSqlQuery query(m_dbManager->database());
    query.prepare("INSERT INTO telescopes (name, aperture, f_ratio, focal_length) VALUES (:name, :aperture, :f_ratio, :focal_length)");
    query.bindValue(":name", name);
    query.bindValue(":aperture", aperture);
    query.bindValue(":f_ratio", fRatio);
    query.bindValue(":focal_length", focalLength);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to insert telescope:" << errorMessage;
        return false;
    }

    return true;
}

bool TelescopesRepository::updateTelescope(int id, const QString &name, int aperture, double fRatio, int focalLength, QString &errorMessage) const {
    QSqlQuery query(m_dbManager->database());
    query.prepare("UPDATE telescopes SET name = :name, aperture = :aperture, f_ratio = :f_ratio, focal_length = :focal_length WHERE id = :id");
    query.bindValue(":name", name);
    query.bindValue(":aperture", aperture);
    query.bindValue(":f_ratio", fRatio);
    query.bindValue(":focal_length", focalLength);
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to update telescope:" << errorMessage;
        return false;
    }

    return true;
}

bool TelescopesRepository::deleteTelescope(int id, QString &errorMessage) const {
    QSqlQuery query(m_dbManager->database());
    query.prepare("DELETE FROM telescopes WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to delete telescope:" << errorMessage;
        return false;
    }

    return true;
}
