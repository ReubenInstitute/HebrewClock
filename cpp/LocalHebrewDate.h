/* LocalHebrewDate.h
 * Ported from LIBS/LocalHebrewDate.py
 */
#ifndef LOCALHEBREWDATE_H
#define LOCALHEBREWDATE_H

#include "Custom.h"
#include "Date.h"
#include "Geo.h"
#include "HebrewDate.h"
#include "Moon.h"
#include "Sun.h"

enum class Zman {
	NONE = 0,   /* added sentinel: Python's Event.zman defaults to None, not a real IntEnum member */
	YEMAMA_START = 1,
	NIGHTFALL = 2,
	MIDNIGHT = 3,
	DAWN = 4,
	MISHEYAKIR = 5,
	SUNRISE = 6,
	SHEMA_END = 7,
	AMIDAH_END = 8,
	NOON = 9,
	MINCHA_GEDOLAH = 10,
	MINCHA_KETANA = 11,
	PLAG_MINCHA = 12,
	SUNSET = 13,
	YEMAMA_END = 14,
	MOONRISE = 15,
	MOONSET = 16
};

enum class BandId {
	NONE = 0,
	NIGHTFALL = 1,
	DAWN = 2,
	MISHEYAKIR = 3,
	SHEMA = 4,
	AMIDAH = 5,
	MINCHA_GEDOLAH = 6,
	MINCHA_KETANA = 7,
	PLAG_MINCHA = 8,
	WATCH1 = 9,
	WATCH2 = 10,
	WATCH3 = 11,
	REST = 101,
	REST_STRIPED = 102,
	YOMTOV = 103,
	FAST = 104,
	FAST_STRIPED = 105,
	NIGHT = 201,
	DAY = 202
};

class LocalHebrewDate;   /* forward decl; LocalHebrewDateTime aliases one, never owns/copies it */

/* Pointer, not reference: LocalHebrewDateTime/Event/Band need to live in
 * fixed arrays (default-constructed then filled in place, same convention
 * used throughout this port), and a reference member would make that
 * impossible (references can't be default-constructed or reseated).
 * Mirrors Python's aliasing (self.localHebrewDate = localHebrewDate) via a
 * pointer instead of a reference for that structural reason. */
class LocalHebrewDateTime {
public:
	const LocalHebrewDate* localHebrewDate;
	double hour;

	LocalHebrewDateTime();
	LocalHebrewDateTime(const LocalHebrewDate& localHebrewDate, double hour);
};

class Event {
public:
	DateTime datetime;
	Zman zman;
	LocalHebrewDateTime localHebrewDateTime;

	Event();
	explicit Event(DateTime datetime, Zman zman = Zman::NONE);
};

/* Band::id is polymorphic in Python: halakhicBands/sunBands/observanceBands
 * tag it with a BandId, but moonBands tags it with a MoonPhaseType (Moon.py)
 * instead -- and unlike HebrewHolidayId's sub-enums, BandId and
 * MoonPhaseType numeric ranges genuinely collide (both start at 1), so they
 * can't be merged into one enum the way HebrewHolidayId's were. Ported as a
 * plain int id here; callers interpret it as BandId or MoonPhaseType based
 * on which band-collection method produced it (never mixed within one
 * array), same as Python's late-bound duck typing. */
class Band {
public:
	Event start;
	Event end;
	int id;
	bool stripe;

	Band();
	Band(Event start, Event end, int id = static_cast<int>(BandId::NONE), bool stripe = false);
};

#define LOCAL_HEBREW_DATE_MAX_EVENTS 14
#define LOCAL_HEBREW_DATE_MAX_HALAKHIC_BANDS 11
#define LOCAL_HEBREW_DATE_MAX_SUN_BANDS 2
#define LOCAL_HEBREW_DATE_MAX_MOON_BANDS 4
#define LOCAL_HEBREW_DATE_MAX_OBSERVANCE_BANDS 5

class LocalHebrewDate {
public:
	HebrewDate hebrewDate;
	Location location;
	Custom custom;
	Sun sun;
	Sun sunYesterday;

	LocalHebrewDate(HebrewDate hebrewDate, Location location, Custom custom);

	/* Cached like Python's self._events (the only property Python itself
	 * caches -- computing it re-triggers several Sun trig calls). Returns a
	 * pointer into this object's own storage, valid as long as *this is. */
	const Event* events(int& count);

	DateTime sunrise() const;
	DateTime sunset() const;
	DateTime dawn() const;
	DateTime nightfall() const;
	DateTime misheyakir() const;
	DateTime yemamaStart() const;
	DateTime dayStart() const;
	DateTime yemamaEnd() const;
	DateTime shemaEnd() const;
	DateTime amidahEnd() const;
	DateTime noon() const;
	DateTime mincha() const;
	DateTime minchaKetana() const;
	DateTime plagMincha() const;
	DateTime midnight() const;
	long dayLength() const;
	long nightLength() const;

	double hour(const Time& time) const;
	double angle(double hour) const;

	LocalHebrewDate previousYemama() const;
	LocalHebrewDate nextYemama() const;

	void hours(DateTime out[24]) const;

	int halakhicBands(Band out[LOCAL_HEBREW_DATE_MAX_HALAKHIC_BANDS]) const;
	int sunBands(Band out[LOCAL_HEBREW_DATE_MAX_SUN_BANDS]) const;
	int moonBands(Band out[LOCAL_HEBREW_DATE_MAX_MOON_BANDS]) const;
	int observanceBands(Band out[LOCAL_HEBREW_DATE_MAX_OBSERVANCE_BANDS]) const;

private:
	DateTime _nightfallFor(const Sun& sunRef) const;
	Band _makeBand(DateTime startDt, Zman startZman, DateTime endDt, Zman endZman, int id, bool stripe = false) const;

	bool eventsComputed;
	Event eventsStorage[LOCAL_HEBREW_DATE_MAX_EVENTS];
	int eventsCount;
	void _computeEvents();
};

class ZmanNames {
public:
	static const char* NAMES_EN[17];
	static const char* NAMES_HE[17];

	Lang lang;
	Zman zman;

	ZmanNames(Lang lang, Zman zman);

	const char* name() const;
};

class HalakhicBandNames {
public:
	static const char* NAMES_EN[12];
	static const char* NAMES_HE[12];

	Lang lang;
	BandId band;

	HalakhicBandNames(Lang lang, BandId band);

	const char* name() const;
};

class DayNightBandNames {
public:
	static const char* NAMES_EN[12];
	static const char* NAMES_HE[12];

	Lang lang;
	int band;

	DayNightBandNames(Lang lang, int band);

	const char* name() const;
};

#endif
