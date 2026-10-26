/* LocalHebrewDateTest.cpp
 * Manual verification harness for LocalHebrewDate.h/LocalHebrewDate.cpp,
 * cross-checked against LIBS/LocalHebrewDate.py output for the same inputs.
 */
#include "Custom.h"
#include "Date.h"
#include "Geo.h"
#include "HebrewDate.h"
#include "LocalHebrewDate.h"

#include <stdio.h>

static void dump(const char* name, const DateTime& dt)
{
	printf("%s %d %d %d %d %d %d\n", name, dt.date.year, dt.date.month, dt.date.day,
	       dt.time.hour, dt.time.minute, dt.time.second);
}

int main()
{
	Location jerusalem(31.7683, 35.2137);
	HebrewDate hd = HebrewDate::fromdate(Date(2026, 9, 15));

	int presets[] = { Custom::LOCAL, Custom::ASHKENAZI_GRA };
	for (int pi = 0; pi < 2; pi++) {
		Custom c(static_cast<Custom::Preset>(presets[pi]));
		LocalHebrewDate local(hd, jerusalem, c);
		printf("=== preset %d ===\n", presets[pi]);

		dump("sunrise", local.sunrise());
		dump("sunset", local.sunset());
		dump("dawn", local.dawn());
		dump("nightfall", local.nightfall());
		dump("misheyakir", local.misheyakir());
		dump("yemamaStart", local.yemamaStart());
		dump("yemamaEnd", local.yemamaEnd());
		dump("dayStart", local.dayStart());
		dump("noon", local.noon());
		dump("mincha", local.mincha());
		dump("minchaKetana", local.minchaKetana());
		dump("plagMincha", local.plagMincha());
		dump("midnight", local.midnight());
		printf("dayLength %ld\n", local.dayLength());
		printf("nightLength %ld\n", local.nightLength());
		printf("hour_12 %.17g\n", local.hour(Time(12, 0, 0)));
		printf("hour_3 %.17g\n", local.hour(Time(3, 0, 0)));
		printf("angle_6 %.17g\n", local.angle(6.0));
		printf("angle_18 %.17g\n", local.angle(18.0));

		int evCount;
		const Event* ev = local.events(evCount);
		printf("events_count %d\n", evCount);
		for (int i = 0; i < evCount; i++) {
			printf("  ev %d %d %d %d %d\n", static_cast<int>(ev[i].zman),
			       ev[i].datetime.date.day, ev[i].datetime.time.hour,
			       ev[i].datetime.time.minute, ev[i].datetime.time.second);
		}

		Band hb[LOCAL_HEBREW_DATE_MAX_HALAKHIC_BANDS];
		int hbCount = local.halakhicBands(hb);
		printf("halakhic_count %d\n", hbCount);
		for (int i = 0; i < hbCount; i++) {
			printf("  hb %d %d %d %d %d %d\n", hb[i].id, hb[i].stripe,
			       hb[i].start.datetime.time.hour, hb[i].start.datetime.time.minute,
			       hb[i].end.datetime.time.hour, hb[i].end.datetime.time.minute);
		}

		Band sb[LOCAL_HEBREW_DATE_MAX_SUN_BANDS];
		int sbCount = local.sunBands(sb);
		printf("sun_count %d\n", sbCount);
		for (int i = 0; i < sbCount; i++) {
			printf("  sb %d\n", sb[i].id);
		}

		Band mb[LOCAL_HEBREW_DATE_MAX_MOON_BANDS];
		int mbCount = local.moonBands(mb);
		printf("moon_count %d\n", mbCount);
		for (int i = 0; i < mbCount; i++) {
			printf("  mb %d %d %d %d %d\n", mb[i].id,
			       mb[i].start.datetime.time.hour, mb[i].start.datetime.time.minute,
			       mb[i].end.datetime.time.hour, mb[i].end.datetime.time.minute);
		}

		Band ob[LOCAL_HEBREW_DATE_MAX_OBSERVANCE_BANDS];
		int obCount = local.observanceBands(ob);
		printf("observance_count %d\n", obCount);
		for (int i = 0; i < obCount; i++) {
			printf("  ob %d %d\n", ob[i].id, ob[i].stripe);
		}

		DateTime hoursArr[24];
		local.hours(hoursArr);
		printf("hours_count 24\n");
		printf("hours_4 %d %d\n", hoursArr[4].time.hour, hoursArr[4].time.minute);
		printf("hours_8 %d %d\n", hoursArr[8].time.hour, hoursArr[8].time.minute);

		LocalHebrewDate prev = local.previousYemama();
		printf("prev_hebrewdate %d %d %d\n", prev.hebrewDate.year, prev.hebrewDate.month, prev.hebrewDate.day);
		LocalHebrewDate next = local.nextYemama();
		printf("next_hebrewdate %d %d %d\n", next.hebrewDate.year, next.hebrewDate.month, next.hebrewDate.day);
	}

	return 0;
}
