#ifndef CAMERASREPOSITORY_H
#define CAMERASREPOSITORY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>

class DatabaseManager;

struct CameraData
{
    int id;
    QString name;
    QString sensor;
    double pixelSize;
    int width;
    int height;
};

class CamerasRepository : public QObject
{
    Q_OBJECT

public:
    explicit CamerasRepository(DatabaseManager *dbManager, QObject *parent = nullptr);
    ~CamerasRepository() override = default;

    // Query operations
    QVector<CameraData> getAllCameras(QString &errorMessage) const;
    bool addCamera(const QString &name, const QString &sensor, double pixelSize, int width, int height, QString &errorMessage) const;
    bool updateCamera(int id, const QString &name, const QString &sensor, double pixelSize, int width, int height, QString &errorMessage) const;
    bool deleteCamera(int id, QString &errorMessage) const;

private:
    DatabaseManager *m_dbManager;
};

#endif // CAMERASREPOSITORY_H