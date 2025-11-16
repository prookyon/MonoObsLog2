#include "astrocalc.h"
extern "C"
{
#include "novas.h"
}
#include <cmath>

QDateTime AstroCalc::transitTime(double lat, double lon, double raHours, double decDegrees)
{

    cat_entry cat; // Structure to contain information on sidereal source
    novas_init_cat_entry(&cat, NULL, raHours, decDegrees);

    object object; // Encapsulates a sidereal or a Solar-system source
    make_cat_object(&cat, &object);

    observer obs; // Structure to contain observer location
    // Specify the location we are observing from, e.g. a GPS / WGS84 location
    // 50.7374 deg N, 7.0982 deg E, 60m elevation
    make_gps_observer(lat, lon, 60.0, &obs);

    int leap_seconds = 37;   // [s] UTC - TAI time difference
    double dut1 = 0.042;     // [s] UT1 - UTC time difference
    novas_timespec obs_time; // Structure that will define astrometric time
    // Set the time of observation to the precise UTC-based UNIX time
    novas_set_current_time(leap_seconds, dut1, &obs_time);

    novas_frame obs_frame; // Structure that will define the observing frame
    double xp = 0;         // [mas] Earth polar offset x, e.g. from IERS Bulletin A.
    double yp = 0;         // [mas] Earth polar offset y, from same source as above.
    // Initialize the observing frame with the given observing parameters
    novas_make_frame(NOVAS_REDUCED_ACCURACY, &obs, &obs_time, xp, yp, &obs_frame);

    // UTC-based Julian day *after* observer frame, of next source transit
    double jd_transit = novas_transit_time(&object, &obs_frame);
    int year, month, day;
    double hour;
    novas_jd_to_date(jd_transit, NOVAS_GREGORIAN_CALENDAR, &year, &month, &day, &hour);
    int fullHour = int(hour);
    int minutes = int(hour * 60) % 60;
    return QDateTime(QDate(year, month, day), QTime(fullHour, minutes));
}