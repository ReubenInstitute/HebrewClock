/* Sun.h
 * Ported from LIBS/Sun.py
 */
#ifndef SUN_H
#define SUN_H

#include "Astro.h"
#include "Date.h"
#include "Geo.h"

class Sun {
public:
	Location location;
	DateTime datetime;

	Sun(Location location, DateTime datetime);

	void updateLocation(Location location);
	void updateDateTime(DateTime datetime);

	double distance() const;
	CelestialLocation celestialLocation() const;
	double noonJD() const;
	double setJD() const;
	double riseJD() const;
	DateTime noon() const;
	DateTime set() const;
	DateTime rise() const;

	DateTime dawnTime(double angleDegrees) const;
	DateTime duskTime(double angleDegrees) const;

private:
	/* Python declared these as plain instance methods (not @staticmethod),
	 * even though most don't read member state beyond location/datetime --
	 * ported as regular (non-static) private methods to match. */
	DateTime _midnight() const;
	double _lw() const;
	double _n() const;
	double _ds() const;
	double _M() const;
	double _solarEclipticLongitude() const;

	DateTime _timeAtAltitude(double altitudeDeg, bool rising) const;
	double _julianCycle(double d, double lw) const;
	double _approxTransit(double Ht, double lw, double n) const;
	double _solarTransitJ(double ds, double M, double L) const;
	double _solarMeanAnomaly(double daysSinceJ2000) const;
	double _eclipticLongitude(double M) const;
	double _declination(double L, double b) const;
	double _rightAscension(double L, double b) const;
	double _siderealTime(double daysSinceJ2000, double lw) const;
	double _astroRefraction(double h) const;
	double _hourAngle(double h, double phi, double dec) const;
};

#endif
