/* HebrewDate.h
 * Ported from LIBS/HebrewDate.py
 */
#ifndef HEBREWDATE_H
#define HEBREWDATE_H

#include "Date.h"

enum class HebrewMonth {
	NISAN = 1,
	IYAR,
	SIVAN,
	TAMMUZ,
	AV,
	ELUL,
	TISHREI,
	CHESHVAN,
	KISLEV,
	TEVET,
	SHEVAT,
	ADAR,
	ADAR_II
};

enum class ShabbatType {
	NONE = 0,
	SHEKALIM,
	ZAHOR,
	PARAH,
	HAHODESH,
	HAGGADOL,
	SHUVAH,
	CHAZON,
	NACHAMU
};

enum class ShabbatChodeshType {
	NONE = 0,
	MACHAR_CHODESH,
	ROSH_CHODESH
};

enum class YearLength {
	DEFICIENT = 1,
	REGULAR,
	COMPLETE
};

enum class YearType {
	B_CH_G = 1,
	B_SH_H,
	G_K_H,
	H_K_Z,
	H_SH_A,
	Z_CH_A,
	Z_SH_G,
	B_CH_H,
	B_SH_Z,
	G_K_Z,
	H_CH_A,
	H_SH_G,
	Z_CH_G,
	Z_SH_H
};

class HebrewDate {
public:
	static const long EPOCH = 347996;

	int year;
	int month;   /* 1-13, plain int: participates in raw arithmetic */
	int day;
	long ordinal;

	HebrewDate(int year = 1, int month = static_cast<int>(HebrewMonth::TISHREI), int day = 1);
	static HebrewDate fromordinal(long ordinal);
	static HebrewDate fromdate(const Date& date);

	bool operator==(const HebrewDate& other) const;
	bool operator!=(const HebrewDate& other) const;
	bool operator<(const HebrewDate& other) const;
	bool operator<=(const HebrewDate& other) const;
	bool operator>(const HebrewDate& other) const;
	bool operator>=(const HebrewDate& other) const;

	HebrewDate next() const;
	HebrewDate prev() const;

	Weekday dayofweek() const;
	HebrewDate shabbat() const;
	ShabbatType shabbatType() const;
	ShabbatChodeshType shabbatChodeshType() const;

	static bool isleapyear(int year);
	static int numMonthInYear(int year);
	static long elapsedDays(int year);
	static long numDaysInYear(int year);
	static bool hasLongHeshvan(int year);
	static bool hasShortKislev(int year);
	static int numdaysinmonth(int year, int month);
	static YearLength yearLengthType(int year);
	static YearType yeartype(int year);
	static int biblical(int year, int civil_month);
};

class HebrewMonthCalendar {
public:
	HebrewDate storage[6][7];
	HebrewDate* dates[6][7];   /* &storage[r][c] if populated, NULL if empty */

	HebrewMonthCalendar();   /* all cells NULL; used only as array-member placeholder before assignment */
	HebrewMonthCalendar(int year, int month);
	HebrewMonthCalendar(const HebrewMonthCalendar& other);
	HebrewMonthCalendar& operator=(const HebrewMonthCalendar& other);
};

#define HEBREW_YEAR_CALENDAR_MAX_MONTHS 13
#define HEBREW_YEAR_CALENDAR_MAX_WEEKS 57   /* up to 13 months * ~30 days, safe upper bound */

class HebrewYearCalendar {
public:
	int year;
	HebrewMonthCalendar monthCalendars[HEBREW_YEAR_CALENDAR_MAX_MONTHS];
	int monthCalendarCount;   /* 12 or 13; only the first monthCalendarCount entries are meaningful */

	HebrewDate weekStorage[HEBREW_YEAR_CALENDAR_MAX_WEEKS][7];
	HebrewDate* weeksGrid[HEBREW_YEAR_CALENDAR_MAX_WEEKS][7];
	int weekCount;

	HebrewYearCalendar(int year);

private:
	void computeWeeks();
};

class HebrewWeekdayDayNames {
public:
	static const char* NAMES_EN[8];
	static const char* NAMES_HE[8];

	Lang lang;
	Weekday dayofweek;

	HebrewWeekdayDayNames(Lang lang, Weekday dayofweek);

	const char* name() const;
};

class HebrewWeekdayNightNames {
public:
	static const char* NAMES_EN[8];
	static const char* NAMES_HE[8];

	Lang lang;
	Weekday dayofweek;

	HebrewWeekdayNightNames(Lang lang, Weekday dayofweek);

	const char* name() const;
};

class HebrewMonthNames {
public:
	static const char* NAMES_EN[14];
	static const char* NAMES_HE[14];

	Lang lang;
	int year;
	int month;

	HebrewMonthNames(Lang lang, int year, int month);

	const char* name() const;
};

class ShabbatTypeNames {
public:
	static const char* NAMES_EN[9];
	static const char* NAMES_HE[9];

	Lang lang;
	ShabbatType shabbatType;

	ShabbatTypeNames(Lang lang, ShabbatType shabbatType);

	const char* name() const;
};

class ShabbatChodeshTypeNames {
public:
	static const char* NAMES_EN[3];
	static const char* NAMES_HE[3];

	Lang lang;
	ShabbatChodeshType shabbatChodeshType;

	ShabbatChodeshTypeNames(Lang lang, ShabbatChodeshType shabbatChodeshType);

	const char* name() const;
};

#endif
