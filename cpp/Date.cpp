/* Date.cpp
 * Ported from LIBS/Date.py
 */
#include "Date.h"
#include "Astro.h"

#include <math.h>
#include <stddef.h>

/* ---------- Date ---------- */

Date::Date(int year, int month, int day)
{
	this->year = year;
	this->month = month;
	this->day = day;

	long serial = day;
	for (int m = 1; m < month; m++) {
		serial += Date::numdaysinmonth(m, year);
	}
	int y = year - 1;
	serial += 365L * y;
	serial += y / 4;
	serial -= y / 100;
	serial += y / 400;
	this->ordinal = serial + Date::EPOCH;
}

Date Date::fromordinal(long ordinal)
{
	long serial = ordinal - Date::EPOCH;

	int y = 1;
	for (;;) {
		int daysInYear = Date::isleapyear(y) ? 366 : 365;
		if (serial <= daysInYear) {
			break;
		}
		serial -= daysInYear;
		y++;
	}

	int m = 1;
	for (;;) {
		int daysInMonth = Date::numdaysinmonth(m, y);
		if (serial <= daysInMonth) {
			break;
		}
		serial -= daysInMonth;
		m++;
	}

	return Date(y, m, static_cast<int>(serial));
}

Date Date::operator+(long x) const
{
	return Date::fromordinal(ordinal + x);
}

Date Date::operator-(long x) const
{
	return Date::fromordinal(ordinal - x);
}

bool Date::operator==(const Date& other) const
{
	return ordinal == other.ordinal;
}

bool Date::operator!=(const Date& other) const
{
	return !(*this == other);
}

Date Date::next() const
{
	return *this + 1;
}

Date Date::prev() const
{
	return *this - 1;
}

bool Date::isleapyear(int year)
{
	if (year % 4 != 0) {
		return false;
	}
	if (year % 100 != 0) {
		return true;
	}
	return year % 400 == 0;
}

int Date::numdaysinmonth(int month, int year)
{
	if (month == static_cast<int>(GregorianMonth::FEBRUARY)) {
		return Date::isleapyear(year) ? 29 : 28;
	}
	if (month == static_cast<int>(GregorianMonth::APRIL) ||
	    month == static_cast<int>(GregorianMonth::JUNE) ||
	    month == static_cast<int>(GregorianMonth::SEPTEMBER) ||
	    month == static_cast<int>(GregorianMonth::NOVEMBER)) {
		return 30;
	}
	return 31;
}

Weekday Date::dayofweek() const
{
	long dow = (ordinal % 7 + 1) % 7 + 1;
	return static_cast<Weekday>(dow);
}

/* ---------- Time ---------- */

Time::Time(int hour, int minute, int second)
{
	this->hour = hour;
	this->minute = minute;
	this->second = second;
	this->serial = static_cast<long>(hour) * 3600 + minute * 60 + second;
}

Time Time::fromserial(long serial)
{
	long hour = serial / 3600;
	long remainder = serial % 3600;
	long minute = remainder / 60;
	long second = remainder % 60;
	return Time(static_cast<int>(hour), static_cast<int>(minute), static_cast<int>(second));
}

Time Time::operator+(long x) const
{
	/* Python's % always returns a non-negative result; C++'s % can
	 * return negative for a negative left-hand operand, so replicate
	 * floor-mod explicitly (callers do pass negative x, e.g. -TIMEZONE). */
	long s = (serial + x) % 86400;
	if (s < 0) {
		s += 86400;
	}
	return Time::fromserial(s);
}

bool Time::operator==(const Time& other) const
{
	return serial == other.serial;
}

bool Time::operator!=(const Time& other) const
{
	return !(*this == other);
}

/* ---------- DateTimeSpan ---------- */

DateTimeSpan::DateTimeSpan(long days, long seconds)
{
	this->days = days;
	this->seconds = seconds;
}

/* ---------- DateTime ---------- */

DateTime::DateTime()
	: date(Date()), time(Time())
{
}

DateTime::DateTime(Date date, Time time)
	: date(date), time(time)
{
}

double DateTime::julianday() const
{
	return static_cast<double>(date.ordinal) - 0.5 + (static_cast<double>(time.serial) / 86400.0);
}

double DateTime::julianDay2K() const
{
	return julianday() - ASTRO_J2000;
}

DateTime DateTime::fromjulianday(double jd)
{
	long totalSeconds = static_cast<long>(round((jd + 0.5) * 86400.0));
	long ordinal = totalSeconds / 86400;
	long serial = totalSeconds % 86400;
	if (serial < 0) {
		serial += 86400;
		ordinal -= 1;
	}
	return DateTime(Date::fromordinal(ordinal), Time::fromserial(serial));
}

DateTimeSpan DateTime::operator-(const DateTime& other) const
{
	long days = date.ordinal - other.date.ordinal;
	long secs;
	if (days == 0) {
		secs = time.serial - other.time.serial;
	} else {
		secs = (time + (-other.time.serial)).serial;   /* <-- BUG, ported as-is from Date.py */
	}
	return DateTimeSpan(days, secs);
}

DateTime DateTime::operator+(long seconds) const
{
	Time new_time = time + seconds;
	if (seconds >= 0 && new_time.serial < time.serial) {
		return DateTime(date + 1, new_time);
	} else if (seconds < 0 && new_time.serial > time.serial) {
		return DateTime(date - 1, new_time);
	}
	return DateTime(date, new_time);
}

bool DateTime::operator<(const DateTime& other) const
{
	if (date.ordinal != other.date.ordinal) {
		return date.ordinal < other.date.ordinal;
	}
	return time.serial < other.time.serial;
}

bool DateTime::operator<=(const DateTime& other) const
{
	if (date.ordinal != other.date.ordinal) {
		return date.ordinal < other.date.ordinal;
	}
	return time.serial <= other.time.serial;
}

bool DateTime::operator>(const DateTime& other) const
{
	return other < *this;
}

bool DateTime::operator>=(const DateTime& other) const
{
	return other <= *this;
}

bool DateTime::operator==(const DateTime& other) const
{
	return date == other.date && time == other.time;
}

bool DateTime::operator!=(const DateTime& other) const
{
	return !(*this == other);
}

/* ---------- MonthCalendar ---------- */

MonthCalendar::MonthCalendar(int year, int month)
{
	int daysInMonth = Date::numdaysinmonth(month, year);
	Date firstDate(year, month, 1);
	int startCol = static_cast<int>(firstDate.dayofweek()) - 1;

	int dayCounter = 1;
	for (int r = 0; r < 6; r++) {
		for (int c = 0; c < 7; c++) {
			if ((r == 0 && c < startCol) || dayCounter > daysInMonth) {
				dates[r][c] = NULL;
			} else {
				storage[r][c] = Date(year, month, dayCounter);
				dates[r][c] = &storage[r][c];
				dayCounter++;
			}
		}
	}
}

MonthCalendar::MonthCalendar(const MonthCalendar& other)
{
	for (int r = 0; r < 6; r++) {
		for (int c = 0; c < 7; c++) {
			storage[r][c] = other.storage[r][c];
			dates[r][c] = other.dates[r][c] ? &storage[r][c] : NULL;
		}
	}
}

MonthCalendar& MonthCalendar::operator=(const MonthCalendar& other)
{
	if (this != &other) {
		for (int r = 0; r < 6; r++) {
			for (int c = 0; c < 7; c++) {
				storage[r][c] = other.storage[r][c];
				dates[r][c] = other.dates[r][c] ? &storage[r][c] : NULL;
			}
		}
	}
	return *this;
}

/* ---------- YearCalendar ---------- */

YearCalendar::YearCalendar(int year)
	: year(year),
	  monthCalendars{
	      MonthCalendar(year, 1), MonthCalendar(year, 2), MonthCalendar(year, 3),
	      MonthCalendar(year, 4), MonthCalendar(year, 5), MonthCalendar(year, 6),
	      MonthCalendar(year, 7), MonthCalendar(year, 8), MonthCalendar(year, 9),
	      MonthCalendar(year, 10), MonthCalendar(year, 11), MonthCalendar(year, 12)
	  }
{
	computeWeeks();
}

void YearCalendar::computeWeeks()
{
	Date start(year, 1, 1);
	Date end(year, 12, 31);

	Date cur = start;
	while (cur.dayofweek() != Weekday::SUNDAY) {
		cur = cur.prev();
	}
	Date first_sunday = cur;

	cur = end;
	while (cur.dayofweek() != Weekday::SATURDAY) {
		cur = cur.next();
	}
	Date last_saturday = cur;

	weekCount = 0;
	Date d = first_sunday;
	while (d.ordinal <= last_saturday.ordinal && weekCount < YEAR_CALENDAR_MAX_WEEKS) {
		for (int c = 0; c < 7; c++) {
			if (d.ordinal < start.ordinal || d.ordinal > end.ordinal) {
				weeksGrid[weekCount][c] = NULL;
			} else {
				weekStorage[weekCount][c] = d;
				weeksGrid[weekCount][c] = &weekStorage[weekCount][c];
			}
			d = d.next();
		}
		weekCount++;
	}
}

/* ---------- WeekdayNames ---------- */

const char* WeekdayNames::WEEKDAY_NAMES[2][8] = {
	/* LANG_EN */ { "", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" },
	/* LANG_HE */ { "", "\xd7\xa8\xd7\x90\xd7\xa9\xd7\x95\xd7\x9f", "\xd7\xa9\xd7\xa0\xd7\x99",
	                "\xd7\xa9\xd7\x9c\xd7\x99\xd7\xa9\xd7\x99", "\xd7\xa8\xd7\x91\xd7\x99\xd7\xa2\xd7\x99",
	                "\xd7\x97\xd7\x9e\xd7\x99\xd7\xa9\xd7\x99", "\xd7\xa9\xd7\x99\xd7\xa9\xd7\x99",
	                "\xd7\xa9\xd7\x91\xd7\xaa" }
};

const char* WeekdayNames::SHORT_NAMES[2][8] = {
	/* LANG_EN */ { "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" },
	/* LANG_HE */ { "", "\xd7\x90", "\xd7\x91", "\xd7\x92", "\xd7\x93", "\xd7\x94", "\xd7\x95", "\xd7\xa9" }
};

WeekdayNames::WeekdayNames(Lang lang, Weekday dayofweek)
{
	this->lang = lang;
	this->dayofweek = dayofweek;
}

const char* WeekdayNames::name() const
{
	return WEEKDAY_NAMES[static_cast<int>(lang)][static_cast<int>(dayofweek)];
}

const char* WeekdayNames::shortname() const
{
	return SHORT_NAMES[static_cast<int>(lang)][static_cast<int>(dayofweek)];
}

/* ---------- MonthNames ---------- */

const char* MonthNames::MONTH_NAMES[2][13] = {
	/* LANG_EN */ { "", "January", "February", "March", "April", "May", "June",
	                "July", "August", "September", "October", "November", "December" },
	/* LANG_HE */ { "", "\xd7\x99\xd7\xa0\xd7\x95\xd7\x90\xd7\xa8", "\xd7\xa4\xd7\x91\xd7\xa8\xd7\x95\xd7\x90\xd7\xa8",
	                "\xd7\x9e\xd7\xa8\xd7\xa5", "\xd7\x90\xd7\xa4\xd7\xa8\xd7\x99\xd7\x9c",
	                "\xd7\x9e\xd7\x90\xd7\x99", "\xd7\x99\xd7\x95\xd7\xa0\xd7\x99",
	                "\xd7\x99\xd7\x95\xd7\x9c\xd7\x99", "\xd7\x90\xd7\x95\xd7\x92\xd7\x95\xd7\xa1\xd7\x98",
	                "\xd7\xa1\xd7\xa4\xd7\x98\xd7\x9e\xd7\x91\xd7\xa8", "\xd7\x90\xd7\x95\xd7\xa7\xd7\x98\xd7\x95\xd7\x91\xd7\xa8",
	                "\xd7\xa0\xd7\x95\xd7\x91\xd7\x9e\xd7\x91\xd7\xa8", "\xd7\x93\xd7\xa6\xd7\x9e\xd7\x91\xd7\xa8" }
};

MonthNames::MonthNames(Lang lang, int month)
{
	this->lang = lang;
	this->month = month;
}

const char* MonthNames::name() const
{
	return MONTH_NAMES[static_cast<int>(lang)][month];
}
