#ifndef FILTERSREPOSITORY_H
#define FILTERSREPOSITORY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>

class DatabaseManager;

struct FilterData
{
    int id;
    QString name;
    int filterTypeId;
    QString filterTypeName;
};

class FiltersRepository : public QObject
{
    Q_OBJECT

public:
    explicit FiltersRepository(DatabaseManager *dbManager, QObject *parent = nullptr);
    ~FiltersRepository() = default;

    // Query operations
    QVector<FilterData> getAllFilters(QString &errorMessage);
    bool addFilter(const QString &name, int filterTypeId, QString &errorMessage);
    bool updateFilter(int id, const QString &name, int filterTypeId, QString &errorMessage);
    bool deleteFilter(int id, QString &errorMessage);

private:
    DatabaseManager *m_dbManager;
};

#endif // FILTERSREPOSITORY_H
