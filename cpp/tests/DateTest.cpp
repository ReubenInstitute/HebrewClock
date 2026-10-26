/* DateTest.cpp
 * Manual verification harness for Date.h/Date.cpp, cross-checked against
 * LIBS/Date.py output for the same inputs.
 */
#include "Date.h"

#include <stdio.h>

int main()
{
	Date d(2026, 9, 15);
	printf("date_ordinal %ld\n", d.ordinal);
	printf("date_dayofweek %d\n", static_cast<int>(d.dayofweek()));

	Date d2 = Date::fromordinal(d.ordinal);
	printf("roundtrip_eq %d\n", d2 == d);

	Date leap = Date(2024, 2, 28).next();
	printf("leap_next %d %d %d\n", leap.year, leap.month, leap.day);
	Date leap2 = Date(2024, 3, 1).prev();
	printf("leap_prev %d %d %d\n", leap2.year, leap2.month, leap2.day);

	Time t1(23, 59, 50);
	Time t2 = t1 + 20;
	printf("time_wrap %d %d %d\n", t2.hour, t2.minute, t2.second);

	DateTime dt1(Date(2026, 9, 15), Time(23, 59, 50));
	DateTime dt2(Date(2026, 9, 16), Time(0, 0, 10));
	DateTimeSpan span = dt2 - dt1;
	printf("span_bug %ld %ld\n", span.days, span.seconds);

	DateTime dt3 = dt1 + 20;
	printf("add_seconds %d %d %d %d %d %d\n",
	       dt3.date.year, dt3.date.month, dt3.date.day,
	       dt3.time.hour, dt3.time.minute, dt3.time.second);

	double jd = dt1.julianday();
	printf("julianday %.10f\n", jd);
	DateTime dt4 = DateTime::fromjulianday(jd);
	printf("fromjulianday_roundtrip %d %d %d %d %d %d\n",
	       dt4.date.year, dt4.date.month, dt4.date.day,
	       dt4.time.hour, dt4.time.minute, dt4.time.second);

	MonthCalendar mc(2026, 9);
	printf("monthcal ");
	for (int r = 0; r < 6; r++) {
		for (int c = 0; c < 7; c++) {
			printf("%d", mc.dates[r][c] ? 1 : 0);
		}
		printf(" ");
	}
	printf("\n");

	printf("weekday_en %s\n", WeekdayNames(Lang::EN, d.dayofweek()).name());
	printf("weekday_he %s\n", WeekdayNames(Lang::HE, d.dayofweek()).name());
	printf("month_en %s\n", MonthNames(Lang::EN, d.month).name());
	printf("month_he %s\n", MonthNames(Lang::HE, d.month).name());

	YearCalendar yc(2026);
	printf("weekcount %d\n", yc.weekCount);

	return 0;
}
