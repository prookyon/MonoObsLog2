#include "db/sessionsrepository.h"
#include "db/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

SessionsRepository::SessionsRepository(DatabaseManager *dbManager, QObject *parent)
    : QObject(parent), m_dbManager(dbManager)
{
}

QVector<SessionData> SessionsRepository::getAllSessions(QString &errorMessage) const {
    QVector<SessionData> sessions;

    QSqlQuery query(m_dbManager->database());
    query.prepare(R"(
        SELECT
            s.id,
            s.name,
            s.start_date,
            s.moon_illumination,
            s.moon_ra,
            s.moon_dec,
            s.comments,
            COALESCE(SUM(o.total_exposure) / 3600.0, 0) as exposure_total_hours
        FROM sessions s
        LEFT JOIN observations o ON s.id = o.session_id
        GROUP BY s.id, s.name, s.start_date, s.moon_illumination, s.moon_ra, s.moon_dec, s.comments
        ORDER BY s.start_date DESC
    )");

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to query sessions:" << errorMessage;
        return sessions;
    }

    while (query.next())
    {
        SessionData session;
        session.id = query.value(0).toInt();
        session.name = query.value(1).toString();
        session.startDate = query.value(2).toDate();
        session.moonIllumination = query.value(3);
        session.moonRa = query.value(4);
        session.moonDec = query.value(5);
        session.comments = query.value(6).toString();
        session.exposureTotal = query.value(7);
        sessions.append(session);
    }

    return sessions;
}

bool SessionsRepository::addSession(const QString &name, const QDate &startDate, const QString &comments, const double &moonIllumination, const double &moonRa, const double &moonDec, QString &errorMessage) const {
    QSqlQuery query(m_dbManager->database());
    query.prepare("INSERT INTO sessions (name, start_date, moon_illumination, moon_ra, moon_dec, comments) VALUES (:name, :start_date, :moon_illumination, :moon_ra, :moon_dec, :comments)");
    query.bindValue(":name", name);
    query.bindValue(":start_date", startDate.toString(Qt::ISODate));
    query.bindValue(":moon_illumination", moonIllumination);
    query.bindValue(":moon_ra", moonRa);
    query.bindValue(":moon_dec", moonDec);
    query.bindValue(":comments", comments);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to insert session:" << errorMessage;
        return false;
    }

    return true;
}

bool SessionsRepository::updateSession(int id, const QString &name, const QDate &startDate, const QString &comments, const double &moonIllumination, const double &moonRa, const double &moonDec, QString &errorMessage) const {
    QSqlQuery query(m_dbManager->database());
    query.prepare("UPDATE sessions SET name = :name, start_date = :start_date, comments = :comments, moon_illumination = :moon_illumination, moon_ra = :moon_ra, moon_dec = :moon_dec WHERE id = :id");
    query.bindValue(":name", name);
    query.bindValue(":start_date", startDate.toString(Qt::ISODate));
    query.bindValue(":moon_illumination", moonIllumination);
    query.bindValue(":moon_ra", moonRa);
    query.bindValue(":moon_dec", moonDec);
    query.bindValue(":comments", comments);
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to update session:" << errorMessage;
        return false;
    }

    return true;
}

bool SessionsRepository::deleteSession(int id, QString &errorMessage) const {
    QSqlQuery query(m_dbManager->database());
    query.prepare("DELETE FROM sessions WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        qDebug() << "Failed to delete session:" << errorMessage;
        return false;
    }

    return true;
}
