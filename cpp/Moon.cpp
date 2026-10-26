/* Moon.cpp
 * Ported from LIBS/Moon.py
 */
#include "Moon.h"

#include <math.h>
#include <stddef.h>

#define MOON_PI 3.14159265358979323846

/* ---------- Moon ---------- */

Moon::Moon(Location location, DateTime datetime)
	: location(location), datetime(datetime), sun(location, datetime),
	  timesCalculated(false), hasRiseValue(false), hasSetValue(false),
	  riseStorage(datetime), setStorage(datetime)
{
}

void Moon::updateDateTime(DateTime datetime)
{
	this->datetime = datetime;
	sun.updateDateTime(datetime);
	timesCalculated = false;   /* invalidate cached rise/set (Python always recomputes fresh) */
}

CelestialLocation Moon::celestialLocation() const
{
	double days = datetime.julianDay2K();
	double L = ASTRO_RAD * (218.316 + 13.176396 * days);
	double M = ASTRO_RAD * (134.963 + 13.064993 * days);
	double F = ASTRO_RAD * (93.272 + 13.229350 * days);
	double l = L + ASTRO_RAD * 6.289 * sin(M);
	double b = ASTRO_RAD * 5.128 * sin(F);
	double ra = atan2(
		sin(l) * cos(ASTRO_EARTH_OBLIQUITY) - tan(b) * sin(ASTRO_EARTH_OBLIQUITY),
		cos(l)
	);
	double dec = asin(
		sin(b) * cos(ASTRO_EARTH_OBLIQUITY) +
		cos(b) * sin(ASTRO_EARTH_OBLIQUITY) * sin(l)
	);
	return CelestialLocation(ra, dec);
}

double Moon::distance() const
{
	double days = datetime.julianDay2K();
	double M = ASTRO_RAD * (134.963 + 13.064993 * days);
	return 385001.0 - 20905.0 * cos(M);
}

double Moon::altitude() const
{
	double lw = ASTRO_RAD * -location.longitude;
	double phi = ASTRO_RAD * location.latitude;
	CelestialLocation moonDir = celestialLocation();
	double H = Moon::siderealTime(datetime, lw) - moonDir.rightascension;
	double h = asin(sin(phi) * sin(moonDir.declination) +
	                 cos(phi) * cos(moonDir.declination) * cos(H));

	/* Parallax correction (essential for the moon!) */
	double parallax = asin(6371.0 / distance()) * cos(h);
	h = h - parallax;   /* Moon appears lower due to parallax */

	return h + Moon::astroRefraction(h);
}

void Moon::altitudes(double out[25]) const
{
	for (int h = 0; h < 25; h++) {
		DateTime t = (h == 24)
			? DateTime(datetime.date.next(), Time(0, 0, 0))
			: DateTime(datetime.date, Time(h, 0, 0));
		Moon m(location, t);
		out[h] = m.altitude();
	}
}

void Moon::_calculateTimes()
{
	if (timesCalculated) {
		return;
	}
	timesCalculated = true;

	double alts[25];
	altitudes(alts);

	bool riseFound = false;
	bool setFound = false;
	double riseHour = 0.0;
	double setHour = 0.0;

	for (int i = 0; i < 23; i++) {
		double h0 = alts[i];
		double h1 = alts[i + 1];
		double h2 = alts[i + 2];

		double a = (h0 + h2) / 2.0 - h1;
		double b = (h2 - h0) / 2.0;
		double d = b * b - 4.0 * a * h1;
		int roots = 0;
		double x1 = 0.0;
		double x2 = 0.0;

		if (a != 0.0) {
			double xe = -b / (2.0 * a);
			double ye = (a * xe + b) * xe + h1;
			if (d >= 0.0) {
				double dx = sqrt(d) / (fabs(a) * 2.0);
				x1 = xe - dx;
				x2 = xe + dx;
				if (fabs(x1) <= 1.0) {
					roots++;
				}
				if (fabs(x2) <= 1.0) {
					roots++;
				}
				if (x1 < -1.0) {
					x1 = x2;
				}
			}

			if (roots == 1) {
				if (h0 < 0.0) {
					if (!riseFound) {
						riseFound = true;
						riseHour = (i + 1) + x1;
					}
				} else {
					if (!setFound) {
						setFound = true;
						setHour = (i + 1) + x1;
					}
				}
			} else if (roots == 2) {
				if (!riseFound) {
					riseFound = true;
					riseHour = (i + 1) + (ye < 0.0 ? x2 : x1);
				}
				if (!setFound) {
					setFound = true;
					setHour = (i + 1) + (ye < 0.0 ? x1 : x2);
				}
			}
		}

		if (riseFound && setFound) {
			break;
		}
	}

	hasRiseValue = riseFound;
	hasSetValue = setFound;
	if (riseFound) {
		DateTime midnight(datetime.date, Time(0, 0, 0));
		riseStorage = midnight + static_cast<long>(riseHour * 3600);
	}
	if (setFound) {
		DateTime midnight(datetime.date, Time(0, 0, 0));
		setStorage = midnight + static_cast<long>(setHour * 3600);
	}
}

DateTime* Moon::rise()
{
	_calculateTimes();
	return hasRiseValue ? &riseStorage : NULL;
}

DateTime* Moon::set()
{
	_calculateTimes();
	return hasSetValue ? &setStorage : NULL;
}

bool Moon::isAlwaysUp()
{
	_calculateTimes();
	if (!hasRiseValue && !hasSetValue) {
		DateTime noon(datetime.date, Time(12, 0, 0));
		Moon m(location, noon);
		return m.altitude() > 0.0;
	}
	return false;
}

MoonIllumination Moon::illumination() const
{
	return MoonIllumination(*this);
}

const char* Moon::emoji() const
{
	return illumination().phaseEmoji();
}

double Moon::eclipticLongitude(double M)
{
	double C = ASTRO_RAD * (1.9148 * sin(M) + 0.02 * sin(2.0 * M) + 0.0003 * sin(3.0 * M));
	double P = ASTRO_RAD * 102.9372;
	return M + C + P + MOON_PI;
}

double Moon::declination(double L, double b)
{
	return asin(
		sin(b) * cos(ASTRO_EARTH_OBLIQUITY) +
		cos(b) * sin(ASTRO_EARTH_OBLIQUITY) * sin(L)
	);
}

double Moon::rightAscension(double L, double b)
{
	return atan2(
		sin(L) * cos(ASTRO_EARTH_OBLIQUITY) - tan(b) * sin(ASTRO_EARTH_OBLIQUITY),
		cos(L)
	);
}

double Moon::siderealTime(const DateTime& dt, double lw)
{
	double days = dt.julianDay2K();
	return ASTRO_RAD * (280.16 + 360.9856235 * days) - lw;
}

double Moon::astroRefraction(double h)
{
	if (h < 0.0) {
		h = 0.0;
	}
	return 0.0002967 / tan(h + 0.00312536 / (h + 0.08901179));
}

/* ---------- MoonIllumination ---------- */

MoonIllumination::MoonIllumination(const Moon& moon)
	: moon(moon)
{
}

double MoonIllumination::phi() const
{
	CelestialLocation sunLocation = moon.sun.celestialLocation();
	CelestialLocation moonLocation = moon.celestialLocation();
	return acos(
		sin(sunLocation.declination) * sin(moonLocation.declination) +
		cos(sunLocation.declination) * cos(moonLocation.declination) *
		cos(sunLocation.rightascension - moonLocation.rightascension)
	);
}

double MoonIllumination::inc() const
{
	return atan2(
		moon.sun.distance() * sin(phi()),
		moon.distance() - moon.sun.distance() * cos(phi())
	);
}

double MoonIllumination::fraction() const
{
	return (1.0 + cos(inc())) / 2.0;
}

double MoonIllumination::angle() const
{
	CelestialLocation sunLocation = moon.sun.celestialLocation();
	CelestialLocation moonLocation = moon.celestialLocation();
	return atan2(
		cos(sunLocation.declination) * sin(sunLocation.rightascension - moonLocation.rightascension),
		sin(sunLocation.declination) * cos(moonLocation.declination) -
		cos(sunLocation.declination) * sin(moonLocation.declination) *
		cos(sunLocation.rightascension - moonLocation.rightascension)
	);
}

double MoonIllumination::phase() const
{
	return 0.5 + 0.5 * inc() * (angle() < 0.0 ? -1.0 : 1.0) / MOON_PI;
}

MoonPhaseType MoonIllumination::phaseType() const
{
	double p = phase();
	if (p < 0.0625 || p >= 0.9375) {
		return MoonPhaseType::NEW_MOON;
	}
	if (p < 0.3125) {
		return MoonPhaseType::WAXING_CRESCENT;
	}
	if (p < 0.4375) {
		return MoonPhaseType::FIRST_QUARTER;
	}
	if (p < 0.5625) {
		return MoonPhaseType::FULL_MOON;
	}
	if (p < 0.6875) {
		return MoonPhaseType::WANING_GIBBOUS;
	}
	if (p < 0.8125) {
		return MoonPhaseType::LAST_QUARTER;
	}
	return MoonPhaseType::WANING_CRESCENT;
}

const char* MoonIllumination::phaseEmoji() const
{
	bool north = moon.location.latitude >= 0;
	MoonPhaseType t = phaseType();
	if (t == MoonPhaseType::NEW_MOON) {
		return "\xf0\x9f\x8c\x91";                              /* 🌑 */
	}
	if (t == MoonPhaseType::WAXING_CRESCENT) {
		return north ? "\xf0\x9f\x8c\x92" : "\xf0\x9f\x8c\x98"; /* 🌒 : 🌘 */
	}
	if (t == MoonPhaseType::FIRST_QUARTER) {
		return north ? "\xf0\x9f\x8c\x93" : "\xf0\x9f\x8c\x97"; /* 🌓 : 🌗 */
	}
	if (t == MoonPhaseType::FULL_MOON) {
		return "\xf0\x9f\x8c\x95";                              /* 🌕 */
	}
	if (t == MoonPhaseType::WANING_GIBBOUS) {
		return north ? "\xf0\x9f\x8c\x96" : "\xf0\x9f\x8c\x94"; /* 🌖 : 🌔 */
	}
	if (t == MoonPhaseType::LAST_QUARTER) {
		return north ? "\xf0\x9f\x8c\x97" : "\xf0\x9f\x8c\x93"; /* 🌗 : 🌓 */
	}
	if (t == MoonPhaseType::WANING_CRESCENT) {
		return north ? "\xf0\x9f\x8c\x98" : "\xf0\x9f\x8c\x92"; /* 🌘 : 🌒 */
	}
	return "\xf0\x9f\x8c\x91";                                  /* 🌑 */
}

/* ---------- MoonPhaseTypeNames ---------- */

const char* MoonPhaseTypeNames::NAMES_EN[9] = {
	"", "New Moon", "Waxing Crescent", "First Quarter", "Waxing Gibbous",
	"Full Moon", "Waning Gibbous", "Last Quarter", "Waning Crescent"
};

const char* MoonPhaseTypeNames::NAMES_HE[9] = {
	"",
	"\xd7\x9e\xd7\x95\xd7\x9c\xd7\x93",
	"\xd7\xa1\xd7\x94\xd7\xa8 \xd7\xa2\xd7\x95\xd7\x9c\xd7\x94",
	"\xd7\xa8\xd7\x91\xd7\xa2 \xd7\xa8\xd7\x90\xd7\xa9\xd7\x95\xd7\x9f",
	"\xd7\x92\xd7\x91\xd7\xa0\xd7\x95\xd7\x9f \xd7\xa2\xd7\x95\xd7\x9c\xd7\x94",
	"\xd7\x99\xd7\xa8\xd7\x97 \xd7\x9e\xd7\x9c\xd7\x90",
	"\xd7\x92\xd7\x91\xd7\xa0\xd7\x95\xd7\x9f \xd7\x99\xd7\x95\xd7\xa8\xd7\x93",
	"\xd7\xa8\xd7\x91\xd7\xa2 \xd7\x90\xd7\x97\xd7\xa8\xd7\x95\xd7\x9f",
	"\xd7\xa1\xd7\x94\xd7\xa8 \xd7\x99\xd7\x95\xd7\xa8\xd7\x93"
};

MoonPhaseTypeNames::MoonPhaseTypeNames(Lang lang, MoonPhaseType phaseType)
{
	this->lang = lang;
	this->phaseType = phaseType;
}

const char* MoonPhaseTypeNames::name() const
{
	return (lang == Lang::EN) ? NAMES_EN[static_cast<int>(phaseType)] : NAMES_HE[static_cast<int>(phaseType)];
}
