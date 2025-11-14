#include "db/camerasrepository.h"
#include "db/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

CamerasRepository::CamerasRepository(DatabaseManager *dbManager, QObject *parent)
    : QObject(parent), m_dbManager(dbManager)
{
}

QVector<CameraData> CamerasRepository::getAllCameras(QString &errorMessage)
{
    QVector<CameraData> cameras;

    QSqlQuery query(m_dbManager->database());
    query.prepare("SELECT id, name, sensor, pixel_size, width, height FROM cameras ORDER BY name");

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to query cameras:" << errorMessage;
        return cameras;
    }

    while (query.next())
    {
        CameraData cam;
        cam.id = query.value(0).toInt();
        cam.name = query.value(1).toString();
        cam.sensor = query.value(2).toString();
        cam.pixelSize = query.value(3).toDouble();
        cam.width = query.value(4).toInt();
        cam.height = query.value(5).toInt();
        cameras.append(cam);
    }

    return cameras;
}

bool CamerasRepository::addCamera(const QString &name, const QString &sensor, double pixelSize, int width, int height, QString &errorMessage)
{
    QSqlQuery query(m_dbManager->database());
    query.prepare("INSERT INTO cameras (name, sensor, pixel_size, width, height) VALUES (:name, :sensor, :pixel_size, :width, :height)");
    query.bindValue(":name", name);
    query.bindValue(":sensor", sensor);
    query.bindValue(":pixel_size", pixelSize);
    query.bindValue(":width", width);
    query.bindValue(":height", height);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to insert camera:" << errorMessage;
        return false;
    }

    return true;
}

bool CamerasRepository::updateCamera(int id, const QString &name, const QString &sensor, double pixelSize, int width, int height, QString &errorMessage)
{
    QSqlQuery query(m_dbManager->database());
    query.prepare("UPDATE cameras SET name = :name, sensor = :sensor, pixel_size = :pixel_size, width = :width, height = :height WHERE id = :id");
    query.bindValue(":name", name);
    query.bindValue(":sensor", sensor);
    query.bindValue(":pixel_size", pixelSize);
    query.bindValue(":width", width);
    query.bindValue(":height", height);
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to update camera:" << errorMessage;
        return false;
    }

    return true;
}

bool CamerasRepository::deleteCamera(int id, QString &errorMessage)
{
    QSqlQuery query(m_dbManager->database());
    query.prepare("DELETE FROM cameras WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to delete camera:" << errorMessage;
        return false;
    }

    return true;
}