#include "db/observationsrepository.h"
#include "db/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QtMath>

ObservationsRepository::ObservationsRepository(DatabaseManager *dbManager, QObject *parent)
    : QObject(parent), m_dbManager(dbManager)
{
}

QVector<ObservationData> ObservationsRepository::getAllObservations(QString &errorMessage)
{
    QVector<ObservationData> observations;
    errorMessage.clear();

    QSqlQuery query(m_dbManager->database());

    // Join with related tables to get names and calculate angular separation
    QString sql = R"(
        SELECT 
            o.id, o.image_count, o.exposure_length, o.total_exposure, o.comments,
            o.session_id, o.object_id, o.camera_id, o.telescope_id, o.filter_id,
            s.name AS session_name, s.start_date AS session_date,
            obj.name AS object_name, obj.ra AS object_ra, obj.dec AS object_dec,
            c.name AS camera_name,
            t.name AS telescope_name,
            f.name AS filter_name,
            s.moon_illumination,
            s.moon_ra, s.moon_dec
        FROM observations o
        INNER JOIN sessions s ON o.session_id = s.id
        INNER JOIN objects obj ON o.object_id = obj.id
        INNER JOIN cameras c ON o.camera_id = c.id
        INNER JOIN telescopes t ON o.telescope_id = t.id
        INNER JOIN filters f ON o.filter_id = f.id
        ORDER BY s.start_date DESC, o.id DESC
    )";

    if (!query.exec(sql))
    {
        errorMessage = QString("Query failed: %1").arg(query.lastError().text());
        return observations;
    }

    while (query.next())
    {
        ObservationData obs;
        obs.id = query.value("id").toInt();
        obs.imageCount = query.value("image_count").toInt();
        obs.exposureLength = query.value("exposure_length").toInt();
        obs.totalExposure = query.value("total_exposure").toInt();
        obs.comments = query.value("comments").toString();
        obs.sessionId = query.value("session_id").toInt();
        obs.objectId = query.value("object_id").toInt();
        obs.cameraId = query.value("camera_id").toInt();
        obs.telescopeId = query.value("telescope_id").toInt();
        obs.filterId = query.value("filter_id").toInt();
        obs.sessionName = query.value("session_name").toString();
        obs.sessionDate = query.value("session_date").toString();
        obs.objectName = query.value("object_name").toString();
        obs.cameraName = query.value("camera_name").toString();
        obs.telescopeName = query.value("telescope_name").toString();
        obs.filterName = query.value("filter_name").toString();
        obs.moonIllumination = query.value("moon_illumination").toDouble();

        // Calculate angular separation if coordinates are available
        obs.angularSeparation = -1.0; // Default: not available
        if (!query.value("object_ra").isNull() && !query.value("object_dec").isNull() &&
            !query.value("moon_ra").isNull() && !query.value("moon_dec").isNull())
        {
            double objRa = query.value("object_ra").toDouble();
            double objDec = query.value("object_dec").toDouble();
            double moonRa = query.value("moon_ra").toDouble();
            double moonDec = query.value("moon_dec").toDouble();

            // Convert RA from hours to radians (objRa is in hours)
            objRa = objRa * 15.0 * M_PI / 180.0;
            moonRa = moonRa * 15.0 * M_PI / 180.0;

            // Convert DEC to radians
            objDec = objDec * M_PI / 180.0;
            moonDec = moonDec * M_PI / 180.0;

            // Calculate angular separation using spherical law of cosines
            double cosDistance = qSin(objDec) * qSin(moonDec) +
                                 qCos(objDec) * qCos(moonDec) * qCos(objRa - moonRa);

            // Clamp to prevent floating point errors
            cosDistance = qBound(-1.0, cosDistance, 1.0);
            obs.angularSeparation = qAcos(cosDistance) * 180.0 / M_PI;
        }

        observations.append(obs);
    }

    return observations;
}

QVector<ObservationData> ObservationsRepository::getObservationsByObject(int objectId, QString &errorMessage)
{
    QVector<ObservationData> observations;
    errorMessage.clear();

    QSqlQuery query(m_dbManager->database());

    QString sql = R"(
        SELECT 
            o.id, o.image_count, o.exposure_length, o.total_exposure, o.comments,
            o.session_id, o.object_id, o.camera_id, o.telescope_id, o.filter_id,
            s.name AS session_name, s.start_date AS session_date,
            obj.name AS object_name, obj.ra AS object_ra, obj.dec AS object_dec,
            c.name AS camera_name,
            t.name AS telescope_name,
            f.name AS filter_name,
            s.moon_illumination,
            s.moon_ra, s.moon_dec
        FROM observations o
        INNER JOIN sessions s ON o.session_id = s.id
        INNER JOIN objects obj ON o.object_id = obj.id
        INNER JOIN cameras c ON o.camera_id = c.id
        INNER JOIN telescopes t ON o.telescope_id = t.id
        INNER JOIN filters f ON o.filter_id = f.id
        WHERE o.object_id = :objectId
        ORDER BY s.start_date DESC, o.id DESC
    )";

    query.prepare(sql);
    query.bindValue(":objectId", objectId);

    if (!query.exec())
    {
        errorMessage = QString("Query failed: %1").arg(query.lastError().text());
        return observations;
    }

    while (query.next())
    {
        ObservationData obs;
        obs.id = query.value("id").toInt();
        obs.imageCount = query.value("image_count").toInt();
        obs.exposureLength = query.value("exposure_length").toInt();
        obs.totalExposure = query.value("total_exposure").toInt();
        obs.comments = query.value("comments").toString();
        obs.sessionId = query.value("session_id").toInt();
        obs.objectId = query.value("object_id").toInt();
        obs.cameraId = query.value("camera_id").toInt();
        obs.telescopeId = query.value("telescope_id").toInt();
        obs.filterId = query.value("filter_id").toInt();
        obs.sessionName = query.value("session_name").toString();
        obs.sessionDate = query.value("session_date").toString();
        obs.objectName = query.value("object_name").toString();
        obs.cameraName = query.value("camera_name").toString();
        obs.telescopeName = query.value("telescope_name").toString();
        obs.filterName = query.value("filter_name").toString();
        obs.moonIllumination = query.value("moon_illumination").toDouble();

        // Calculate angular separation
        obs.angularSeparation = -1.0;
        if (!query.value("object_ra").isNull() && !query.value("object_dec").isNull() &&
            !query.value("moon_ra").isNull() && !query.value("moon_dec").isNull())
        {
            double objRa = query.value("object_ra").toDouble();
            double objDec = query.value("object_dec").toDouble();
            double moonRa = query.value("moon_ra").toDouble();
            double moonDec = query.value("moon_dec").toDouble();

            objRa = objRa * 15.0 * M_PI / 180.0;
            moonRa = moonRa * 15.0 * M_PI / 180.0;
            objDec = objDec * M_PI / 180.0;
            moonDec = moonDec * M_PI / 180.0;

            double cosDistance = qSin(objDec) * qSin(moonDec) +
                                 qCos(objDec) * qCos(moonDec) * qCos(objRa - moonRa);
            cosDistance = qBound(-1.0, cosDistance, 1.0);
            obs.angularSeparation = qAcos(cosDistance) * 180.0 / M_PI;
        }

        observations.append(obs);
    }

    return observations;
}

bool ObservationsRepository::addObservation(int imageCount, int exposureLength, const QString &comments,
                                            int sessionId, int objectId, int cameraId, int telescopeId,
                                            int filterId, QString &errorMessage)
{
    errorMessage.clear();

    // Calculate total exposure
    int totalExposure = imageCount * exposureLength;

    QSqlQuery query(m_dbManager->database());
    query.prepare(R"(
        INSERT INTO observations (image_count, exposure_length, total_exposure, comments,
                                 session_id, object_id, camera_id, telescope_id, filter_id)
        VALUES (:imageCount, :exposureLength, :totalExposure, :comments,
                :sessionId, :objectId, :cameraId, :telescopeId, :filterId)
    )");

    query.bindValue(":imageCount", imageCount);
    query.bindValue(":exposureLength", exposureLength);
    query.bindValue(":totalExposure", totalExposure);
    query.bindValue(":comments", comments.isEmpty() ? QVariant() : comments);
    query.bindValue(":sessionId", sessionId);
    query.bindValue(":objectId", objectId);
    query.bindValue(":cameraId", cameraId);
    query.bindValue(":telescopeId", telescopeId);
    query.bindValue(":filterId", filterId);

    if (!query.exec())
    {
        errorMessage = QString("Failed to add observation: %1").arg(query.lastError().text());
        return false;
    }

    return true;
}

bool ObservationsRepository::updateObservation(int id, int imageCount, int exposureLength, const QString &comments,
                                               int sessionId, int objectId, int cameraId, int telescopeId,
                                               int filterId, QString &errorMessage)
{
    errorMessage.clear();

    // Calculate total exposure
    int totalExposure = imageCount * exposureLength;

    QSqlQuery query(m_dbManager->database());
    query.prepare(R"(
        UPDATE observations 
        SET image_count = :imageCount,
            exposure_length = :exposureLength,
            total_exposure = :totalExposure,
            comments = :comments,
            session_id = :sessionId,
            object_id = :objectId,
            camera_id = :cameraId,
            telescope_id = :telescopeId,
            filter_id = :filterId
        WHERE id = :id
    )");

    query.bindValue(":imageCount", imageCount);
    query.bindValue(":exposureLength", exposureLength);
    query.bindValue(":totalExposure", totalExposure);
    query.bindValue(":comments", comments.isEmpty() ? QVariant() : comments);
    query.bindValue(":sessionId", sessionId);
    query.bindValue(":objectId", objectId);
    query.bindValue(":cameraId", cameraId);
    query.bindValue(":telescopeId", telescopeId);
    query.bindValue(":filterId", filterId);
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = QString("Failed to update observation: %1").arg(query.lastError().text());
        return false;
    }

    return true;
}

bool ObservationsRepository::deleteObservation(int id, QString &errorMessage)
{
    errorMessage.clear();

    QSqlQuery query(m_dbManager->database());
    query.prepare("DELETE FROM observations WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = QString("Failed to delete observation: %1").arg(query.lastError().text());
        return false;
    }

    return true;
}