/* HebrewDate.cpp
 * Ported from LIBS/HebrewDate.py
 */
#include "HebrewDate.h"

#include <stddef.h>

/* ---------- HebrewDate ---------- */

HebrewDate::HebrewDate(int year, int month, int day)
{
	this->year = year;
	this->month = month;
	this->day = day;

	long raw = day;
	if (month < static_cast<int>(HebrewMonth::TISHREI)) {
		for (int m = static_cast<int>(HebrewMonth::TISHREI); m <= HebrewDate::numMonthInYear(year); m++) {
			raw += HebrewDate::numdaysinmonth(year, m);
		}
		for (int m = static_cast<int>(HebrewMonth::NISAN); m < month; m++) {
			raw += HebrewDate::numdaysinmonth(year, m);
		}
	} else {
		for (int m = static_cast<int>(HebrewMonth::TISHREI); m < month; m++) {
			raw += HebrewDate::numdaysinmonth(year, m);
		}
	}
	raw += HebrewDate::elapsedDays(year);
	this->ordinal = raw + HebrewDate::EPOCH;
}

HebrewDate HebrewDate::fromordinal(long ordinal)
{
	long raw = ordinal - HebrewDate::EPOCH;

	int y = static_cast<int>(raw / 366);
	for (;;) {
		long tempRaw = HebrewDate(y + 1, static_cast<int>(HebrewMonth::TISHREI), 1).ordinal - HebrewDate::EPOCH;
		if (raw < tempRaw) {
			break;
		}
		y++;
	}

	long raw1Nisan = HebrewDate(y, static_cast<int>(HebrewMonth::NISAN), 1).ordinal - HebrewDate::EPOCH;
	int start = (raw < raw1Nisan) ? static_cast<int>(HebrewMonth::TISHREI) : static_cast<int>(HebrewMonth::NISAN);

	int m = start;
	for (;;) {
		long monthEnd = HebrewDate(y, m, HebrewDate::numdaysinmonth(y, m)).ordinal - HebrewDate::EPOCH;
		if (raw <= monthEnd) {
			break;
		}
		m++;
	}

	int day = static_cast<int>(raw - (HebrewDate(y, m, 1).ordinal - HebrewDate::EPOCH) + 1);

	return HebrewDate(y, m, day);
}

HebrewDate HebrewDate::fromdate(const Date& date)
{
	return HebrewDate::fromordinal(date.ordinal);
}

bool HebrewDate::operator==(const HebrewDate& other) const
{
	return ordinal == other.ordinal;
}

bool HebrewDate::operator!=(const HebrewDate& other) const
{
	return !(*this == other);
}

bool HebrewDate::operator<(const HebrewDate& other) const
{
	return ordinal < other.ordinal;
}

bool HebrewDate::operator<=(const HebrewDate& other) const
{
	return ordinal <= other.ordinal;
}

bool HebrewDate::operator>(const HebrewDate& other) const
{
	return ordinal > other.ordinal;
}

bool HebrewDate::operator>=(const HebrewDate& other) const
{
	return ordinal >= other.ordinal;
}

HebrewDate HebrewDate::next() const
{
	return HebrewDate::fromordinal(ordinal + 1);
}

HebrewDate HebrewDate::prev() const
{
	return HebrewDate::fromordinal(ordinal - 1);
}

bool HebrewDate::isleapyear(int year)
{
	return ((year * 7) + 1) % 19 < 7;
}

int HebrewDate::numMonthInYear(int year)
{
	return HebrewDate::isleapyear(year) ? static_cast<int>(HebrewMonth::ADAR_II) : static_cast<int>(HebrewMonth::ADAR);
}

long HebrewDate::elapsedDays(int year)
{
	long monthsElapsed = 235L * ((year - 1) / 19);
	monthsElapsed += 12L * ((year - 1) % 19);
	monthsElapsed += (((year - 1) % 19 * 7 + 1) / 19);

	long partsElapsed = ((monthsElapsed % 1080) * 793) + 204;
	long hoursElapsed = 5 + (monthsElapsed * 12) +
		((monthsElapsed / 1080) * 793) + (partsElapsed / 1080);

	long day = 1 + (29 * monthsElapsed) + (hoursElapsed / 24);
	long parts = ((hoursElapsed % 24) * 1080) + (partsElapsed % 1080);

	bool addDay = (parts >= 19440) ||
		(day % 7 == 2 && parts >= 9924 && !HebrewDate::isleapyear(year)) ||
		(day % 7 == 1 && parts >= 16789 && HebrewDate::isleapyear(year - 1));

	long altDay = addDay ? day + 1 : day;
	if (altDay % 7 == 0 || altDay % 7 == 3 || altDay % 7 == 5) {
		altDay += 1;
	}
	return altDay;
}

long HebrewDate::numDaysInYear(int year)
{
	return HebrewDate::elapsedDays(year + 1) - HebrewDate::elapsedDays(year);
}

bool HebrewDate::hasLongHeshvan(int year)
{
	return HebrewDate::numDaysInYear(year) % 10 == 5;
}

bool HebrewDate::hasShortKislev(int year)
{
	return HebrewDate::numDaysInYear(year) % 10 == 3;
}

int HebrewDate::numdaysinmonth(int year, int month)
{
	if (month == static_cast<int>(HebrewMonth::IYAR) ||
	    month == static_cast<int>(HebrewMonth::TAMMUZ) ||
	    month == static_cast<int>(HebrewMonth::ELUL) ||
	    month == static_cast<int>(HebrewMonth::TEVET) ||
	    month == static_cast<int>(HebrewMonth::ADAR_II)) {
		return 29;
	}
	if (month == static_cast<int>(HebrewMonth::ADAR) && !HebrewDate::isleapyear(year)) {
		return 29;
	}
	if (month == static_cast<int>(HebrewMonth::CHESHVAN) && !HebrewDate::hasLongHeshvan(year)) {
		return 29;
	}
	if (month == static_cast<int>(HebrewMonth::KISLEV) && HebrewDate::hasShortKislev(year)) {
		return 29;
	}
	return 30;
}

YearLength HebrewDate::yearLengthType(int year)
{
	if (HebrewDate::hasLongHeshvan(year) && !HebrewDate::hasShortKislev(year)) {
		return YearLength::COMPLETE;
	}
	if (!HebrewDate::hasLongHeshvan(year) && HebrewDate::hasShortKislev(year)) {
		return YearLength::DEFICIENT;
	}
	return YearLength::REGULAR;
}

YearType HebrewDate::yeartype(int year)
{
	Weekday rh = HebrewDate(year, static_cast<int>(HebrewMonth::TISHREI), 1).dayofweek();
	Weekday pesach = HebrewDate(year, static_cast<int>(HebrewMonth::NISAN), 15).dayofweek();
	YearLength length = HebrewDate::yearLengthType(year);

	if (!HebrewDate::isleapyear(year)) {
		if (rh == Weekday::MONDAY && length == YearLength::DEFICIENT && pesach == Weekday::TUESDAY) {
			return YearType::B_CH_G;
		}
		if (rh == Weekday::MONDAY && length == YearLength::COMPLETE && pesach == Weekday::THURSDAY) {
			return YearType::B_SH_H;
		}
		if (rh == Weekday::TUESDAY && length == YearLength::REGULAR && pesach == Weekday::THURSDAY) {
			return YearType::G_K_H;
		}
		if (rh == Weekday::THURSDAY && length == YearLength::REGULAR && pesach == Weekday::SATURDAY) {
			return YearType::H_K_Z;
		}
		if (rh == Weekday::THURSDAY && length == YearLength::COMPLETE && pesach == Weekday::SUNDAY) {
			return YearType::H_SH_A;
		}
		if (rh == Weekday::SATURDAY && length == YearLength::DEFICIENT && pesach == Weekday::SUNDAY) {
			return YearType::Z_CH_A;
		}
		if (rh == Weekday::SATURDAY && length == YearLength::COMPLETE && pesach == Weekday::TUESDAY) {
			return YearType::Z_SH_G;
		}
	} else {
		if (rh == Weekday::MONDAY && length == YearLength::DEFICIENT && pesach == Weekday::THURSDAY) {
			return YearType::B_CH_H;
		}
		if (rh == Weekday::MONDAY && length == YearLength::COMPLETE && pesach == Weekday::SATURDAY) {
			return YearType::B_SH_Z;
		}
		if (rh == Weekday::TUESDAY && length == YearLength::REGULAR && pesach == Weekday::SATURDAY) {
			return YearType::G_K_Z;
		}
		if (rh == Weekday::THURSDAY && length == YearLength::DEFICIENT && pesach == Weekday::SUNDAY) {
			return YearType::H_CH_A;
		}
		if (rh == Weekday::THURSDAY && length == YearLength::COMPLETE && pesach == Weekday::TUESDAY) {
			return YearType::H_SH_G;
		}
		if (rh == Weekday::SATURDAY && length == YearLength::DEFICIENT && pesach == Weekday::TUESDAY) {
			return YearType::Z_CH_G;
		}
		if (rh == Weekday::SATURDAY && length == YearLength::COMPLETE && pesach == Weekday::THURSDAY) {
			return YearType::Z_SH_H;
		}
	}
	return static_cast<YearType>(0);   /* fallthrough, mirrors Python's `return 0` */
}

int HebrewDate::biblical(int year, int civil_month)
{
	bool is_leap = HebrewDate::isleapyear(year);
	if (civil_month <= 6) {
		return civil_month + 6;
	} else if (civil_month == 7) {
		return is_leap ? 13 : 1;
	} else {
		return is_leap ? civil_month - 7 : civil_month - 6;
	}
}

Weekday HebrewDate::dayofweek() const
{
	long dow = (ordinal % 7 + 1) % 7 + 1;
	return static_cast<Weekday>(dow);
}

HebrewDate HebrewDate::shabbat() const
{
	return HebrewDate::fromordinal(ordinal + (7 - static_cast<int>(dayofweek())));
}

ShabbatType HebrewDate::shabbatType() const
{
	HebrewDate sh = shabbat();
	int y = sh.year;

	HebrewDate anchor(y, static_cast<int>(HebrewMonth::NISAN), 1);
	HebrewDate hahodeshShabbat = HebrewDate::fromordinal(anchor.ordinal - (static_cast<int>(anchor.dayofweek()) % 7));
	if (sh.ordinal == hahodeshShabbat.ordinal) {
		return ShabbatType::HAHODESH;
	}

	if (sh.ordinal == hahodeshShabbat.ordinal - 7) {
		return ShabbatType::PARAH;
	}

	int adarMonth = HebrewDate::numMonthInYear(y);
	anchor = HebrewDate(y, adarMonth, 13);
	HebrewDate zahorShabbat = HebrewDate::fromordinal(anchor.ordinal - (static_cast<int>(anchor.dayofweek()) % 7));
	if (sh.ordinal == zahorShabbat.ordinal) {
		return ShabbatType::ZAHOR;
	}

	anchor = HebrewDate(y, adarMonth, 1);
	HebrewDate shekalimShabbat = HebrewDate::fromordinal(anchor.ordinal - (static_cast<int>(anchor.dayofweek()) % 7));
	if (sh.ordinal == shekalimShabbat.ordinal) {
		return ShabbatType::SHEKALIM;
	}

	anchor = HebrewDate(y, static_cast<int>(HebrewMonth::NISAN), 14);
	HebrewDate haggadolShabbat = HebrewDate::fromordinal(anchor.ordinal - (static_cast<int>(anchor.dayofweek()) % 7));
	if (sh.ordinal == haggadolShabbat.ordinal) {
		return ShabbatType::HAGGADOL;
	}

	anchor = HebrewDate(y, static_cast<int>(HebrewMonth::TISHREI), 9);
	HebrewDate shuvahShabbat = HebrewDate::fromordinal(anchor.ordinal - (static_cast<int>(anchor.dayofweek()) % 7));
	if (sh.ordinal == shuvahShabbat.ordinal) {
		return ShabbatType::SHUVAH;
	}

	anchor = HebrewDate(y, static_cast<int>(HebrewMonth::AV), 9);
	HebrewDate chazonShabbat = HebrewDate::fromordinal(anchor.ordinal - (static_cast<int>(anchor.dayofweek()) % 7));
	if (sh.ordinal == chazonShabbat.ordinal) {
		return ShabbatType::CHAZON;
	}

	HebrewDate dayAfter = HebrewDate::fromordinal(anchor.ordinal + 1);
	HebrewDate nachamuShabbat = HebrewDate::fromordinal(dayAfter.ordinal + (7 - static_cast<int>(dayAfter.dayofweek())));
	if (sh.ordinal == nachamuShabbat.ordinal) {
		return ShabbatType::NACHAMU;
	}

	return ShabbatType::NONE;
}

ShabbatChodeshType HebrewDate::shabbatChodeshType() const
{
	HebrewDate sh = shabbat();

	if (sh.day == 1 && sh.month != static_cast<int>(HebrewMonth::TISHREI)) {
		return ShabbatChodeshType::ROSH_CHODESH;
	}

	HebrewDate nextDay = HebrewDate::fromordinal(sh.ordinal + 1);
	if (nextDay.day == 1 && nextDay.month != static_cast<int>(HebrewMonth::TISHREI)) {
		return ShabbatChodeshType::MACHAR_CHODESH;
	}

	return ShabbatChodeshType::NONE;
}

/* ---------- HebrewMonthCalendar ---------- */

HebrewMonthCalendar::HebrewMonthCalendar()
{
	for (int r = 0; r < 6; r++) {
		for (int c = 0; c < 7; c++) {
			dates[r][c] = NULL;
		}
	}
}

HebrewMonthCalendar::HebrewMonthCalendar(int year, int month)
{
	int daysInMonth = HebrewDate::numdaysinmonth(year, month);
	HebrewDate firstDate(year, month, 1);
	int startCol = static_cast<int>(firstDate.dayofweek()) - 1;

	int dayCounter = 1;
	for (int r = 0; r < 6; r++) {
		for (int c = 0; c < 7; c++) {
			if ((r == 0 && c < startCol) || dayCounter > daysInMonth) {
				dates[r][c] = NULL;
			} else {
				storage[r][c] = HebrewDate(year, month, dayCounter);
				dates[r][c] = &storage[r][c];
				dayCounter++;
			}
		}
	}
}

HebrewMonthCalendar::HebrewMonthCalendar(const HebrewMonthCalendar& other)
{
	for (int r = 0; r < 6; r++) {
		for (int c = 0; c < 7; c++) {
			storage[r][c] = other.storage[r][c];
			dates[r][c] = other.dates[r][c] ? &storage[r][c] : NULL;
		}
	}
}

HebrewMonthCalendar& HebrewMonthCalendar::operator=(const HebrewMonthCalendar& other)
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

/* ---------- HebrewYearCalendar ---------- */

HebrewYearCalendar::HebrewYearCalendar(int year)
	: year(year)
{
	int idx = 0;
	int lastMonth = HebrewDate::numMonthInYear(year);
	for (int m = static_cast<int>(HebrewMonth::TISHREI); m <= lastMonth; m++) {
		monthCalendars[idx] = HebrewMonthCalendar(year, m);
		idx++;
	}
	for (int m = static_cast<int>(HebrewMonth::NISAN); m < static_cast<int>(HebrewMonth::TISHREI); m++) {
		monthCalendars[idx] = HebrewMonthCalendar(year, m);
		idx++;
	}
	monthCalendarCount = idx;

	computeWeeks();
}

void HebrewYearCalendar::computeWeeks()
{
	HebrewDate start(year, static_cast<int>(HebrewMonth::TISHREI), 1);
	HebrewDate end = HebrewDate::fromordinal(start.ordinal + HebrewDate::numDaysInYear(year) - 1);

	HebrewDate cur = start;
	while (cur.dayofweek() != Weekday::SUNDAY) {
		cur = cur.prev();
	}
	HebrewDate first_sunday = cur;

	cur = end;
	while (cur.dayofweek() != Weekday::SATURDAY) {
		cur = cur.next();
	}
	HebrewDate last_saturday = cur;

	weekCount = 0;
	HebrewDate d = first_sunday;
	while (d.ordinal <= last_saturday.ordinal && weekCount < HEBREW_YEAR_CALENDAR_MAX_WEEKS) {
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

/* ---------- HebrewWeekdayDayNames ---------- */

const char* HebrewWeekdayDayNames::NAMES_EN[8] = {
	"", "First day", "Second day", "Third day", "Fourth day",
	"Fifth day", "Sixth day", "Shabbat"
};

const char* HebrewWeekdayDayNames::NAMES_HE[8] = {
	"", "\xd7\x99\xd7\x95\xd7\x9d \xd7\xa8\xd7\x90\xd7\xa9\xd7\x95\xd7\x9f",
	"\xd7\x99\xd7\x95\xd7\x9d \xd7\xa9\xd7\xa0\xd7\x99",
	"\xd7\x99\xd7\x95\xd7\x9d \xd7\xa9\xd7\x9c\xd7\x99\xd7\xa9\xd7\x99",
	"\xd7\x99\xd7\x95\xd7\x9d \xd7\xa8\xd7\x91\xd7\x99\xd7\xa2\xd7\x99",
	"\xd7\x99\xd7\x95\xd7\x9d \xd7\x97\xd7\x9e\xd7\x99\xd7\xa9\xd7\x99",
	"\xd7\x99\xd7\x95\xd7\x9d \xd7\xa9\xd7\x99\xd7\xa9\xd7\x99",
	"\xd7\x99\xd7\x95\xd7\x9d \xd7\xa9\xd7\x91\xd7\xaa"
};

HebrewWeekdayDayNames::HebrewWeekdayDayNames(Lang lang, Weekday dayofweek)
{
	this->lang = lang;
	this->dayofweek = dayofweek;
}

const char* HebrewWeekdayDayNames::name() const
{
	return (lang == Lang::EN) ? NAMES_EN[static_cast<int>(dayofweek)] : NAMES_HE[static_cast<int>(dayofweek)];
}

/* ---------- HebrewWeekdayNightNames ---------- */

const char* HebrewWeekdayNightNames::NAMES_EN[8] = {
	"", "Eve of first day", "Eve of second day", "Eve of third day",
	"Eve of fourth day", "Eve of fifth day", "Eve of sixth day", "Eve of Shabbat"
};

const char* HebrewWeekdayNightNames::NAMES_HE[8] = {
	"", "\xd7\x9c\xd7\x99\xd7\x9c \xd7\xa8\xd7\x90\xd7\xa9\xd7\x95\xd7\x9f",
	"\xd7\x9c\xd7\x99\xd7\x9c \xd7\xa9\xd7\xa0\xd7\x99",
	"\xd7\x9c\xd7\x99\xd7\x9c \xd7\xa9\xd7\x9c\xd7\x99\xd7\xa9\xd7\x99",
	"\xd7\x9c\xd7\x99\xd7\x9c \xd7\xa8\xd7\x91\xd7\x99\xd7\xa2\xd7\x99",
	"\xd7\x9c\xd7\x99\xd7\x9c \xd7\x97\xd7\x9e\xd7\x99\xd7\xa9\xd7\x99",
	"\xd7\x9c\xd7\x99\xd7\x9c \xd7\xa9\xd7\x99\xd7\xa9\xd7\x99",
	"\xd7\x9c\xd7\x99\xd7\x9c \xd7\xa9\xd7\x91\xd7\xaa"
};

HebrewWeekdayNightNames::HebrewWeekdayNightNames(Lang lang, Weekday dayofweek)
{
	this->lang = lang;
	this->dayofweek = dayofweek;
}

const char* HebrewWeekdayNightNames::name() const
{
	return (lang == Lang::EN) ? NAMES_EN[static_cast<int>(dayofweek)] : NAMES_HE[static_cast<int>(dayofweek)];
}

/* ---------- HebrewMonthNames ---------- */

const char* HebrewMonthNames::NAMES_EN[14] = {
	"", "Nisan", "Iyar", "Sivan", "Tammuz", "Av", "Elul",
	"Tishrei", "Cheshvan", "Kislev", "Tevet", "Shevat", "Adar", "Adar II"
};

const char* HebrewMonthNames::NAMES_HE[14] = {
	"", "\xd7\xa0\xd7\x99\xd7\xa1\xd7\x9f", "\xd7\x90\xd7\x99\xd7\x99\xd7\xa8",
	"\xd7\xa1\xd7\x99\xd7\x95\xd7\x95\xd7\x9f", "\xd7\xaa\xd7\x9e\xd7\x95\xd7\x96",
	"\xd7\x90\xd7\x91", "\xd7\x90\xd7\x9c\xd7\x95\xd7\x9c",
	"\xd7\xaa\xd7\xa9\xd7\xa8\xd7\x99", "\xd7\x97\xd7\xa9\xd7\x95\xd7\x9f",
	"\xd7\x9b\xd7\xa1\xd7\x9c\xd7\x95", "\xd7\x98\xd7\x91\xd7\xaa",
	"\xd7\xa9\xd7\x91\xd7\x98", "\xd7\x90\xd7\x93\xd7\xa8",
	"\xd7\x90\xd7\x93\xd7\xa8 \xd7\x91\xd7\xb3"
};

HebrewMonthNames::HebrewMonthNames(Lang lang, int year, int month)
{
	this->lang = lang;
	this->year = year;
	this->month = month;
}

const char* HebrewMonthNames::name() const
{
	if (lang == Lang::EN) {
		if (month == static_cast<int>(HebrewMonth::ADAR) && HebrewDate::isleapyear(year)) {
			return "Adar I";
		}
		return NAMES_EN[month];
	}
	if (month == static_cast<int>(HebrewMonth::ADAR) && HebrewDate::isleapyear(year)) {
		return "\xd7\x90\xd7\x93\xd7\xa8 \xd7\x90\xd7\xb3";
	}
	return NAMES_HE[month];
}

/* ---------- ShabbatTypeNames ---------- */

const char* ShabbatTypeNames::NAMES_EN[9] = {
	"", "Shekalim", "Zachor", "Parah", "HaChodesh", "HaGadol",
	"Shuvah", "Chazon", "Nachamu"
};

const char* ShabbatTypeNames::NAMES_HE[9] = {
	"", "\xd7\xa9\xd7\xa7\xd7\x9c\xd7\x99\xd7\x9d", "\xd7\x96\xd7\x9b\xd7\x95\xd7\xa8",
	"\xd7\xa4\xd7\xa8\xd7\x94", "\xd7\x94\xd7\x97\xd7\x95\xd7\x93\xd7\xa9",
	"\xd7\x94\xd7\x92\xd7\x93\xd7\x95\xd7\x9c", "\xd7\xa9\xd7\x95\xd7\x91\xd7\x94",
	"\xd7\x97\xd7\x96\xd7\x95\xd7\x9f", "\xd7\xa0\xd7\x97\xd7\x9e\xd7\x95"
};

ShabbatTypeNames::ShabbatTypeNames(Lang lang, ShabbatType shabbatType)
{
	this->lang = lang;
	this->shabbatType = shabbatType;
}

const char* ShabbatTypeNames::name() const
{
	return (lang == Lang::EN) ? NAMES_EN[static_cast<int>(shabbatType)] : NAMES_HE[static_cast<int>(shabbatType)];
}

/* ---------- ShabbatChodeshTypeNames ---------- */

const char* ShabbatChodeshTypeNames::NAMES_EN[3] = { "", "Machar Chodesh", "Rosh Chodesh" };

const char* ShabbatChodeshTypeNames::NAMES_HE[3] = {
	"", "\xd7\x9e\xd7\x97\xd7\xa8 \xd7\x97\xd7\x95\xd7\x93\xd7\xa9",
	"\xd7\xa8\xd7\x90\xd7\xa9 \xd7\x97\xd7\x95\xd7\x93\xd7\xa9"
};

ShabbatChodeshTypeNames::ShabbatChodeshTypeNames(Lang lang, ShabbatChodeshType shabbatChodeshType)
{
	this->lang = lang;
	this->shabbatChodeshType = shabbatChodeshType;
}

const char* ShabbatChodeshTypeNames::name() const
{
	return (lang == Lang::EN) ? NAMES_EN[static_cast<int>(shabbatChodeshType)] : NAMES_HE[static_cast<int>(shabbatChodeshType)];
}
