/* Moon.h
 * Ported from LIBS/Moon.py
 */
#ifndef MOON_H
#define MOON_H

#include "Astro.h"
#include "Date.h"
#include "Geo.h"
#include "Sun.h"

enum class MoonPhaseColor {
	NEW_MOON = 0xB35900,
	WAXING_CRESCENT = 0xC6650A,
	FIRST_QUARTER = 0xD97614,
	WAXING_GIBBOUS = 0xEC8A1E,
	FULL_MOON = 0xFF9800,
	WANING_GIBBOUS = 0xEC8A1E,
	LAST_QUARTER = 0xD97614,
	WANING_CRESCENT = 0xC6650A
};

enum class MoonPhaseType {
	NEW_MOON = 1,
	WAXING_CRESCENT,
	FIRST_QUARTER,
	WAXING_GIBBOUS,
	FULL_MOON,
	WANING_GIBBOUS,
	LAST_QUARTER,
	WANING_CRESCENT
};

class MoonIllumination;   /* defined below; Moon::illumination() returns one by value */

class Moon {
public:
	Location location;
	DateTime datetime;
	Sun sun;

	Moon(Location location, DateTime datetime);

	void updateDateTime(DateTime datetime);

	CelestialLocation celestialLocation() const;
	double distance() const;
	double altitude() const;
	void altitudes(double out[25]) const;   /* was a 25-element list property */

	DateTime* rise();    /* &riseStorage if the moon rises in this window, NULL otherwise */
	DateTime* set();     /* &setStorage if the moon sets in this window, NULL otherwise */
	bool isAlwaysUp();

	MoonIllumination illumination() const;
	const char* emoji() const;

	static double eclipticLongitude(double M);
	static double declination(double L, double b);
	static double rightAscension(double L, double b);
	static double siderealTime(const DateTime& dt, double lw);
	static double astroRefraction(double h);

private:
	bool timesCalculated;
	bool hasRiseValue;
	bool hasSetValue;
	DateTime riseStorage;
	DateTime setStorage;

	void _calculateTimes();
};

class MoonIllumination {
public:
	const Moon& moon;   /* Python stores a reference to the same Moon object; mirrored here, not a copy */

	explicit MoonIllumination(const Moon& moon);

	double phi() const;
	double inc() const;
	double fraction() const;
	double angle() const;
	double phase() const;
	MoonPhaseType phaseType() const;
	const char* phaseEmoji() const;
};

class MoonPhaseTypeNames {
public:
	static const char* NAMES_EN[9];
	static const char* NAMES_HE[9];

	Lang lang;
	MoonPhaseType phaseType;

	MoonPhaseTypeNames(Lang lang, MoonPhaseType phaseType);

	const char* name() const;
};

#endif
