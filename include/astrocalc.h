#ifndef ASTROCALC_H
#define ASTROCALC_H

#include <QDateTime>

struct EquatorialCoords
{
    double ra;  // right ascension in hours
    double dec; // declination in degrees
};

class AstroCalc
{
public:
    static QDateTime transitTime(double lat, double lon, double raHours, double decDegrees);
};

#endif // ASTROCALC_H