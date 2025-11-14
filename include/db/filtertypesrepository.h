#ifndef FILTERTYPESREPOSITORY_H
#define FILTERTYPESREPOSITORY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>

class DatabaseManager;

struct FilterTypeData
{
    int id;
    QString name;
    int priority;
};

class FilterTypesRepository : public QObject
{
    Q_OBJECT

public:
    explicit FilterTypesRepository(DatabaseManager *dbManager, QObject *parent = nullptr);
    ~FilterTypesRepository() = default;

    // Query operations
    QVector<FilterTypeData> getAllFilterTypes(QString &errorMessage);
    bool addFilterType(const QString &name, int priority, QString &errorMessage);
    bool updateFilterType(int id, const QString &name, int priority, QString &errorMessage);
    bool deleteFilterType(int id, QString &errorMessage);

private:
    DatabaseManager *m_dbManager;
};

#endif // FILTERTYPESREPOSITORY_H