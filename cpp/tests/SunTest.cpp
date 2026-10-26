/* SunTest.cpp
 * Manual verification harness for Sun.h/Sun.cpp and Geo.h/Geo.cpp,
 * cross-checked against LIBS/Sun.py and LIBS/Geo.py output for the same
 * inputs.
 */
#include "Date.h"
#include "Geo.h"
#include "Sun.h"

#include <stdio.h>

int main()
{
	Location jerusalem(31.7683, 35.2137);
	Location telaviv(32.0853, 34.7818);
	DateTime dt(Date(2026, 9, 15), Time(0, 0, 0));

	Sun sun(jerusalem, dt);

	DateTime n = sun.noon();
	printf("sun_noon %d %d %d %d\n", n.date.day, n.time.hour, n.time.minute, n.time.second);
	DateTime r = sun.rise();
	printf("sun_rise %d %d %d %d\n", r.date.day, r.time.hour, r.time.minute, r.time.second);
	DateTime s = sun.set();
	printf("sun_set %d %d %d %d\n", s.date.day, s.time.hour, s.time.minute, s.time.second);
	printf("sun_distance %.10f\n", sun.distance());
	CelestialLocation cl = sun.celestialLocation();
	printf("sun_celestial %.10f %.10f\n", cl.rightascension, cl.declination);

	DateTime dawn6 = sun.dawnTime(6.0);
	DateTime dusk6 = sun.duskTime(6.0);
	printf("dawn6 %d %d %d %d\n", dawn6.date.day, dawn6.time.hour, dawn6.time.minute, dawn6.time.second);
	printf("dusk6 %d %d %d %d\n", dusk6.date.day, dusk6.time.hour, dusk6.time.minute, dusk6.time.second);
	DateTime dawn85 = sun.dawnTime(8.5);
	DateTime dusk85 = sun.duskTime(8.5);
	printf("dawn85 %d %d %d %d\n", dawn85.date.day, dawn85.time.hour, dawn85.time.minute, dawn85.time.second);
	printf("dusk85 %d %d %d %d\n", dusk85.date.day, dusk85.time.hour, dusk85.time.minute, dusk85.time.second);

	printf("loc_distance %.10f\n", jerusalem.distance(telaviv));
	printf("loc_direction %.10f\n", jerusalem.direction(telaviv));

	return 0;
}
