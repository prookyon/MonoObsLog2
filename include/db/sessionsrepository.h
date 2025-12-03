#ifndef SESSIONSREPOSITORY_H
#define SESSIONSREPOSITORY_H

#include <QDate>
#include <QVector>
#include <expected>
#include "ER.h"

class DatabaseManager;

struct SessionData
{
    int id;
    QString name;
    QDate startDate;
    QVariant moonIllumination; // Can be null
    QVariant moonRa;           // Can be null
    QVariant moonDec;          // Can be null
    QString comments;
    QVariant exposureTotal; // Can be null (in hours)
};

class SessionsRepository : public QObject
{
    Q_OBJECT

public:
    explicit SessionsRepository(DatabaseManager *dbManager, QObject *parent = nullptr);
    ~SessionsRepository() override = default;

    // Query operations
    std::expected<QVector<SessionData>, ER> getAllSessions() const;
    std::expected<void, ER> addSession(const QString &name, const QDate &startDate, const QString &comments, const double &moonIllumination, const double &moonRa, const double &moonDec) const;
    std::expected<void, ER> updateSession(int id, const QString &name, const QDate &startDate, const QString &comments, const double &moonIllumination, const double &moonRa, const double &moonDec) const;
    std::expected<void, ER> deleteSession(int id) const;

private:
    DatabaseManager *m_dbManager;
};

#endif // SESSIONSREPOSITORY_H
