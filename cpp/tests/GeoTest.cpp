/* GeoTest.cpp
 * Manual verification harness for the full Geo.h/Geo.cpp (Location + Place),
 * cross-checked against LIBS/Geo.py output for the same inputs.
 */
#include "Geo.h"

#include <stdio.h>

int main()
{
	printf("count %d\n", Place::ALL_COUNT);
	printf("jerusalem %s %.4f %.4f %.2f\n", Place::JERUSALEM.id, Place::JERUSALEM.latitude,
	       Place::JERUSALEM.longitude, Place::JERUSALEM.area);
	printf("rome %s %.4f %.4f %.2f\n", Place::ROME.id, Place::ROME.latitude,
	       Place::ROME.longitude, Place::ROME.area);

	Location loc = Place::JERUSALEM.location();
	printf("location %.4f %.4f\n", loc.latitude, loc.longitude);

	printf("isDiaspora_jerusalem %d\n", Place::JERUSALEM.isDiaspora());
	printf("isDiaspora_newyork %d\n", Place::NEW_YORK.isDiaspora());

	const Place& p = Place::fromCoordinates(31.7683, 35.2137);
	printf("fromCoordinates_match %s\n", p.id);
	const Place& p2 = Place::fromCoordinates(0.0, 0.0);
	printf("fromCoordinates_nomatch %s\n", p2.id);

	const Place& closest = Place::closest(Location(32.08, 34.78));
	printf("closest %s\n", closest.id);

	printf("eq %d %d\n", Place::JERUSALEM == Place::JERUSALEM, Place::JERUSALEM == Place::TEL_AVIV);

	return 0;
}
