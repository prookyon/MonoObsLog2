#ifndef ASTROCALC_H
#define ASTROCALC_H

#include <QDateTime>
#include <QTimeZone>

struct EquatorialCoords
{
    double ra;  // right ascension in hours
    double dec; // declination in degrees
};

struct ObjectInfo
{
    QDateTime transitTime;
    QDateTime riseTime;
    QDateTime setTime;
    double azimuth;
    double altitude;
};

class AstroCalc
{
public:
    static ObjectInfo getObjectInfo(double lat, double lon, double raHours, double decDegrees);
    static void moonInfoForDate(QDateTime time, double lat, double lon, double *illumination, double *ra, double *dec);
};

#endif // ASTROCALC_H