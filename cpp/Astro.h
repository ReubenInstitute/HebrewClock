/* Astro.h
 * Ported from LIBS/Astro.py
 */
#ifndef ASTRO_H
#define ASTRO_H

#define ASTRO_J2000 2451545.0

/* (PI / 180) * 23.4397, defined in Astro.cpp using a local PI literal
 * rather than relying on the non-standard M_PI macro. */
extern const double ASTRO_EARTH_OBLIQUITY;

/* math.pi / 180.0 -- shared by Sun.py/Moon.py as a module-level `RAD`
 * constant in Python; ported once here rather than redefined per file. */
extern const double ASTRO_RAD;

class CelestialLocation {
public:
	/* NOTE: field is named "rightascension" (all lowercase), matching
	 * Astro.py's self.rightascension exactly -- even though the
	 * constructor parameter is spelled "rightAscension". Astro.py has
	 * this same inconsistent casing; ported faithfully since Moon.py's
	 * MoonIllumination reads it as `.rightascension`. */
	double rightascension;
	double declination;

	CelestialLocation(double rightAscension, double declination);
};

#endif
