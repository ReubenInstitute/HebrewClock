/* Astro.cpp
 * Ported from LIBS/Astro.py
 */
#include "Astro.h"

#define ASTRO_PI 3.14159265358979323846

const double ASTRO_EARTH_OBLIQUITY = (ASTRO_PI / 180.0) * 23.4397;
const double ASTRO_RAD = ASTRO_PI / 180.0;

CelestialLocation::CelestialLocation(double rightAscension, double declination)
{
	this->rightascension = rightAscension;
	this->declination = declination;
}
