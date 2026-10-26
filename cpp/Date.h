/* Date.h
 * Ported from LIBS/Date.py
 */
#ifndef DATE_H
#define DATE_H

enum class Weekday {
	SUNDAY = 1,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY
};

enum class GregorianMonth {
	JANUARY = 1,
	FEBRUARY,
	MARCH,
	APRIL,
	MAY,
	JUNE,
	JULY,
	AUGUST,
	SEPTEMBER,
	OCTOBER,
	NOVEMBER,
	DECEMBER
};

class Date {
public:
	static const long EPOCH = 1721425;

	int year;
	int month;   /* 1-12, plain int: participates in raw arithmetic */
	int day;
	long ordinal;

	Date(int year = 1, int month = 1, int day = 1);
	static Date fromordinal(long ordinal);

	Date operator+(long x) const;
	Date operator-(long x) const;
	bool operator==(const Date& other) const;
	bool operator!=(const Date& other) const;

	Date next() const;
	Date prev() const;
	Weekday dayofweek() const;

	static bool isleapyear(int year);
	static int numdaysinmonth(int month, int year);
};

class Time {
public:
	int hour;
	int minute;
	int second;
	long serial;

	Time(int hour = 0, int minute = 0, int second = 0);
	static Time fromserial(long serial);

	Time operator+(long x) const;
	bool operator==(const Time& other) const;
	bool operator!=(const Time& other) const;
};

class DateTimeSpan {
public:
	long days;
	long seconds;

	DateTimeSpan(long days, long seconds);
};

class DateTime {
public:
	Date date;
	Time time;

	DateTime();
	DateTime(Date date, Time time);

	double julianday() const;
	double julianDay2K() const;
	static DateTime fromjulianday(double jd);

	/* Ported exactly as-is from Date.py, including the cross-midnight
	 * sign-loss bug flagged in the Python source's __sub__. */
	DateTimeSpan operator-(const DateTime& other) const;
	DateTime operator+(long seconds) const;

	bool operator<(const DateTime& other) const;
	bool operator<=(const DateTime& other) const;
	bool operator>(const DateTime& other) const;
	bool operator>=(const DateTime& other) const;
	bool operator==(const DateTime& other) const;
	bool operator!=(const DateTime& other) const;
};

class MonthCalendar {
public:
	Date storage[6][7];
	Date* dates[6][7];   /* &storage[r][c] if populated, NULL if empty */

	MonthCalendar(int year, int month);
	MonthCalendar(const MonthCalendar& other);
	MonthCalendar& operator=(const MonthCalendar& other);
};

#define YEAR_CALENDAR_MAX_WEEKS 54   /* ceil(366/7) + 1 padding row, safe upper bound */

class YearCalendar {
public:
	int year;
	MonthCalendar monthCalendars[12];

	Date weekStorage[YEAR_CALENDAR_MAX_WEEKS][7];
	Date* weeksGrid[YEAR_CALENDAR_MAX_WEEKS][7];
	int weekCount;

	YearCalendar(int year);

private:
	void computeWeeks();
};

enum class Lang { EN, HE };

class WeekdayNames {
public:
	static const char* WEEKDAY_NAMES[2][8];
	static const char* SHORT_NAMES[2][8];

	Lang lang;
	Weekday dayofweek;

	WeekdayNames(Lang lang, Weekday dayofweek);

	const char* name() const;
	const char* shortname() const;
};

class MonthNames {
public:
	static const char* MONTH_NAMES[2][13];

	Lang lang;
	int month;

	MonthNames(Lang lang, int month);

	const char* name() const;
};

#endif
