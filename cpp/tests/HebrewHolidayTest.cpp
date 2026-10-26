/* HebrewHolidayTest.cpp
 * Manual verification harness for HebrewHoliday.h/HebrewHoliday.cpp,
 * cross-checked against LIBS/HebrewHoliday.py output for the same inputs.
 */
#include "Date.h"
#include "HebrewDate.h"
#include "HebrewHoliday.h"

#include <stdio.h>

int main()
{
	int years[] = { 5784, 5786 };
	for (int yi = 0; yi < 2; yi++) {
		int year = years[yi];
		HebrewDate start(year, static_cast<int>(HebrewMonth::TISHREI), 1);
		HebrewDate end(year + 1, static_cast<int>(HebrewMonth::TISHREI), 1);
		HebrewDate h = start;
		while (h.ordinal < end.ordinal) {
			HebrewHoliday hol(h);
			if (hol.id != HebrewHolidayId::NONE) {
				HebrewHoliday moed, erev;
				bool hasMoed = hol.moed(moed);
				bool hasErev = hol.erev(erev);
				printf("%d %d %d %d %d %d %d %d %d %d %d %d ",
				       year, h.year, h.month, h.day, static_cast<int>(hol.id),
				       hol.is_shabbat(), hol.is_rest_day(), hol.is_festive(),
				       hol.is_fast(), hol.is_national(), hol.is_solemn(),
				       static_cast<int>(hol.diaspora()));
				if (hasMoed) {
					printf("%d ", static_cast<int>(moed.id));
				} else {
					printf("None ");
				}
				if (hasErev) {
					printf("%d\n", static_cast<int>(erev.id));
				} else {
					printf("None\n");
				}
			}
			h = h.next();
		}
	}
	return 0;
}
