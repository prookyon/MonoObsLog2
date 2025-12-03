#ifndef OBSERVATIONSREPOSITORY_H
#define OBSERVATIONSREPOSITORY_H

#include <QObject>
#include <QString>
#include <QVector>
#include <expected>
#include "ER.h"

class DatabaseManager;

struct ObservationData
{
    int id;
    int imageCount;
    int exposureLength;
    int totalExposure;
    QString comments;
    int sessionId;
    int objectId;
    int cameraId;
    int telescopeId;
    int filterId;

    // Joined data for display
    QString sessionName;
    QString sessionDate;
    QString objectName;
    QString objectComments;
    QString cameraName;
    QString telescopeName;
    QString filterName;
    double moonIllumination;
    double angularSeparation;
};

class ObservationsRepository : public QObject
{
    Q_OBJECT

public:
    explicit ObservationsRepository(DatabaseManager *dbManager, QObject *parent = nullptr);
    ~ObservationsRepository() override = default;

    // Query operations
    std::expected<QVector<ObservationData>, ER> getAllObservations() const;
    std::expected<QVector<ObservationData>, ER> getObservationsByObject(int objectId) const;
    std::expected<void, ER> addObservation(int imageCount, int exposureLength, const QString &comments,
                                        int sessionId, int objectId, int cameraId, int telescopeId,
                                        int filterId) const;
    std::expected<void, ER> updateObservation(int id, int imageCount, int exposureLength, const QString &comments,
                                            int sessionId, int objectId, int cameraId, int telescopeId,
                                            int filterId) const;
    std::expected<void, ER> deleteObservation(int id) const;

private:
    DatabaseManager *m_dbManager;
};

#endif // OBSERVATIONSREPOSITORY_H