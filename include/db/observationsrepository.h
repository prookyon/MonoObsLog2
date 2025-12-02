#ifndef OBSERVATIONSREPOSITORY_H
#define OBSERVATIONSREPOSITORY_H

#include <QObject>
#include <QString>
#include <QVector>

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
    QVector<ObservationData> getAllObservations(QString &errorMessage) const;
    QVector<ObservationData> getObservationsByObject(int objectId, QString &errorMessage) const;
    bool addObservation(int imageCount, int exposureLength, const QString &comments,
                        int sessionId, int objectId, int cameraId, int telescopeId,
                        int filterId, QString &errorMessage) const;
    bool updateObservation(int id, int imageCount, int exposureLength, const QString &comments,
                           int sessionId, int objectId, int cameraId, int telescopeId,
                           int filterId, QString &errorMessage) const;
    bool deleteObservation(int id, QString &errorMessage) const;

private:
    DatabaseManager *m_dbManager;
};

#endif // OBSERVATIONSREPOSITORY_H