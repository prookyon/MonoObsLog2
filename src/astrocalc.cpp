#include "astrocalc.h"
extern "C"
{
#include "novas.h"
}
#include <cmath>

ObjectInfo AstroCalc::getObjectInfo(double lat, double lon, double raHours, double decDegrees)
{

    cat_entry cat; // Structure to contain information on sidereal source
    novas_init_cat_entry(&cat, nullptr, raHours, decDegrees);

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
    double jd_rise = novas_rises_above(0.0, &object, &obs_frame, novas_standard_refraction);
    double jd_set = novas_sets_below(0.0, &object, &obs_frame, novas_standard_refraction);

    double az, el;
    novas_app_to_hor(&obs_frame, NOVAS_ICRS, raHours, decDegrees, nullptr, &az, &el);

    int year, month, day;
    double hour;
    novas_jd_to_date(jd_transit, NOVAS_GREGORIAN_CALENDAR, &year, &month, &day, &hour);
    int fullHour = static_cast<int>(hour);
    int minutes = static_cast<int>(hour * 60) % 60;
    QDateTime transitTime(QDate(year, month, day), QTime(fullHour, minutes), QTimeZone::UTC);

    novas_jd_to_date(jd_rise, NOVAS_GREGORIAN_CALENDAR, &year, &month, &day, &hour);
    fullHour = static_cast<int>(hour);
    minutes = static_cast<int>(hour * 60) % 60;
    QDateTime riseTime(QDate(year, month, day), QTime(fullHour, minutes), QTimeZone::UTC);

    novas_jd_to_date(jd_set, NOVAS_GREGORIAN_CALENDAR, &year, &month, &day, &hour);
    fullHour = static_cast<int>(hour);
    minutes = static_cast<int>(hour * 60) % 60;
    QDateTime setTime(QDate(year, month, day), QTime(fullHour, minutes), QTimeZone::UTC);

    // return QDateTime(QDate(year, month, day), QTime(fullHour, minutes), QTimeZone::UTC);
    return ObjectInfo{transitTime, riseTime, setTime, az, el};
}

/// @brief Calculates Moon illumination percentage, RA and Dec for given time and location
/// @param time - local time
/// @param lat - latitude
/// @param lon - longitude
/// @param illumination - returned illumination percentage
/// @param ra - returned RA (decimal degrees)
/// @param dec - returned Dec (decimal degrees)
void AstroCalc::moonInfoForDate(const QDateTime& time, double lat, double lon, double *illumination, double *ra, double *dec)
{
    QDateTime utc = time.toUTC();
    double jd = julian_date(static_cast<short>(utc.date().year()), static_cast<short>(utc.date().month()), static_cast<short>(utc.date().day()), utc.time().hour() + utc.time().minute() / 60.0 + utc.time().second() / 3600.0);

    observer obs;
    make_gps_observer(lat, lon, 0.0, &obs);

    int leap_seconds = 37;   // [s] UTC - TAI time difference
    double dut1 = 0.042;     // [s] UT1 - UTC time difference
    novas_timespec obs_time; // Structure that will define astrometric time
    novas_set_time(NOVAS_UTC, jd, leap_seconds, dut1, &obs_time);

    novas_frame obs_frame; // Structure that will define the observing frame
    double xp = 0;         // [mas] Earth polar offset x, e.g. from IERS Bulletin A.
    double yp = 0;         // [mas] Earth polar offset y, from same source as above.
    // Initialize the observing frame with the given observing parameters
    novas_make_frame(NOVAS_REDUCED_ACCURACY, &obs, &obs_time, xp, yp, &obs_frame);

    object moon;
    novas_orbital moon_orbit = NOVAS_ORBIT_INIT;
    novas_make_moon_orbit(jd, &moon_orbit);
    make_orbital_object("Moon", -1, &moon_orbit, &moon);

    *illumination = 100.0 * novas_solar_illum(&moon, &obs_frame);

    sky_pos moonPos;
    novas_sky_pos(&moon, &obs_frame, NOVAS_ICRS, &moonPos);

    *ra = 15 * moonPos.ra;
    *dec = moonPos.dec;
}