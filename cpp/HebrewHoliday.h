/* HebrewHoliday.h
 * Ported from LIBS/HebrewHoliday.py
 */
#ifndef HEBREWHOLIDAY_H
#define HEBREWHOLIDAY_H

#include "Date.h"
#include "HebrewDate.h"

/* Python splits this id space across three IntEnums: HebrewHolidayId,
 * DiasporaHolidayId, and HebrewSecondYomTovId. Their numeric values never
 * collide, and HebrewHoliday::diaspora() freely returns values from any of
 * the three interchangeably compared against HebrewHolidayId and indexed
 * into one unified HolidayNames table (0-38) -- it's really one flat id
 * space split into three Python classes for organizational grouping, not a
 * meaningful type distinction. Merged into a single enum class here: same
 * values, same behavior, and avoids a 3-way polymorphic return type for
 * diaspora(). */
enum class HebrewHolidayId {
	NONE = 0,
	PESACH = 1,
	PESACH_II = 2,                       /* was DiasporaHolidayId.PESACH_II */
	PESACH_VII = 3,
	PESACH_VIII = 4,                     /* was HebrewSecondYomTovId.PESACH_VIII */
	YOM_HASHOAH = 5,
	YOM_HAZIKARON = 6,
	YOM_HAATZMAUT = 7,
	PESACH_SHENI = 8,
	LAG_BOMER = 9,
	YOM_YERUSHALAYIM = 10,
	SHAVUOT = 11,
	SHAVUOT_II = 12,                     /* was HebrewSecondYomTovId.SHAVUOT_II */
	FAST_OF_TAMMUZ = 13,
	FAST_OF_AV = 14,
	TU_BAV = 15,
	ROSH_HASHANA = 16,
	ROSH_HASHANA_II = 17,
	TZOM_GEDALIAH = 18,
	YOM_KIPPUR = 19,
	SUKKOT = 20,
	SUKKOT_II = 21,                      /* was DiasporaHolidayId.SUKKOT_II */
	HOSHANA_RABA = 22,
	SHEMINI_ATZERET = 23,
	SIMCHAT_TORAH = 24,
	SIMCHAT_TORAH_SHEMINI_ATZERETH = 25,
	CHANUKKA_I = 26,
	CHANUKKA_II = 27,
	CHANUKKA_III = 28,
	CHANUKKA_IV = 29,
	CHANUKKA_V = 30,
	CHANUKKA_VI = 31,
	CHANUKKA_VII = 32,
	CHANUKKA_VIII = 33,
	FAST_OF_TEVET = 34,
	TU_BSHEVAT = 35,
	FAST_OF_ESTHER = 36,
	PURIM = 37,
	SHUSHAN_PURIM = 38
};

#define HEBREW_HOLIDAY_MAX_PER_YEAR 40   /* safe upper bound on flagged days in a Hebrew year (~30 typical) */

class HebrewHoliday {
public:
	HebrewDate hebrewDate;
	HebrewHolidayId id;

	explicit HebrewHoliday(HebrewDate hebrewDate = HebrewDate());

	bool is_shabbat() const;
	bool is_rest_day() const;
	bool is_festive() const;
	bool is_fast() const;
	bool is_national() const;
	bool is_solemn() const;

	/* Optional[HebrewHoliday] in Python. Can't use the usual
	 * pointer-to-member-storage trick here (a HebrewHoliday can't contain a
	 * HebrewHoliday member -- that's a self-referential value type, illegal
	 * in C++ without a pointer/heap allocation). Falls back to bool return +
	 * reference out-param instead, purely as a structural necessity for
	 * this one case. */
	bool moed(HebrewHoliday& out) const;
	bool erev(HebrewHoliday& out) const;

	HebrewHolidayId diaspora() const;

	static void holidays(int year, HebrewDate out[HEBREW_HOLIDAY_MAX_PER_YEAR], int& count);
};

class HolidayNames {
public:
	static const char* NAMES_EN[39];
	static const char* NAMES_HE[39];

	Lang lang;
	HebrewHolidayId id;

	HolidayNames(Lang lang, HebrewHolidayId id);

	const char* name() const;
};

#endif
