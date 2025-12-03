#include "simbadquery.h"
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QDebug>

SimbadQuery::SimbadQuery(QObject *parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this)), m_currentReply(nullptr)
{
}

SimbadQuery::~SimbadQuery()
{
    cancelQuery();
}

void SimbadQuery::queryObject(const QString &objectName)
{
    if (objectName.isEmpty())
    {
        emit errorOccurred("Object name cannot be empty");
        return;
    }

    // Cancel any existing query
    cancelQuery();

    m_currentObjectName = objectName;
    emit queryStarted(objectName);

    // Build SIMBAD query URL
    // Using SIMBAD's sim-id service which returns VOTable format
    QUrl url("https://simbad.u-strasbg.fr/simbad/sim-id");
    QUrlQuery query;
    query.addQueryItem("output.format", "votable");
    query.addQueryItem("Ident", objectName);
    url.setQuery(query);

    qDebug() << "Querying SIMBAD for object:" << objectName;
    qDebug() << "URL:" << url.toString();

    // Create network request
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "MonoObsLog/1.0");

    // Send GET request
    m_currentReply = m_networkManager->get(request);

    // Connect signals
    connect(m_currentReply, &QNetworkReply::finished,
            this, &SimbadQuery::handleNetworkReply);
    connect(m_currentReply, &QNetworkReply::errorOccurred,
            this, &SimbadQuery::handleNetworkError);
}

void SimbadQuery::cancelQuery()
{
    if (m_currentReply)
    {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

void SimbadQuery::handleNetworkReply()
{
    if (!m_currentReply)
        return;

    // Check for network errors
    if (m_currentReply->error() != QNetworkReply::NoError)
    {
        const QString errorMsg = QString("Network error: %1").arg(m_currentReply->errorString());
        qDebug() << errorMsg;
        emit errorOccurred(errorMsg);
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        return;
    }

    // Read response data
    const QByteArray responseData = m_currentReply->readAll();
    m_currentReply->deleteLater();
    m_currentReply = nullptr;

    qDebug() << "Received response, size:" << responseData.size() << "bytes";

    // Parse the VOTable response
    if (double ra = 0.0, dec = 0.0; parseVOTableResponse(responseData, ra, dec))
    {
        qDebug() << "Successfully parsed coordinates - RA:" << ra << "Dec:" << dec;
        emit coordinatesReceived(ra, dec, m_currentObjectName);
    }
    else
    {
        const QString errorMsg = QString("Failed to parse coordinates for object '%1'").arg(m_currentObjectName);
        qDebug() << errorMsg;
        emit errorOccurred(errorMsg);
    }
}

void SimbadQuery::handleNetworkError(const QNetworkReply::NetworkError code)
{
    if (m_currentReply)
    {
        const QString errorMsg = QString("Network error (%1): %2")
                               .arg(code)
                               .arg(m_currentReply->errorString());
        qDebug() << errorMsg;
        emit errorOccurred(errorMsg);
    }
}

bool SimbadQuery::parseVOTableResponse(const QByteArray &data, double &ra, double &dec)
{
    QXmlStreamReader xml(data);

    bool raFound = false;
    bool decFound = false;
    QString raFieldId;
    QString decFieldId;
    QStringList fieldIds;
    QStringList fieldNames;

    // First pass: find FIELD elements for RA and DEC
    while (!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == QStringLiteral("FIELD"))
        {
            QString id = xml.attributes().value("ID").toString();
            QString name = xml.attributes().value("name").toString();
            QString ucd = xml.attributes().value("ucd").toString();

            fieldIds.append(id);
            fieldNames.append(name);

            // SIMBAD uses these UCDs for coordinates:
            // RA: pos.eq.ra;meta.main
            // DEC: pos.eq.dec;meta.main
            if (ucd.contains("pos.eq.ra;meta.main") || name.toLower() == "ra")
            {
                raFieldId = id;
                qDebug() << "Found RA field:" << id << name << ucd;
            }
            else if (ucd.contains("pos.eq.dec;meta.main") || name.toLower() == "dec")
            {
                decFieldId = id;
                qDebug() << "Found DEC field:" << id << name << ucd;
            }
        }
    }

    // Reset reader for second pass
    xml.clear();
    xml.addData(data);

    // Second pass: extract data from TR/TD elements
    const qsizetype raIndex = fieldIds.indexOf(raFieldId);
    const qsizetype decIndex = fieldIds.indexOf(decFieldId);

    if (raIndex == -1 || decIndex == -1)
    {
        qDebug() << "Could not find RA/DEC field indices";
        return false;
    }

    qDebug() << "RA index:" << raIndex << "DEC index:" << decIndex;

    // Find the data row
    while (!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == QStringLiteral("TR"))
        {
            // Read all TD elements in this row
            QStringList tdValues;

            while (!(xml.isEndElement() && xml.name() == QStringLiteral("TR")))
            {
                xml.readNext();

                if (xml.isStartElement() && xml.name() == QStringLiteral("TD"))
                {
                    QString value = xml.readElementText();
                    tdValues.append(value);
                }
            }

            // Extract RA and DEC from the collected values
            if (tdValues.size() > raIndex && tdValues.size() > decIndex)
            {
                bool raOk = false;
                bool decOk = false;

                ra = tdValues[raIndex].toDouble(&raOk);
                dec = tdValues[decIndex].toDouble(&decOk);

                if (raOk && decOk)
                {
                    raFound = true;
                    decFound = true;
                    qDebug() << "Extracted values - RA:" << ra << "DEC:" << dec;
                    break;
                }
            }
        }
    }

    if (xml.hasError())
    {
        qDebug() << "XML parse error:" << xml.errorString();
        return false;
    }

    return raFound && decFound;
}
