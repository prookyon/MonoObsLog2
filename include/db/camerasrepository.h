#ifndef CAMERASREPOSITORY_H
#define CAMERASREPOSITORY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include "ER.h"
#include <expected>

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
    std::expected<QVector<CameraData>, ER> getAllCameras() const;
    std::expected<void, ER> addCamera(const QString &name, const QString &sensor, double pixelSize, int width, int height) const;
    std::expected<void, ER> updateCamera(int id, const QString &name, const QString &sensor, double pixelSize, int width, int height) const;
    std::expected<void, ER> deleteCamera(int id) const;

private:
    DatabaseManager *m_dbManager;
};

#endif // CAMERASREPOSITORY_H