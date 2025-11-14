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
    ~ObjectsRepository() = default;

    // Query operations
    QVector<ObjectData> getAllObjects(QString &errorMessage);
    bool addObject(const QString &name, const QVariant &ra, const QVariant &dec, QString &errorMessage);
    bool updateObject(int id, const QString &name, const QVariant &ra, const QVariant &dec, QString &errorMessage);
    bool deleteObject(int id, QString &errorMessage);

private:
    DatabaseManager *m_dbManager;
};

#endif // OBJECTSREPOSITORY_H