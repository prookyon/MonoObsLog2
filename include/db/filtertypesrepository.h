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
    ~FilterTypesRepository() override = default;

    // Query operations
    QVector<FilterTypeData> getAllFilterTypes(QString &errorMessage) const;
    bool addFilterType(const QString &name, int priority, QString &errorMessage) const;
    bool updateFilterType(int id, const QString &name, int priority, QString &errorMessage) const;
    bool deleteFilterType(int id, QString &errorMessage) const;

private:
    DatabaseManager *m_dbManager;
};

#endif // FILTERTYPESREPOSITORY_H