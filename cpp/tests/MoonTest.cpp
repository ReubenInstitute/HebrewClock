/* MoonTest.cpp
 * Manual verification harness for Moon.h/Moon.cpp, cross-checked against
 * LIBS/Moon.py output for the same inputs.
 */
#include "Date.h"
#include "Geo.h"
#include "Moon.h"

#include <stdio.h>

int main()
{
	Location jerusalem(31.7683, 35.2137);
	DateTime dt(Date(2026, 9, 15), Time(0, 0, 0));

	Moon moon(jerusalem, dt);

	CelestialLocation mcl = moon.celestialLocation();
	printf("moon_celestial %.10f %.10f\n", mcl.rightascension, mcl.declination);
	printf("moon_distance %.10f\n", moon.distance());
	printf("moon_altitude %.10f\n", moon.altitude());

	double alts[25];
	moon.altitudes(alts);
	printf("moon_altitudes");
	for (int i = 0; i < 25; i++) {
		printf(" %.10f", alts[i]);
	}
	printf("\n");

	DateTime* mrise = moon.rise();
	if (mrise) {
		printf("moon_rise %d %d %d %d\n", mrise->date.day, mrise->time.hour, mrise->time.minute, mrise->time.second);
	} else {
		printf("moon_rise None\n");
	}
	DateTime* mset = moon.set();
	if (mset) {
		printf("moon_set %d %d %d %d\n", mset->date.day, mset->time.hour, mset->time.minute, mset->time.second);
	} else {
		printf("moon_set None\n");
	}
	printf("moon_alwaysup %d\n", moon.isAlwaysUp());

	MoonIllumination ill = moon.illumination();
	printf("moon_phase %.10f\n", ill.phase());
	printf("moon_phasetype %d\n", static_cast<int>(ill.phaseType()));
	printf("moon_phaseemoji %s\n", ill.phaseEmoji());
	printf("moon_phasetype_en %s\n", MoonPhaseTypeNames(Lang::EN, ill.phaseType()).name());
	printf("moon_phasetype_he %s\n", MoonPhaseTypeNames(Lang::HE, ill.phaseType()).name());

	return 0;
}
