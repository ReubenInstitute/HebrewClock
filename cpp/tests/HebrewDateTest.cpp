/* HebrewDateTest.cpp
 * Manual verification harness for HebrewDate.h/HebrewDate.cpp, cross-checked
 * against LIBS/HebrewDate.py output for the same inputs.
 */
#include "Date.h"
#include "HebrewDate.h"

#include <stdio.h>

int main()
{
	HebrewDate d(5786, static_cast<int>(HebrewMonth::TISHREI), 1);
	printf("ordinal_5786_tishrei_1 %ld\n", d.ordinal);

	HebrewDate d2 = HebrewDate::fromdate(Date(2026, 9, 15));
	printf("fromdate %d %d %d\n", d2.year, d2.month, d2.day);
	printf("roundtrip %d\n", HebrewDate::fromordinal(d2.ordinal) == d2);

	int years[] = { 5784, 5785, 5786, 5787, 5788 };
	for (int i = 0; i < 5; i++) {
		int y = years[i];
		printf("isleapyear %d %d numMonthInYear %d\n",
		       y, HebrewDate::isleapyear(y), HebrewDate::numMonthInYear(y));
	}

	printf("elapsedDays_5786 %ld\n", HebrewDate::elapsedDays(5786));
	printf("numDaysInYear_5786 %ld\n", HebrewDate::numDaysInYear(5786));
	printf("hasLongHeshvan_5786 %d\n", HebrewDate::hasLongHeshvan(5786));
	printf("hasShortKislev_5786 %d\n", HebrewDate::hasShortKislev(5786));
	printf("yearLengthType_5786 %d\n", static_cast<int>(HebrewDate::yearLengthType(5786)));
	printf("yeartype_5786 %d\n", static_cast<int>(HebrewDate::yeartype(5786)));
	printf("biblical %d %d\n", HebrewDate::biblical(5786, 7), HebrewDate::biblical(5786, 1));

	struct { int y, m, dd; } tests[] = {
		{ 5786, static_cast<int>(HebrewMonth::NISAN), 1 },
		{ 5786, static_cast<int>(HebrewMonth::ADAR), 1 },
		{ 5786, static_cast<int>(HebrewMonth::TISHREI), 5 },
		{ 5786, static_cast<int>(HebrewMonth::AV), 9 },
	};
	for (int i = 0; i < 4; i++) {
		HebrewDate hd(tests[i].y, tests[i].m, tests[i].dd);
		HebrewDate sh = hd.shabbat();
		printf("dow %d %d %d %d %d %d %d %d %d\n",
		       tests[i].y, tests[i].m, tests[i].dd,
		       static_cast<int>(hd.dayofweek()),
		       sh.year, sh.month, sh.day,
		       static_cast<int>(hd.shabbatType()),
		       static_cast<int>(hd.shabbatChodeshType()));
	}

	HebrewMonthCalendar mc(5786, static_cast<int>(HebrewMonth::TISHREI));
	printf("monthcal ");
	for (int r = 0; r < 6; r++) {
		for (int c = 0; c < 7; c++) {
			printf("%d", mc.dates[r][c] ? 1 : 0);
		}
		printf(" ");
	}
	printf("\n");

	HebrewYearCalendar yc(5786);
	printf("monthcalcount %d\n", yc.monthCalendarCount);
	printf("weekcount %d\n", yc.weekCount);

	printf("hwdd_en %s\n", HebrewWeekdayDayNames(Lang::EN, Weekday::SUNDAY).name());
	printf("hwdd_he %s\n", HebrewWeekdayDayNames(Lang::HE, Weekday::SUNDAY).name());
	printf("hwnn_en %s\n", HebrewWeekdayNightNames(Lang::EN, Weekday::SATURDAY).name());
	printf("hwnn_he %s\n", HebrewWeekdayNightNames(Lang::HE, Weekday::SATURDAY).name());
	printf("hmn_en_adar_leap %s\n", HebrewMonthNames(Lang::EN, 5784, static_cast<int>(HebrewMonth::ADAR)).name());
	printf("hmn_he_adar_leap %s\n", HebrewMonthNames(Lang::HE, 5784, static_cast<int>(HebrewMonth::ADAR)).name());
	printf("hmn_en_adar_nonleap %s\n", HebrewMonthNames(Lang::EN, 5786, static_cast<int>(HebrewMonth::ADAR)).name());
	printf("stn_en %s\n", ShabbatTypeNames(Lang::EN, ShabbatType::ZAHOR).name());
	printf("stn_he %s\n", ShabbatTypeNames(Lang::HE, ShabbatType::ZAHOR).name());
	printf("sctn_en %s\n", ShabbatChodeshTypeNames(Lang::EN, ShabbatChodeshType::ROSH_CHODESH).name());
	printf("sctn_he %s\n", ShabbatChodeshTypeNames(Lang::HE, ShabbatChodeshType::ROSH_CHODESH).name());

	return 0;
}
