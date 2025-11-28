#ifndef TELESCOPESREPOSITORY_H
#define TELESCOPESREPOSITORY_H

#include <QVariant>

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
    QVector<TelescopeData> getAllTelescopes(QString &errorMessage) const;
    bool addTelescope(const QString &name, int aperture, double fRatio, int focalLength, QString &errorMessage) const;
    bool updateTelescope(int id, const QString &name, int aperture, double fRatio, int focalLength, QString &errorMessage) const;
    bool deleteTelescope(int id, QString &errorMessage) const;

private:
    DatabaseManager *m_dbManager;
};

#endif // TELESCOPESREPOSITORY_H