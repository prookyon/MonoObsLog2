#ifndef SESSIONSREPOSITORY_H
#define SESSIONSREPOSITORY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QDate>

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
    ~SessionsRepository() = default;

    // Query operations
    QVector<SessionData> getAllSessions(QString &errorMessage);
    bool addSession(const QString &name, const QDate &startDate, const QString &comments, QString &errorMessage);
    bool updateSession(int id, const QString &name, const QDate &startDate, const QString &comments, QString &errorMessage);
    bool deleteSession(int id, QString &errorMessage);

private:
    DatabaseManager *m_dbManager;
};

#endif // SESSIONSREPOSITORY_H
