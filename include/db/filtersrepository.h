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
    ~FiltersRepository() override = default;

    // Query operations
    QVector<FilterData> getAllFilters(QString &errorMessage) const;
    bool addFilter(const QString &name, int filterTypeId, QString &errorMessage) const;
    bool updateFilter(int id, const QString &name, int filterTypeId, QString &errorMessage) const;
    bool deleteFilter(int id, QString &errorMessage) const;

private:
    DatabaseManager *m_dbManager;
};

#endif // FILTERSREPOSITORY_H
