#ifndef FILTERSREPOSITORY_H
#define FILTERSREPOSITORY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include <expected>
#include "ER.h"

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
    std::expected<QVector<FilterData>, ER> getAllFilters() const;
    std::expected<void, ER> addFilter(const QString &name, int filterTypeId) const;
    std::expected<void, ER> updateFilter(int id, const QString &name, int filterTypeId) const;
    std::expected<void, ER> deleteFilter(int id) const;

private:
    DatabaseManager *m_dbManager;
};

#endif // FILTERSREPOSITORY_H
