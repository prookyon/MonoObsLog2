#ifndef OBJECTSREPOSITORY_H
#define OBJECTSREPOSITORY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>

class DatabaseManager;

struct ObjectData
{
    int id;
    QString name;
    QVariant ra;  // Can be null
    QVariant dec; // Can be null
};

class ObjectsRepository : public QObject
{
    Q_OBJECT

public:
    explicit ObjectsRepository(DatabaseManager *dbManager, QObject *parent = nullptr);
    ~ObjectsRepository() override = default;

    // Query operations
    QVector<ObjectData> getAllObjects(QString &errorMessage) const;
    bool addObject(const QString &name, const QVariant &ra, const QVariant &dec, QString &errorMessage) const;
    bool updateObject(int id, const QString &name, const QVariant &ra, const QVariant &dec, QString &errorMessage) const;
    bool deleteObject(int id, QString &errorMessage) const;

private:
    DatabaseManager *m_dbManager;
};

#endif // OBJECTSREPOSITORY_H