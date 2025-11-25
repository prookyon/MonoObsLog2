#ifndef SIMBADQUERY_H
#define SIMBADQUERY_H


#include <QNetworkReply>

/**
 * @brief The SimbadQuery class queries astronomical object coordinates from SIMBAD database
 *
 * This class provides an interface to query the SIMBAD astronomical database
 * for celestial object coordinates (Right Ascension and Declination).
 *
 * Usage example:
 * @code
 * SimbadQuery *query = new SimbadQuery(this);
 * connect(query, &SimbadQuery::coordinatesReceived, [](double ra, double dec) {
 *     qDebug() << "RA:" << ra << "Dec:" << dec;
 * });
 * connect(query, &SimbadQuery::errorOccurred, [](const QString &error) {
 *     qDebug() << "Error:" << error;
 * });
 * query->queryObject("M31");
 * @endcode
 */
class SimbadQuery final : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new SimbadQuery object
     * @param parent Parent QObject
     */
    explicit SimbadQuery(QObject *parent = nullptr);

    /**
     * @brief Destroy the SimbadQuery object
     */
    ~SimbadQuery() override;

    /**
     * @brief Query SIMBAD database for object coordinates
     * @param objectName Name of the astronomical object (e.g., "M31", "NGC 1234", "Betelgeuse")
     *
     * This method initiates an asynchronous query to SIMBAD. Results will be
     * delivered via the coordinatesReceived signal or errorOccurred signal.
     */
    void queryObject(const QString &objectName);

    /**
     * @brief Cancel any pending query
     */
    void cancelQuery();

signals:
    /**
     * @brief Emitted when coordinates are successfully retrieved
     * @param ra Right Ascension in decimal degrees (0-360)
     * @param dec Declination in decimal degrees (-90 to +90)
     * @param objectName The name of the object that was queried
     */
    void coordinatesReceived(double ra, double dec, const QString &objectName);

    /**
     * @brief Emitted when an error occurs during the query
     * @param error Error message describing what went wrong
     */
    void errorOccurred(const QString &error);

    /**
     * @brief Emitted when query starts
     * @param objectName The name of the object being queried
     */
    void queryStarted(const QString &objectName);

private slots:
    /**
     * @brief Handle network reply from SIMBAD
     */
    void handleNetworkReply();

    /**
     * @brief Handle network errors
     * @param code Network error code
     */
    void handleNetworkError(QNetworkReply::NetworkError code);

private:
    /**
     * @brief Parse VOTable XML response from SIMBAD
     * @param data XML data from SIMBAD
     * @param ra Output parameter for Right Ascension
     * @param dec Output parameter for Declination
     * @return true if parsing successful, false otherwise
     */
    static bool parseVOTableResponse(const QByteArray &data, double &ra, double &dec);

    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_currentReply;
    QString m_currentObjectName;
};

#endif // SIMBADQUERY_H