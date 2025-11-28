#include "db/observationsrepository.h"
#include "db/databasemanager.h"
extern "C"
{
#include "novas.h"
}
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

ObservationsRepository::ObservationsRepository(DatabaseManager *dbManager, QObject *parent)
    : QObject(parent), m_dbManager(dbManager)
{
}

QVector<ObservationData> ObservationsRepository::getAllObservations(QString &errorMessage) const {
    QVector<ObservationData> observations;
    errorMessage.clear();

    QSqlQuery query(m_dbManager->database());

    // Join with related tables to get names and calculate angular separation
    const QString sql = R"(
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

        if (!query.value("object_ra").isNull() && !query.value("object_dec").isNull() &&
            !query.value("moon_ra").isNull() && !query.value("moon_dec").isNull())
        {
            const double objRa = query.value("object_ra").toDouble(); // in hours
            const double objDec = query.value("object_dec").toDouble();
            const double moonRa = query.value("moon_ra").toDouble(); // in degrees
            const double moonDec = query.value("moon_dec").toDouble();

            const double separation = novas_sep(objRa * 15.0, objDec, moonRa, moonDec);
            obs.angularSeparation = separation;
        }

        observations.append(obs);
    }

    return observations;
}

QVector<ObservationData> ObservationsRepository::getObservationsByObject(const int objectId, QString &errorMessage) const {
    QVector<ObservationData> observations;
    errorMessage.clear();

    QSqlQuery query(m_dbManager->database());

    const QString sql = R"(
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
        if (!query.value("object_ra").isNull() && !query.value("object_dec").isNull() &&
            !query.value("moon_ra").isNull() && !query.value("moon_dec").isNull())
        {
            const double objRa = query.value("object_ra").toDouble(); // in hours
            const double objDec = query.value("object_dec").toDouble();
            const double moonRa = query.value("moon_ra").toDouble(); // in degrees
            const double moonDec = query.value("moon_dec").toDouble();

            const double separation = novas_sep(objRa * 15.0, objDec, moonRa, moonDec);
            obs.angularSeparation = separation;
        }

        observations.append(obs);
    }

    return observations;
}

bool ObservationsRepository::addObservation(const int imageCount, const int exposureLength, const QString &comments,
                                            const int sessionId, const int objectId, const int cameraId, const int telescopeId,
                                            const int filterId, QString &errorMessage) const {
    errorMessage.clear();

    // Calculate total exposure
    const int totalExposure = imageCount * exposureLength;

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

bool ObservationsRepository::updateObservation(const int id, const int imageCount, const int exposureLength, const QString &comments,
                                               const int sessionId, const int objectId, const int cameraId, const int telescopeId,
                                               const int filterId, QString &errorMessage) const {
    errorMessage.clear();

    // Calculate total exposure
    const int totalExposure = imageCount * exposureLength;

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

bool ObservationsRepository::deleteObservation(const int id, QString &errorMessage) const {
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
