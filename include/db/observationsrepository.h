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
    ~ObservationsRepository() = default;

    // Query operations
    QVector<ObservationData> getAllObservations(QString &errorMessage);
    QVector<ObservationData> getObservationsByObject(int objectId, QString &errorMessage);
    bool addObservation(int imageCount, int exposureLength, const QString &comments,
                        int sessionId, int objectId, int cameraId, int telescopeId,
                        int filterId, QString &errorMessage);
    bool updateObservation(int id, int imageCount, int exposureLength, const QString &comments,
                           int sessionId, int objectId, int cameraId, int telescopeId,
                           int filterId, QString &errorMessage);
    bool deleteObservation(int id, QString &errorMessage);

private:
    DatabaseManager *m_dbManager;
};

#endif // OBSERVATIONSREPOSITORY_H