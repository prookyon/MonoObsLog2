#ifndef TELESCOPESREPOSITORY_H
#define TELESCOPESREPOSITORY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>

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
    ~TelescopesRepository() = default;

    // Query operations
    QVector<TelescopeData> getAllTelescopes(QString &errorMessage);
    bool addTelescope(const QString &name, int aperture, double fRatio, int focalLength, QString &errorMessage);
    bool updateTelescope(int id, const QString &name, int aperture, double fRatio, int focalLength, QString &errorMessage);
    bool deleteTelescope(int id, QString &errorMessage);

private:
    DatabaseManager *m_dbManager;
};

#endif // TELESCOPESREPOSITORY_H