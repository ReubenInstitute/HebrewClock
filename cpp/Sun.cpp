/* Sun.cpp
 * Ported from LIBS/Sun.py
 */
#include "Sun.h"

#include <math.h>

#define SUN_PI 3.14159265358979323846

Sun::Sun(Location location, DateTime datetime)
	: location(location), datetime(datetime)
{
}

void Sun::updateLocation(Location location)
{
	this->location = location;
}

void Sun::updateDateTime(DateTime datetime)
{
	this->datetime = datetime;
}

DateTime Sun::_midnight() const
{
	return DateTime(datetime.date, Time(0, 0, 0));
}

double Sun::_lw() const
{
	return ASTRO_RAD * -location.longitude;
}

double Sun::_n() const
{
	return _julianCycle(_midnight().julianday(), _lw());
}

double Sun::_ds() const
{
	return _approxTransit(0.0, _lw(), _n());
}

double Sun::_M() const
{
	return _solarMeanAnomaly(_ds());
}

double Sun::_solarEclipticLongitude() const
{
	return _eclipticLongitude(_M());
}

double Sun::distance() const
{
	return 149597870.7 + 2500000.0 * cos(
		ASTRO_RAD * (357.5291 + 0.98560028 * datetime.julianDay2K())
	);
}

CelestialLocation Sun::celestialLocation() const
{
	double days = datetime.julianDay2K();
	double M = ASTRO_RAD * (357.5291 + 0.98560028 * days);
	double C = ASTRO_RAD * (1.9148 * sin(M) + 0.02 * sin(2.0 * M) + 0.0003 * sin(3.0 * M));
	double P = ASTRO_RAD * 102.9372;
	double L = M + C + P + SUN_PI;
	double dec = asin(
		sin(0.0) * cos(ASTRO_EARTH_OBLIQUITY) +
		cos(0.0) * sin(ASTRO_EARTH_OBLIQUITY) * sin(L)
	);
	double ra = atan2(
		sin(L) * cos(ASTRO_EARTH_OBLIQUITY) - tan(0.0) * sin(ASTRO_EARTH_OBLIQUITY),
		cos(L)
	);
	return CelestialLocation(ra, dec);
}

double Sun::noonJD() const
{
	return _solarTransitJ(_ds(), _M(), _solarEclipticLongitude());
}

double Sun::setJD() const
{
	double phi = ASTRO_RAD * location.latitude;
	double dec = _declination(_solarEclipticLongitude(), 0.0);
	double h0 = -0.833 * ASTRO_RAD;
	double w = _hourAngle(h0, phi, dec);
	double a = _approxTransit(w, _lw(), _n());
	return _solarTransitJ(a, _M(), _solarEclipticLongitude());
}

double Sun::riseJD() const
{
	return noonJD() - (setJD() - noonJD());
}

DateTime Sun::noon() const
{
	return DateTime::fromjulianday(noonJD());
}

DateTime Sun::set() const
{
	return DateTime::fromjulianday(setJD());
}

DateTime Sun::rise() const
{
	return DateTime::fromjulianday(riseJD());
}

DateTime Sun::dawnTime(double angleDegrees) const
{
	return _timeAtAltitude(-angleDegrees, true);
}

DateTime Sun::duskTime(double angleDegrees) const
{
	return _timeAtAltitude(-angleDegrees, false);
}

DateTime Sun::_timeAtAltitude(double altitudeDeg, bool rising) const
{
	double phi = ASTRO_RAD * location.latitude;
	double dec = _declination(_solarEclipticLongitude(), 0.0);
	double h0 = altitudeDeg * ASTRO_RAD;
	double w = _hourAngle(h0, phi, dec);
	double a = _approxTransit(w, _lw(), _n());
	double Jset = _solarTransitJ(a, _M(), _solarEclipticLongitude());
	double Jnoon = noonJD();
	return DateTime::fromjulianday(rising ? (Jnoon - (Jset - Jnoon)) : Jset);
}

double Sun::_julianCycle(double d, double lw) const
{
	return round(d - ASTRO_J2000 - 0.0009 - lw / (2.0 * SUN_PI));
}

double Sun::_approxTransit(double Ht, double lw, double n) const
{
	return 0.0009 + (Ht + lw) / (2.0 * SUN_PI) + n;
}

double Sun::_solarTransitJ(double ds, double M, double L) const
{
	return ASTRO_J2000 + ds + 0.0053 * sin(M) - 0.0069 * sin(2.0 * L);
}

double Sun::_solarMeanAnomaly(double daysSinceJ2000) const
{
	return ASTRO_RAD * (357.5291 + 0.98560028 * daysSinceJ2000);
}

double Sun::_eclipticLongitude(double M) const
{
	double C = ASTRO_RAD * (1.9148 * sin(M) + 0.02 * sin(2.0 * M) + 0.0003 * sin(3.0 * M));
	double P = ASTRO_RAD * 102.9372;
	return M + C + P + SUN_PI;
}

double Sun::_declination(double L, double b) const
{
	return asin(
		sin(b) * cos(ASTRO_EARTH_OBLIQUITY) +
		cos(b) * sin(ASTRO_EARTH_OBLIQUITY) * sin(L)
	);
}

double Sun::_rightAscension(double L, double b) const
{
	return atan2(
		sin(L) * cos(ASTRO_EARTH_OBLIQUITY) - tan(b) * sin(ASTRO_EARTH_OBLIQUITY),
		cos(L)
	);
}

double Sun::_siderealTime(double daysSinceJ2000, double lw) const
{
	return ASTRO_RAD * (280.16 + 360.9856235 * daysSinceJ2000) - lw;
}

double Sun::_astroRefraction(double h) const
{
	if (h < 0.0) {
		h = 0.0;
	}
	return 0.0002967 / tan(h + 0.00312536 / (h + 0.08901179));
}

double Sun::_hourAngle(double h, double phi, double dec) const
{
	return acos(
		(sin(h) - sin(phi) * sin(dec)) /
		(cos(phi) * cos(dec))
	);
}
