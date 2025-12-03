#ifndef OBJECTSREPOSITORY_H
#define OBJECTSREPOSITORY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include <expected>
#include "ER.h"

class DatabaseManager;

struct ObjectData
{
    int id;
    QString name;
    QVariant ra;  // Can be null
    QVariant dec; // Can be null
    QString comments;
};

class ObjectsRepository : public QObject
{
    Q_OBJECT

public:
    explicit ObjectsRepository(DatabaseManager *dbManager, QObject *parent = nullptr);
    ~ObjectsRepository() override = default;

    // Query operations
    std::expected<QVector<ObjectData>, ER> getAllObjects() const;
    std::expected<void, ER> addObject(const QString &name, const QVariant &ra, const QVariant &dec, const QString &comments) const;
    std::expected<void, ER> updateObject(int id, const QString &name, const QVariant &ra, const QVariant &dec, const QString &comments) const;
    std::expected<void, ER> deleteObject(int id) const;

private:
    DatabaseManager *m_dbManager;
};

#endif // OBJECTSREPOSITORY_H