/* LocalHebrewDate.cpp
 * Ported from LIBS/LocalHebrewDate.py
 */
#include "LocalHebrewDate.h"
#include "HebrewHoliday.h"

#include <math.h>

/* ---------- LocalHebrewDateTime ---------- */

LocalHebrewDateTime::LocalHebrewDateTime()
{
	localHebrewDate = NULL;
	hour = 0.0;
}

LocalHebrewDateTime::LocalHebrewDateTime(const LocalHebrewDate& localHebrewDate, double hour)
{
	this->localHebrewDate = &localHebrewDate;
	this->hour = hour;
}

/* ---------- Event ---------- */

Event::Event()
	: datetime(Date(), Time()), zman(Zman::NONE)
{
}

Event::Event(DateTime datetime, Zman zman)
	: datetime(datetime), zman(zman)
{
}

/* ---------- Band ---------- */

Band::Band()
	: id(static_cast<int>(BandId::NONE)), stripe(false)
{
}

Band::Band(Event start, Event end, int id, bool stripe)
	: start(start), end(end), id(id), stripe(stripe)
{
}

/* ---------- LocalHebrewDate ---------- */

LocalHebrewDate::LocalHebrewDate(HebrewDate hebrewDate, Location location, Custom custom)
	: hebrewDate(hebrewDate), location(location), custom(custom),
	  sun(location, DateTime(Date::fromordinal(hebrewDate.ordinal), Time(0, 0, 0))),
	  sunYesterday(location, DateTime(Date::fromordinal(hebrewDate.ordinal - 1), Time(0, 0, 0))),
	  eventsComputed(false), eventsCount(0)
{
}

DateTime LocalHebrewDate::sunrise() const
{
	return sun.rise();
}

DateTime LocalHebrewDate::sunset() const
{
	return sun.set();
}

DateTime LocalHebrewDate::dawn() const
{
	DateTime sr = sun.rise();
	long astroDaySec = (sunset() - sr).seconds;
	DawnCustom c = custom.dawn;
	if (c == DawnCustom::DEG_16_1) {
		return sun.dawnTime(16.1);
	}
	if (c == DawnCustom::DEG_16_9) {
		return sun.dawnTime(16.9);
	}
	if (c == DawnCustom::DEG_18) {
		return sun.dawnTime(18.0);
	}
	if (c == DawnCustom::DEG_19_8) {
		return sun.dawnTime(19.8);
	}
	if (c == DawnCustom::MIN_72) {
		return sr + (-72L * 60);
	}
	if (c == DawnCustom::REL_MIN_72) {
		return sr + static_cast<long>(-astroDaySec / 10.0);
	}
	return sr;
}

DateTime LocalHebrewDate::_nightfallFor(const Sun& sunRef) const
{
	DateTime sunsetRef = sunRef.set();
	double refDayLength = 0.0;
	if (custom.nightfall == NightfallCustom::REL_MIN_72 || custom.nightfall == NightfallCustom::REL_MIN_90) {
		refDayLength = static_cast<double>((sunsetRef - sunRef.rise()).seconds);
	}

	NightfallCustom c = custom.nightfall;
	if (c == NightfallCustom::DEG_5_95) {
		return sunRef.duskTime(5.95);
	}
	if (c == NightfallCustom::DEG_6) {
		return sunRef.duskTime(6.0);
	}
	if (c == NightfallCustom::DEG_6_45) {
		return sunRef.duskTime(6.45);
	}
	if (c == NightfallCustom::DEG_7_1) {
		return sunRef.duskTime(7.1);
	}
	if (c == NightfallCustom::DEG_8) {
		return sunRef.duskTime(8.0);
	}
	if (c == NightfallCustom::DEG_8_5) {
		return sunRef.duskTime(8.5);
	}
	if (c == NightfallCustom::MIN_72) {
		return sunsetRef + (72L * 60);
	}
	if (c == NightfallCustom::REL_MIN_72) {
		return sunsetRef + static_cast<long>(refDayLength / 10.0);
	}
	if (c == NightfallCustom::REL_MIN_90) {
		return sunsetRef + static_cast<long>(refDayLength / 8.0);
	}
	if (c == NightfallCustom::DEG_16_1) {
		return sunRef.duskTime(16.1);
	}
	if (c == NightfallCustom::DEG_18) {
		return sunRef.duskTime(18.0);
	}
	if (c == NightfallCustom::DEG_19_8) {
		return sunRef.duskTime(19.8);
	}
	return sunsetRef;
}

DateTime LocalHebrewDate::nightfall() const
{
	if (custom.dayDefinition == DayDefinition::GRA) {
		return _nightfallFor(sunYesterday);
	}
	return _nightfallFor(sun);
}

DateTime LocalHebrewDate::misheyakir() const
{
	DateTime sr = sun.rise();
	MisheyakirCustom c = custom.misheyakir;
	if (c == MisheyakirCustom::DEG_11_5) {
		return sun.dawnTime(11.5);
	}
	if (c == MisheyakirCustom::DEG_11) {
		return sun.dawnTime(11.0);
	}
	if (c == MisheyakirCustom::DEG_10_2) {
		return sun.dawnTime(10.2);
	}
	if (c == MisheyakirCustom::MIN_50) {
		return sr + (-50L * 60);
	}
	if (c == MisheyakirCustom::DEG_7_65) {
		return sun.dawnTime(7.65);
	}
	return sr;
}

DateTime LocalHebrewDate::yemamaStart() const
{
	if (custom.dayDefinition == DayDefinition::GRA) {
		return sunYesterday.set();
	}
	return _nightfallFor(sunYesterday);
}

DateTime LocalHebrewDate::dayStart() const
{
	return (custom.dayDefinition == DayDefinition::GRA) ? sunrise() : dawn();
}

DateTime LocalHebrewDate::yemamaEnd() const
{
	return (custom.dayDefinition == DayDefinition::GRA) ? sunset() : nightfall();
}

DateTime LocalHebrewDate::shemaEnd() const
{
	return dayStart() + static_cast<long>((3.0 / 12.0) * dayLength());
}

DateTime LocalHebrewDate::amidahEnd() const
{
	return dayStart() + static_cast<long>((4.0 / 12.0) * dayLength());
}

DateTime LocalHebrewDate::noon() const
{
	return dayStart() + static_cast<long>((6.0 / 12.0) * dayLength());
}

DateTime LocalHebrewDate::mincha() const
{
	return dayStart() + static_cast<long>((6.5 / 12.0) * dayLength());
}

DateTime LocalHebrewDate::minchaKetana() const
{
	return dayStart() + static_cast<long>((9.5 / 12.0) * dayLength());
}

DateTime LocalHebrewDate::plagMincha() const
{
	return dayStart() + static_cast<long>((10.75 / 12.0) * dayLength());
}

DateTime LocalHebrewDate::midnight() const
{
	return yemamaStart() + static_cast<long>(nightLength() / 2.0);
}

long LocalHebrewDate::dayLength() const
{
	return (yemamaEnd() - dayStart()).seconds;
}

long LocalHebrewDate::nightLength() const
{
	return (dayStart() - yemamaStart()).seconds;
}

double LocalHebrewDate::hour(const Time& time) const
{
	DateTime now(Date::fromordinal(hebrewDate.ordinal), time);
	long nowSec = (now - yemamaStart()).seconds;
	if (nowSec < 0) {
		nowSec = 0;
	}

	long nightLen = nightLength();
	long dayLen = dayLength();

	if (nowSec < nightLen) {
		return 12.0 * (static_cast<double>(nowSec) / nightLen);
	} else if (nowSec <= nightLen + dayLen) {
		long dayElapsed = nowSec - nightLen;
		return 12.0 + 12.0 * (static_cast<double>(dayElapsed) / dayLen);
	} else {
		LocalHebrewDate next_ = nextYemama();
		long nextNightLen = next_.nightLength();
		long afterEnd = nowSec - (nightLen + dayLen);
		return 12.0 * (static_cast<double>(afterEnd) / nextNightLen);
	}
}

double LocalHebrewDate::angle(double hourVal) const
{
	long total = dayLength() + nightLength();
	double dayArc = (static_cast<double>(dayLength()) / total) * 360.0;
	double nightArc = 360.0 - dayArc;

	double result;
	if (hourVal < 12.0) {
		double frac = hourVal / 12.0;
		result = dayArc / 2.0 + frac * nightArc;
	} else {
		double frac = (hourVal - 12.0) / 12.0;
		double startAngle = fmod(-dayArc / 2.0, 360.0);
		if (startAngle < 0.0) {
			startAngle += 360.0;
		}
		result = startAngle + frac * dayArc;
	}
	double modResult = fmod(result, 360.0);
	if (modResult < 0.0) {
		modResult += 360.0;
	}
	return modResult;
}

LocalHebrewDate LocalHebrewDate::previousYemama() const
{
	return LocalHebrewDate(hebrewDate.prev(), location, custom);
}

LocalHebrewDate LocalHebrewDate::nextYemama() const
{
	return LocalHebrewDate(hebrewDate.next(), location, custom);
}

void LocalHebrewDate::hours(DateTime out[24]) const
{
	long nightLen = nightLength();
	long dayLen = dayLength();
	DateTime yStart = yemamaStart();
	DateTime dStart = dayStart();
	for (int h = 0; h < 12; h++) {
		out[h] = yStart + static_cast<long>((h / 12.0) * nightLen);
	}
	for (int h = 12; h < 24; h++) {
		out[h] = dStart + static_cast<long>(((h - 12) / 12.0) * dayLen);
	}
}

Band LocalHebrewDate::_makeBand(DateTime startDt, Zman startZman, DateTime endDt, Zman endZman, int id, bool stripe) const
{
	Event start_ev(startDt, startZman);
	Event end_ev(endDt, endZman);
	start_ev.localHebrewDateTime = LocalHebrewDateTime(*this, hour(startDt.time));
	end_ev.localHebrewDateTime = LocalHebrewDateTime(*this, hour(endDt.time));
	return Band(start_ev, end_ev, id, stripe);
}

void LocalHebrewDate::_computeEvents()
{
	if (eventsComputed) {
		return;
	}
	eventsComputed = true;

	int idx = 0;
	if (custom.dayDefinition == DayDefinition::GRA) {
		eventsStorage[idx++] = Event(yemamaStart(), Zman::YEMAMA_START);
		eventsStorage[idx++] = Event(nightfall(), Zman::NIGHTFALL);
		eventsStorage[idx++] = Event(midnight(), Zman::MIDNIGHT);
		eventsStorage[idx++] = Event(dawn(), Zman::DAWN);
		eventsStorage[idx++] = Event(misheyakir(), Zman::MISHEYAKIR);
		eventsStorage[idx++] = Event(sunrise(), Zman::SUNRISE);
		eventsStorage[idx++] = Event(shemaEnd(), Zman::SHEMA_END);
		eventsStorage[idx++] = Event(amidahEnd(), Zman::AMIDAH_END);
		eventsStorage[idx++] = Event(noon(), Zman::NOON);
		eventsStorage[idx++] = Event(mincha(), Zman::MINCHA_GEDOLAH);
		eventsStorage[idx++] = Event(minchaKetana(), Zman::MINCHA_KETANA);
		eventsStorage[idx++] = Event(plagMincha(), Zman::PLAG_MINCHA);
		eventsStorage[idx++] = Event(sunset(), Zman::SUNSET);
		eventsStorage[idx++] = Event(yemamaEnd(), Zman::YEMAMA_END);
	} else {
		eventsStorage[idx++] = Event(yemamaStart(), Zman::YEMAMA_START);
		eventsStorage[idx++] = Event(midnight(), Zman::MIDNIGHT);
		eventsStorage[idx++] = Event(dawn(), Zman::DAWN);
		eventsStorage[idx++] = Event(misheyakir(), Zman::MISHEYAKIR);
		eventsStorage[idx++] = Event(sunrise(), Zman::SUNRISE);
		eventsStorage[idx++] = Event(shemaEnd(), Zman::SHEMA_END);
		eventsStorage[idx++] = Event(amidahEnd(), Zman::AMIDAH_END);
		eventsStorage[idx++] = Event(noon(), Zman::NOON);
		eventsStorage[idx++] = Event(mincha(), Zman::MINCHA_GEDOLAH);
		eventsStorage[idx++] = Event(minchaKetana(), Zman::MINCHA_KETANA);
		eventsStorage[idx++] = Event(plagMincha(), Zman::PLAG_MINCHA);
		eventsStorage[idx++] = Event(sunset(), Zman::SUNSET);
		eventsStorage[idx++] = Event(nightfall(), Zman::NIGHTFALL);
		eventsStorage[idx++] = Event(yemamaEnd(), Zman::YEMAMA_END);
	}
	for (int i = 0; i < idx; i++) {
		eventsStorage[i].localHebrewDateTime = LocalHebrewDateTime(*this, hour(eventsStorage[i].datetime.time));
	}
	eventsCount = idx;
}

const Event* LocalHebrewDate::events(int& count)
{
	_computeEvents();
	count = eventsCount;
	return eventsStorage;
}

int LocalHebrewDate::halakhicBands(Band out[LOCAL_HEBREW_DATE_MAX_HALAKHIC_BANDS]) const
{
	DateTime hoursArr[24];
	hours(hoursArr);
	int idx = 0;

	if (custom.dayDefinition == DayDefinition::GRA) {
		out[idx++] = _makeBand(yemamaStart(), Zman::SUNSET, nightfall(), Zman::NIGHTFALL, static_cast<int>(BandId::NIGHTFALL));
		out[idx++] = _makeBand(nightfall(), Zman::NONE, hoursArr[4], Zman::NONE, static_cast<int>(BandId::WATCH1));
		out[idx++] = _makeBand(hoursArr[4], Zman::NONE, hoursArr[8], Zman::NONE, static_cast<int>(BandId::WATCH2));
		out[idx++] = _makeBand(hoursArr[8], Zman::NONE, dawn(), Zman::NONE, static_cast<int>(BandId::WATCH3));
		out[idx++] = _makeBand(dawn(), Zman::DAWN, misheyakir(), Zman::MISHEYAKIR, static_cast<int>(BandId::DAWN));
		out[idx++] = _makeBand(misheyakir(), Zman::MISHEYAKIR, sunrise(), Zman::SUNRISE, static_cast<int>(BandId::MISHEYAKIR));
		out[idx++] = _makeBand(sunrise(), Zman::SUNRISE, shemaEnd(), Zman::SHEMA_END, static_cast<int>(BandId::SHEMA));
		out[idx++] = _makeBand(shemaEnd(), Zman::SHEMA_END, amidahEnd(), Zman::AMIDAH_END, static_cast<int>(BandId::AMIDAH));
		out[idx++] = _makeBand(mincha(), Zman::MINCHA_GEDOLAH, minchaKetana(), Zman::MINCHA_KETANA, static_cast<int>(BandId::MINCHA_GEDOLAH));
		out[idx++] = _makeBand(minchaKetana(), Zman::MINCHA_KETANA, plagMincha(), Zman::PLAG_MINCHA, static_cast<int>(BandId::MINCHA_KETANA));
		out[idx++] = _makeBand(plagMincha(), Zman::PLAG_MINCHA, sunset(), Zman::SUNSET, static_cast<int>(BandId::PLAG_MINCHA));
	} else {
		out[idx++] = _makeBand(yemamaStart(), Zman::NONE, hoursArr[4], Zman::NONE, static_cast<int>(BandId::WATCH1));
		out[idx++] = _makeBand(hoursArr[4], Zman::NONE, hoursArr[8], Zman::NONE, static_cast<int>(BandId::WATCH2));
		out[idx++] = _makeBand(hoursArr[8], Zman::NONE, dayStart(), Zman::NONE, static_cast<int>(BandId::WATCH3));
		out[idx++] = _makeBand(dawn(), Zman::DAWN, misheyakir(), Zman::MISHEYAKIR, static_cast<int>(BandId::DAWN));
		out[idx++] = _makeBand(misheyakir(), Zman::MISHEYAKIR, sunrise(), Zman::SUNRISE, static_cast<int>(BandId::MISHEYAKIR));
		out[idx++] = _makeBand(sunrise(), Zman::SUNRISE, shemaEnd(), Zman::SHEMA_END, static_cast<int>(BandId::SHEMA));
		out[idx++] = _makeBand(shemaEnd(), Zman::SHEMA_END, amidahEnd(), Zman::AMIDAH_END, static_cast<int>(BandId::AMIDAH));
		out[idx++] = _makeBand(mincha(), Zman::MINCHA_GEDOLAH, minchaKetana(), Zman::MINCHA_KETANA, static_cast<int>(BandId::MINCHA_GEDOLAH));
		out[idx++] = _makeBand(minchaKetana(), Zman::MINCHA_KETANA, plagMincha(), Zman::PLAG_MINCHA, static_cast<int>(BandId::MINCHA_KETANA));
		out[idx++] = _makeBand(plagMincha(), Zman::PLAG_MINCHA, sunset(), Zman::SUNSET, static_cast<int>(BandId::PLAG_MINCHA));
		out[idx++] = _makeBand(sunset(), Zman::SUNSET, nightfall(), Zman::NIGHTFALL, static_cast<int>(BandId::NIGHTFALL));
	}
	return idx;
}

int LocalHebrewDate::sunBands(Band out[LOCAL_HEBREW_DATE_MAX_SUN_BANDS]) const
{
	DateTime sunset_prev = sunYesterday.set();
	DateTime sunrise_today = sun.rise();
	DateTime sunset_today = sun.set();

	out[0] = _makeBand(sunset_prev, Zman::SUNSET, sunrise_today, Zman::SUNRISE, static_cast<int>(BandId::NIGHT));
	out[1] = _makeBand(sunrise_today, Zman::SUNRISE, sunset_today, Zman::SUNSET, static_cast<int>(BandId::DAY));

	return 2;
}

int LocalHebrewDate::moonBands(Band out[LOCAL_HEBREW_DATE_MAX_MOON_BANDS]) const
{
	Date date_start = yemamaStart().date;
	Date date_end = yemamaEnd().date;

	Moon moon_start(location, DateTime(date_start, Time(0, 0, 0)));
	Moon moon_end(location, DateTime(date_end, Time(0, 0, 0)));

	DateTime intervalsStart[LOCAL_HEBREW_DATE_MAX_MOON_BANDS];
	DateTime intervalsEnd[LOCAL_HEBREW_DATE_MAX_MOON_BANDS];
	int intervalCount = 0;

	Moon* moons[2] = { &moon_start, &moon_end };
	for (int m = 0; m < 2; m++) {
		Moon* moon = moons[m];
		DateTime midnightDt(moon->datetime.date, Time(0, 0, 0));
		DateTime next_midnight = midnightDt + 86400L;

		if (moon->isAlwaysUp()) {
			intervalsStart[intervalCount] = midnightDt;
			intervalsEnd[intervalCount] = next_midnight;
			intervalCount++;
		} else {
			DateTime* rise = moon->rise();
			DateTime* set_ = moon->set();
			if (rise && set_) {
				if (*rise < *set_) {
					intervalsStart[intervalCount] = *rise;
					intervalsEnd[intervalCount] = *set_;
					intervalCount++;
				} else {
					intervalsStart[intervalCount] = midnightDt;
					intervalsEnd[intervalCount] = *set_;
					intervalCount++;
					intervalsStart[intervalCount] = *rise;
					intervalsEnd[intervalCount] = next_midnight;
					intervalCount++;
				}
			} else if (rise) {
				intervalsStart[intervalCount] = *rise;
				intervalsEnd[intervalCount] = next_midnight;
				intervalCount++;
			} else if (set_) {
				intervalsStart[intervalCount] = midnightDt;
				intervalsEnd[intervalCount] = *set_;
				intervalCount++;
			}
		}
	}

	DateTime y_start = yemamaStart();
	DateTime y_end = yemamaEnd();

	Moon moon_phase_day(location, DateTime(Date::fromordinal(hebrewDate.ordinal), Time(0, 0, 0)));
	MoonPhaseType phase_type = moon_phase_day.illumination().phaseType();

	int idx = 0;
	for (int i = 0; i < intervalCount; i++) {
		DateTime start = intervalsStart[i];
		DateTime end = intervalsEnd[i];
		if (end <= y_start || start >= y_end) {
			continue;
		}
		DateTime clippedStart = (start < y_start) ? y_start : start;
		DateTime clippedEnd = (end < y_end) ? end : y_end;

		out[idx] = _makeBand(clippedStart, Zman::NONE, clippedEnd, Zman::NONE, static_cast<int>(phase_type));
		idx++;
	}

	return idx;
}

int LocalHebrewDate::observanceBands(Band out[LOCAL_HEBREW_DATE_MAX_OBSERVANCE_BANDS]) const
{
	HebrewHoliday holiday(hebrewDate);
	bool isRest = holiday.id == HebrewHolidayId::ROSH_HASHANA ||
		holiday.id == HebrewHolidayId::ROSH_HASHANA_II ||
		holiday.id == HebrewHolidayId::YOM_KIPPUR ||
		holiday.id == HebrewHolidayId::SUKKOT ||
		holiday.id == HebrewHolidayId::PESACH ||
		holiday.id == HebrewHolidayId::PESACH_VII ||
		holiday.id == HebrewHolidayId::SHAVUOT ||
		holiday.id == HebrewHolidayId::SIMCHAT_TORAH_SHEMINI_ATZERETH ||
		holiday.id == HebrewHolidayId::FAST_OF_AV ||
		holiday.id == HebrewHolidayId::YOM_HAATZMAUT ||
		hebrewDate.dayofweek() == Weekday::SATURDAY;

	HebrewHoliday moedHoliday;
	bool hasMoed = holiday.moed(moedHoliday);
	bool isYomTov = hasMoed && !isRest;

	bool isMajorFast = holiday.id == HebrewHolidayId::YOM_KIPPUR || holiday.id == HebrewHolidayId::FAST_OF_AV;

	bool isMinorFast = holiday.id == HebrewHolidayId::FAST_OF_TAMMUZ ||
		holiday.id == HebrewHolidayId::TZOM_GEDALIAH ||
		holiday.id == HebrewHolidayId::FAST_OF_TEVET ||
		holiday.id == HebrewHolidayId::FAST_OF_ESTHER;

	int idx = 0;
	DateTime start = yemamaStart();
	DateTime orig_end = yemamaEnd();
	DateTime end = orig_end;

	bool hasNextRestBand = false;
	bool nextRestStripe = false;
	BandId nextRestBandId = BandId::NONE;

	if (custom.dayDefinition == DayDefinition::MA) {
		HebrewDate next_hdate = hebrewDate.next();
		HebrewHoliday next_holiday(next_hdate);
		bool next_isRest = next_holiday.id == HebrewHolidayId::ROSH_HASHANA ||
			next_holiday.id == HebrewHolidayId::ROSH_HASHANA_II ||
			next_holiday.id == HebrewHolidayId::YOM_KIPPUR ||
			next_holiday.id == HebrewHolidayId::SUKKOT ||
			next_holiday.id == HebrewHolidayId::PESACH ||
			next_holiday.id == HebrewHolidayId::PESACH_VII ||
			next_holiday.id == HebrewHolidayId::SHAVUOT ||
			next_holiday.id == HebrewHolidayId::SIMCHAT_TORAH_SHEMINI_ATZERETH ||
			next_holiday.id == HebrewHolidayId::FAST_OF_AV ||
			next_holiday.id == HebrewHolidayId::YOM_HAATZMAUT ||
			next_hdate.dayofweek() == Weekday::SATURDAY;

		if (next_isRest) {
			end = sunset();
			bool next_major = next_holiday.id == HebrewHolidayId::YOM_KIPPUR || next_holiday.id == HebrewHolidayId::FAST_OF_AV;
			HebrewHoliday next_moedHoliday;
			bool next_hasMoed = next_holiday.moed(next_moedHoliday);
			bool next_yomtov = next_hasMoed && !next_isRest;
			hasNextRestBand = true;
			if (next_major) {
				nextRestStripe = true;
				nextRestBandId = BandId::REST_STRIPED;
			} else if (next_yomtov) {
				nextRestStripe = false;
				nextRestBandId = BandId::YOMTOV;
			} else {
				nextRestStripe = false;
				nextRestBandId = BandId::REST;
			}
		}
	}

	if (!isRest && !isYomTov && !isMajorFast && !isMinorFast) {
		/* no band */
	} else if (isRest && !isMajorFast) {
		out[idx++] = _makeBand(start, Zman::NONE, end, Zman::NONE, static_cast<int>(BandId::REST));
	} else if (isYomTov) {
		out[idx++] = _makeBand(start, Zman::NONE, end, Zman::NONE, static_cast<int>(BandId::YOMTOV));
	} else if (isMajorFast) {
		out[idx++] = _makeBand(start, Zman::NONE, end, Zman::NONE, static_cast<int>(BandId::REST_STRIPED), true);
	} else if (isMinorFast) {
		DateTime pre_fast_end = dawn();
		DateTime post_fast_start = nightfall();

		if (start < pre_fast_end) {
			out[idx++] = _makeBand(start, Zman::NONE, pre_fast_end, Zman::NONE, static_cast<int>(BandId::FAST));
		}
		if (post_fast_start < end) {
			out[idx++] = _makeBand(post_fast_start, Zman::NONE, end, Zman::NONE, static_cast<int>(BandId::FAST));
		}
		DateTime fast_start = dawn();
		DateTime fast_end = nightfall();
		if (fast_start < fast_end) {
			out[idx++] = _makeBand(fast_start, Zman::NONE, fast_end, Zman::NONE, static_cast<int>(BandId::FAST_STRIPED), true);
		}
		if (idx == 0) {
			out[idx++] = _makeBand(start, Zman::NONE, end, Zman::NONE, static_cast<int>(BandId::FAST_STRIPED), true);
		}
	}

	if (hasNextRestBand) {
		out[idx++] = _makeBand(sunset(), Zman::NONE, orig_end, Zman::NONE, static_cast<int>(nextRestBandId), nextRestStripe);
	}

	return idx;
}

/* ---------- ZmanNames ---------- */

const char* ZmanNames::NAMES_EN[17] = {
	"",
	"Yemama Start",
	"Nightfall",
	"Midnight",
	"Dawn",
	"Misheyakir",
	"Sunrise",
	"Shema End",
	"Amidah End",
	"Noon",
	"Minchah Gedolah",
	"Minchah Ketanah",
	"Plag Minchah",
	"Sunset",
	"Yemama End",
	"Moonrise",
	"Moonset"
};

const char* ZmanNames::NAMES_HE[17] = {
	"",
	"\xd7\xaa\xd7\x97\xd7\x99\xd7\x9c\xd7\xaa \xd7\x94\xd7\x99\xd7\x95\xd7\x9d",
	"\xd7\xa6\xd7\x90\xd7\xaa \xd7\x94\xd7\x9b\xd7\x95\xd7\x9b\xd7\x91\xd7\x99\xd7\x9d",
	"\xd7\x97\xd7\xa6\xd7\x95\xd7\xaa \xd7\x94\xd7\x9c\xd7\x99\xd7\x9c\xd7\x94",
	"\xd7\xa2\xd7\x9c\xd7\x95\xd7\xaa \xd7\x94\xd7\xa9\xd7\x97\xd7\xa8",
	"\xd7\x9e\xd7\xa9\xd7\x99\xd7\x9b\xd7\x99\xd7\xa8",
	"\xd7\x94\xd7\xa0\xd7\xa5 \xd7\x94\xd7\x97\xd7\x9e\xd7\x94",
	"\xd7\xa1\xd7\x95\xd7\xa3 \xd7\x96\xd7\x9e\xd7\x9f \xd7\xa9\xd7\x9e\xd7\xa2",
	"\xd7\xa1\xd7\x95\xd7\xa3 \xd7\x96\xd7\x9e\xd7\x9f \xd7\xaa\xd7\xa4\xd7\x99\xd7\x9c\xd7\x94",
	"\xd7\x97\xd7\xa6\xd7\x95\xd7\xaa \xd7\x94\xd7\x99\xd7\x95\xd7\x9d",
	"\xd7\x9e\xd7\xa0\xd7\x97\xd7\x94 \xd7\x92\xd7\x93\xd7\x95\xd7\x9c\xd7\x94",
	"\xd7\x9e\xd7\xa0\xd7\x97\xd7\x94 \xd7\xa7\xd7\x98\xd7\xa0\xd7\x94",
	"\xd7\xa4\xd7\x9c\xd7\x92 \xd7\x94\xd7\x9e\xd7\xa0\xd7\x97\xd7\x94",
	"\xd7\xa9\xd7\xa7\xd7\x99\xd7\xa2\xd7\x94",
	"\xd7\xa1\xd7\x95\xd7\xa3 \xd7\x94\xd7\x99\xd7\x95\xd7\x9d",
	"\xd7\x96\xd7\xa8\xd7\x99\xd7\x97\xd7\xaa \xd7\x99\xd7\xa8\xd7\x97",
	"\xd7\xa9\xd7\xa7\xd7\x99\xd7\xa2\xd7\xaa \xd7\x99\xd7\xa8\xd7\x97"
};

ZmanNames::ZmanNames(Lang lang, Zman zman)
{
	this->lang = lang;
	this->zman = zman;
}

const char* ZmanNames::name() const
{
	int idx = static_cast<int>(zman);
	return (lang == Lang::EN) ? NAMES_EN[idx] : NAMES_HE[idx];
}

/* ---------- HalakhicBandNames ---------- */

const char* HalakhicBandNames::NAMES_EN[12] = {
	"",
	"Nightfall",
	"Dawn",
	"Misheyakir",
	"Shema",
	"Amidah",
	"Minchah Gedolah",
	"Minchah Ketanah",
	"Plag Minchah",
	"Watch 1",
	"Watch 2",
	"Watch 3"
};

const char* HalakhicBandNames::NAMES_HE[12] = {
	"",
	"\xd7\xa6\xd7\x90\xd7\xaa \xd7\x94\xd7\x9b\xd7\x95\xd7\x9b\xd7\x91\xd7\x99\xd7\x9d",
	"\xd7\xa2\xd7\x9c\xd7\x95\xd7\xaa \xd7\x94\xd7\xa9\xd7\x97\xd7\xa8",
	"\xd7\x9e\xd7\xa9\xd7\x99\xd7\x9b\xd7\x99\xd7\xa8",
	"\xd7\xa9\xd7\x9e\xd7\xa2",
	"\xd7\xaa\xd7\xa4\xd7\x99\xd7\x9c\xd7\x94",
	"\xd7\x9e\xd7\xa0\xd7\x97\xd7\x94 \xd7\x92\xd7\x93\xd7\x95\xd7\x9c\xd7\x94",
	"\xd7\x9e\xd7\xa0\xd7\x97\xd7\x94 \xd7\xa7\xd7\x98\xd7\xa0\xd7\x94",
	"\xd7\xa4\xd7\x9c\xd7\x92 \xd7\x94\xd7\x9e\xd7\xa0\xd7\x97\xd7\x94",
	"\xd7\x90\xd7\xa9\xd7\x9e\xd7\x95\xd7\xa8\xd7\x94 \xd7\x90\xd7\xb3",
	"\xd7\x90\xd7\xa9\xd7\x9e\xd7\x95\xd7\xa8\xd7\x94 \xd7\x91\xd7\xb3",
	"\xd7\x90\xd7\xa9\xd7\x9e\xd7\x95\xd7\xa8\xd7\x94 \xd7\x92\xd7\xb3"
};

HalakhicBandNames::HalakhicBandNames(Lang lang, BandId band)
{
	this->lang = lang;
	this->band = band;
}

const char* HalakhicBandNames::name() const
{
	int idx = static_cast<int>(band);
	return (lang == Lang::EN) ? NAMES_EN[idx] : NAMES_HE[idx];
}

/* ---------- DayNightBandNames ---------- */

const char* DayNightBandNames::NAMES_EN[12] = {
	"",
	"Nightfall",
	"Watch 1 (3)",
	"Watch 2 (3)",
	"Watch 3 (3)",
	"Watch 1 (4)",
	"Watch 2 (4)",
	"Watch 3 (4)",
	"Watch 4 (4)",
	"Dawn",
	"Morning",
	"Afternoon"
};

const char* DayNightBandNames::NAMES_HE[12] = {
	"",
	"\xd7\xa9\xd7\xa7\xd7\x99\xd7\xa2\xd7\x94 \xd7\xa2\xd7\x93 \xd7\xa6\xd7\x90\xd7\xaa",
	"\xd7\x90\xd7\xa9\xd7\x9e\xd7\x95\xd7\xa8\xd7\x94 \xd7\x90\xd7\xb3 (3)",
	"\xd7\x90\xd7\xa9\xd7\x9e\xd7\x95\xd7\xa8\xd7\x94 \xd7\x91\xd7\xb3 (3)",
	"\xd7\x90\xd7\xa9\xd7\x9e\xd7\x95\xd7\xa8\xd7\x94 \xd7\x92\xd7\xb3 (3)",
	"\xd7\x90\xd7\xa9\xd7\x9e\xd7\x95\xd7\xa8\xd7\x94 \xd7\x90\xd7\xb3 (4)",
	"\xd7\x90\xd7\xa9\xd7\x9e\xd7\x95\xd7\xa8\xd7\x94 \xd7\x91\xd7\xb3 (4)",
	"\xd7\x90\xd7\xa9\xd7\x9e\xd7\x95\xd7\xa8\xd7\x94 \xd7\x92\xd7\xb3 (4)",
	"\xd7\x90\xd7\xa9\xd7\x9e\xd7\x95\xd7\xa8\xd7\x94 \xd7\x93\xd7\xb3 (4)",
	"\xd7\xa2\xd7\x9c\xd7\x95\xd7\xaa \xd7\x94\xd7\xa9\xd7\x97\xd7\xa8 \xd7\xa2\xd7\x93 \xd7\xa0\xd7\xa5",
	"\xd7\x91\xd7\x95\xd7\xa7\xd7\xa8",
	"\xd7\xa6\xd7\x94\xd7\xa8\xd7\x99\xd7\x99\xd7\x9d"
};

DayNightBandNames::DayNightBandNames(Lang lang, int band)
{
	this->lang = lang;
	this->band = band;
}

const char* DayNightBandNames::name() const
{
	return (lang == Lang::EN) ? NAMES_EN[band] : NAMES_HE[band];
}
