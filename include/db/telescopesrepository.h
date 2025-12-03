#ifndef TELESCOPESREPOSITORY_H
#define TELESCOPESREPOSITORY_H

#include <QVariant>
#include <QVector>
#include <expected>
#include "ER.h"

class DatabaseManager;

struct TelescopeData
{
    int id;
    QString name;
    int aperture;
    double fRatio;
    int focalLength;
};

class TelescopesRepository : public QObject
{
    Q_OBJECT

public:
    explicit TelescopesRepository(DatabaseManager *dbManager, QObject *parent = nullptr);
    ~TelescopesRepository() override = default;

    // Query operations
    std::expected<QVector<TelescopeData>, ER> getAllTelescopes() const;
    std::expected<void, ER> addTelescope(const QString &name, int aperture, double fRatio, int focalLength) const;
    std::expected<void, ER> updateTelescope(int id, const QString &name, int aperture, double fRatio, int focalLength) const;
    std::expected<void, ER> deleteTelescope(int id) const;

private:
    DatabaseManager *m_dbManager;
};

#endif // TELESCOPESREPOSITORY_H