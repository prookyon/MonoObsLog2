#ifndef FILTERTYPESREPOSITORY_H
#define FILTERTYPESREPOSITORY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include <expected>
#include "ER.h"

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
    std::expected<QVector<FilterTypeData>, ER> getAllFilterTypes() const;
    std::expected<void, ER> addFilterType(const QString &name, int priority) const;
    std::expected<void, ER> updateFilterType(int id, const QString &name, int priority) const;
    std::expected<void, ER> deleteFilterType(int id) const;

private:
    DatabaseManager *m_dbManager;
};

#endif // FILTERTYPESREPOSITORY_H