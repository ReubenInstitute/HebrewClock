from enum import IntEnum

from Date import DateTime, Date, Time
from Sun import Sun
from Custom import Custom, DayDefinition, DawnCustom, NightfallCustom, MisheyakirCustom
from Moon import Moon, MoonPhaseType, MoonPhaseColor


class Zman(IntEnum):
	YEMAMA_START = 1
	NIGHTFALL = 2
	MIDNIGHT = 3
	DAWN = 4
	MISHEYAKIR = 5
	SUNRISE = 6
	SHEMA_END = 7
	AMIDAH_END = 8
	NOON = 9
	MINCHA_GEDOLAH = 10
	MINCHA_KETANA = 11
	PLAG_MINCHA = 12
	SUNSET = 13
	YEMAMA_END = 14
	MOONRISE = 15
	MOONSET = 16

class BandId(IntEnum):
	NIGHTFALL = 1
	DAWN = 2
	MISHEYAKIR = 3
	SHEMA = 4
	AMIDAH = 5
	MINCHA_GEDOLAH = 6
	MINCHA_KETANA = 7
	PLAG_MINCHA = 8
	WATCH1 = 9
	WATCH2 = 10
	WATCH3 = 11
	NONE = 0
	REST = 101
	REST_STRIPED = 102
	YOMTOV = 103
	FAST = 104
	FAST_STRIPED = 105
	NIGHT = 201
	DAY = 202

class Event:
	def __init__(self, datetime, zman=None):
		self.datetime = datetime
		self.zman = zman

	def __repr__(self):
		if self.zman is None:
			name = "—"
		else:
			name = ZmanNames('en', self.zman).name()
		return f"Event({name}, {self.datetime!r})"

class LocalHebrewDateTime:
	def __init__(self, localHebrewDate, hour):
		self.localHebrewDate = localHebrewDate
		self.hour = hour


class Band:
	def __init__(self, start, end, id=None, stripe=False):
		self.start = start
		self.end = end
		self.id = id
		self.stripe = stripe

	def __repr__(self):
		name = HalakhicBandNames('en', self.id).name() if self.id else "—"
		return f"Band({name}, {self.start.zman.name if self.start.zman else '—'} → {self.end.zman.name if self.end.zman else '—'}, stripe={self.stripe})"



class LocalHebrewDate:
	def __init__(self, hebrewDate, location, custom):
		self.hebrewDate = hebrewDate
		self.location = location
		self.custom = custom
		self._events = None

	@property
	def events(self):
		if self._events:
			return self._events
		if self.custom.dayDefinition == DayDefinition.GRA:
			events = [
				Event(self.yemamaStart, Zman.YEMAMA_START),
				Event(self.nightfall, Zman.NIGHTFALL),
				Event(self.midnight, Zman.MIDNIGHT),
				Event(self.dawn, Zman.DAWN),
				Event(self.misheyakir, Zman.MISHEYAKIR),
				Event(self.sunrise, Zman.SUNRISE),
				Event(self.shemaEnd, Zman.SHEMA_END),
				Event(self.amidahEnd, Zman.AMIDAH_END),
				Event(self.noon, Zman.NOON),
				Event(self.mincha, Zman.MINCHA_GEDOLAH),
				Event(self.minchaKetana, Zman.MINCHA_KETANA),
				Event(self.plagMincha, Zman.PLAG_MINCHA),
				Event(self.sunset, Zman.SUNSET),
				Event(self.yemamaEnd, Zman.YEMAMA_END),
			]
		else:
			events = [
				Event(self.yemamaStart, Zman.YEMAMA_START),
				Event(self.midnight, Zman.MIDNIGHT),
				Event(self.dawn, Zman.DAWN),
				Event(self.misheyakir, Zman.MISHEYAKIR),
				Event(self.sunrise, Zman.SUNRISE),
				Event(self.shemaEnd, Zman.SHEMA_END),
				Event(self.amidahEnd, Zman.AMIDAH_END),
				Event(self.noon, Zman.NOON),
				Event(self.mincha, Zman.MINCHA_GEDOLAH),
				Event(self.minchaKetana, Zman.MINCHA_KETANA),
				Event(self.plagMincha, Zman.PLAG_MINCHA),
				Event(self.sunset, Zman.SUNSET),
				Event(self.nightfall, Zman.NIGHTFALL),
				Event(self.yemamaEnd, Zman.YEMAMA_END),
			]
		for e in events:
			e.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(e.datetime.time))
		self._events = events
		return self._events

	@property
	def sun(self):
		if not hasattr(self, '_sun_cache'):
			dt = DateTime(Date.fromordinal(self.hebrewDate.ordinal), Time(0, 0, 0))
			self._sun_cache = Sun(self.location, dt)
		return self._sun_cache

	@property
	def sunYesterday(self):
		if not hasattr(self, '_sunYesterday_cache'):
			dt = DateTime(Date.fromordinal(self.hebrewDate.ordinal - 1), Time(0, 0, 0))
			self._sunYesterday_cache = Sun(self.location, dt)
		return self._sunYesterday_cache

	@property
	def sunrise(self):
		return self.sun.rise

	@property
	def sunset(self):
		return self.sun.set

	@property
	def dawn(self):
		sr = self.sun.rise
		astroDaySec = (self.sunset - sr).seconds
		c = self.custom.dawn
		if c == DawnCustom.DEG_16_1:
			return self.sun.dawnTime(16.1)
		if c == DawnCustom.DEG_16_9:
			return self.sun.dawnTime(16.9)
		if c == DawnCustom.DEG_18:
			return self.sun.dawnTime(18)
		if c == DawnCustom.DEG_19_8:
			return self.sun.dawnTime(19.8)
		if c == DawnCustom.MIN_72:
			return sr + (-72 * 60)
		if c == DawnCustom.REL_MIN_72:
			return sr + (-astroDaySec / 10.0)
		return sr

	def _nightfallFor(self, sunRef):
		sunsetRef = sunRef.set
		refDayLength = 0.0
		if self.custom.nightfall in (NightfallCustom.REL_MIN_72, NightfallCustom.REL_MIN_90):
			refDayLength = (sunsetRef - sunRef.rise).seconds

		c = self.custom.nightfall
		if c == NightfallCustom.DEG_5_95:
			return sunRef.duskTime(5.95)
		if c == NightfallCustom.DEG_6:
			return sunRef.duskTime(6)
		if c == NightfallCustom.DEG_6_45:
			return sunRef.duskTime(6.45)
		if c == NightfallCustom.DEG_7_1:
			return sunRef.duskTime(7.1)
		if c == NightfallCustom.DEG_8:
			return sunRef.duskTime(8)
		if c == NightfallCustom.DEG_8_5:
			return sunRef.duskTime(8.5)
		if c == NightfallCustom.MIN_72:
			return sunsetRef + (72 * 60)
		if c == NightfallCustom.REL_MIN_72:
			return sunsetRef + (refDayLength / 10.0)
		if c == NightfallCustom.REL_MIN_90:
			return sunsetRef + (refDayLength / 8.0)
		if c == NightfallCustom.DEG_16_1:
			return sunRef.duskTime(16.1)
		if c == NightfallCustom.DEG_18:
			return sunRef.duskTime(18)
		if c == NightfallCustom.DEG_19_8:
			return sunRef.duskTime(19.8)
		return sunsetRef

	@property
	def nightfall(self):
		if self.custom.dayDefinition == DayDefinition.GRA:
			return self._nightfallFor(self.sunYesterday)
		else:
			return self._nightfallFor(self.sun)

	@property
	def misheyakir(self):
		sr = self.sun.rise
		c = self.custom.misheyakir
		if c == MisheyakirCustom.DEG_11_5:
			return self.sun.dawnTime(11.5)
		if c == MisheyakirCustom.DEG_11:
			return self.sun.dawnTime(11)
		if c == MisheyakirCustom.DEG_10_2:
			return self.sun.dawnTime(10.2)
		if c == MisheyakirCustom.MIN_50:
			return sr + (-50 * 60)
		if c == MisheyakirCustom.DEG_7_65:
			return self.sun.dawnTime(7.65)
		return sr

	@property
	def yemamaStart(self):
		if self.custom.dayDefinition == DayDefinition.GRA:
			return self.sunYesterday.set
		else:
			return self._nightfallFor(self.sunYesterday)

	@property
	def dayStart(self):
		return self.sunrise if self.custom.dayDefinition == DayDefinition.GRA else self.dawn

	@property
	def yemamaEnd(self):
		return self.sunset if self.custom.dayDefinition == DayDefinition.GRA else self.nightfall

	@property
	def shemaEnd(self):
		return self.dayStart + ((3.0 / 12.0) * self.dayLength)

	@property
	def amidahEnd(self):
		return self.dayStart + ((4.0 / 12.0) * self.dayLength)

	@property
	def noon(self):
		return self.dayStart + ((6.0 / 12.0) * self.dayLength)

	@property
	def mincha(self):
		return self.dayStart + ((6.5 / 12.0) * self.dayLength)

	@property
	def minchaKetana(self):
		return self.dayStart + ((9.5 / 12.0) * self.dayLength)

	@property
	def plagMincha(self):
		return self.dayStart + ((10.75 / 12.0) * self.dayLength)

	@property
	def midnight(self):
		return self.yemamaStart + (self.nightLength / 2.0)

	@property
	def dayLength(self):
		return (self.yemamaEnd - self.dayStart).seconds

	@property
	def nightLength(self):
		return (self.dayStart - self.yemamaStart).seconds

	def hour(self, time):
		now = DateTime(Date.fromordinal(self.hebrewDate.ordinal), time)
		nowSec = (now - self.yemamaStart).seconds
		if nowSec < 0:
			nowSec = 0

		nightLen = self.nightLength
		dayLen = self.dayLength

		if nowSec < nightLen:
			return 12.0 * (nowSec / nightLen)
		elif nowSec <= nightLen + dayLen:
			dayElapsed = nowSec - nightLen
			return 12.0 + 12.0 * (dayElapsed / dayLen)
		else:
			next_ = self.nextYemama
			nextNightLen = next_.nightLength
			afterEnd = nowSec - (nightLen + dayLen)
			return 12.0 * (afterEnd / nextNightLen)

	@property
	def previousYemama(self):
		return LocalHebrewDate(self.hebrewDate.prev, self.location, self.custom)

	@property
	def nextYemama(self):
		return LocalHebrewDate(self.hebrewDate.next, self.location, self.custom)

	def angle(self, hour):
		total = self.dayLength + self.nightLength
		dayArc = (self.dayLength / total) * 360.0
		nightArc = 360.0 - dayArc

		if hour < 12:
			frac = hour / 12.0
			return (dayArc / 2.0 + frac * nightArc) % 360.0
		else:
			frac = (hour - 12.0) / 12.0
			startAngle = (-dayArc / 2.0) % 360.0
			return (startAngle + frac * dayArc) % 360.0



	@property
	def halakhicBands(self):
		bands = []
		if self.custom.dayDefinition == DayDefinition.GRA:
			# Nightfall band
			start = Event(self.yemamaStart, Zman.SUNSET)
			end = Event(self.nightfall, Zman.NIGHTFALL)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.NIGHTFALL))

			# Watch 1 : nightfall → hour 4
			start = Event(self.nightfall)
			end   = Event(self.hours[4])
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime   = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.WATCH1))

			# Watch 2 : hour 4 → hour 8
			start = Event(self.hours[4])
			end   = Event(self.hours[8])
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime   = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.WATCH2))

			# Watch 3 : hour 8 → dawn
			start = Event(self.hours[8])
			end   = Event(self.dawn)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime   = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.WATCH3))

			# Dawn band
			start = Event(self.dawn, Zman.DAWN)
			end = Event(self.misheyakir, Zman.MISHEYAKIR)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.DAWN))

			# Misheyakir band
			start = Event(self.misheyakir, Zman.MISHEYAKIR)
			end = Event(self.sunrise, Zman.SUNRISE)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.MISHEYAKIR))

			# Daytime bands
			start = Event(self.sunrise, Zman.SUNRISE)
			end = Event(self.shemaEnd, Zman.SHEMA_END)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.SHEMA))

			start = Event(self.shemaEnd, Zman.SHEMA_END)
			end = Event(self.amidahEnd, Zman.AMIDAH_END)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.AMIDAH))

			start = Event(self.mincha, Zman.MINCHA_GEDOLAH)
			end = Event(self.minchaKetana, Zman.MINCHA_KETANA)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.MINCHA_GEDOLAH))

			start = Event(self.minchaKetana, Zman.MINCHA_KETANA)
			end = Event(self.plagMincha, Zman.PLAG_MINCHA)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.MINCHA_KETANA))

			start = Event(self.plagMincha, Zman.PLAG_MINCHA)
			end = Event(self.sunset, Zman.SUNSET)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.PLAG_MINCHA))
		else:
			# Watch 1 : nightfall → hour 4
			start = Event(self.yemamaStart)
			end   = Event(self.hours[4])
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime   = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.WATCH1))

			# Watch 2 : hour 4 → hour 8
			start = Event(self.hours[4])
			end   = Event(self.hours[8])
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime   = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.WATCH2))

			# Watch 3 : hour 8 → dawn
			start = Event(self.hours[8])
			end   = Event(self.dayStart)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime   = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.WATCH3))

			# Dawn band
			start = Event(self.dawn, Zman.DAWN)
			end = Event(self.misheyakir, Zman.MISHEYAKIR)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.DAWN))

			# Misheyakir band
			start = Event(self.misheyakir, Zman.MISHEYAKIR)
			end = Event(self.sunrise, Zman.SUNRISE)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.MISHEYAKIR))

			# Daytime bands
			start = Event(self.sunrise, Zman.SUNRISE)
			end = Event(self.shemaEnd, Zman.SHEMA_END)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.SHEMA))

			start = Event(self.shemaEnd, Zman.SHEMA_END)
			end = Event(self.amidahEnd, Zman.AMIDAH_END)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.AMIDAH))

			start = Event(self.mincha, Zman.MINCHA_GEDOLAH)
			end = Event(self.minchaKetana, Zman.MINCHA_KETANA)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.MINCHA_GEDOLAH))

			start = Event(self.minchaKetana, Zman.MINCHA_KETANA)
			end = Event(self.plagMincha, Zman.PLAG_MINCHA)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.MINCHA_KETANA))

			start = Event(self.plagMincha, Zman.PLAG_MINCHA)
			end = Event(self.sunset, Zman.SUNSET)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.PLAG_MINCHA))

			# Nightfall band
			start = Event(self.sunset, Zman.SUNSET)
			end = Event(self.nightfall, Zman.NIGHTFALL)
			start.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.datetime.time))
			end.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.datetime.time))
			bands.append(Band(start, end, BandId.NIGHTFALL))
		return bands
	
	
	@property
	def sunBands(self):
		bands = []
		sunset_prev = self.sunYesterday.set
		sunrise_today = self.sun.rise
		sunset_today = self.sun.set

		start_ev = Event(sunset_prev, Zman.SUNSET)
		end_ev = Event(sunrise_today, Zman.SUNRISE)
		start_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(sunset_prev.time))
		end_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(sunrise_today.time))
		bands.append(Band(start_ev, end_ev, BandId.NIGHT))

		start_ev = Event(sunrise_today, Zman.SUNRISE)
		end_ev = Event(sunset_today, Zman.SUNSET)
		start_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(sunrise_today.time))
		end_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(sunset_today.time))
		bands.append(Band(start_ev, end_ev, BandId.DAY))

		return bands

	@property
	def moonBands(self):
		date_start = self.yemamaStart.date
		date_end = self.yemamaEnd.date

		moon_start = Moon(self.location, DateTime(date_start, Time(0, 0, 0)))
		moon_end = Moon(self.location, DateTime(date_end, Time(0, 0, 0)))

		intervals = []
		for moon in (moon_start, moon_end):
			midnight = DateTime(moon.datetime.date, Time(0, 0, 0))
			next_midnight = midnight + 86400

			if moon.isAlwaysUp:
				intervals.append((midnight, next_midnight))
			else:
				rise = moon.rise
				set_ = moon.set
				if rise is not None and set_ is not None:
					if rise < set_:
						intervals.append((rise, set_))
					else:
						intervals.append((midnight, set_))
						intervals.append((rise, next_midnight))
				elif rise is not None:
					intervals.append((rise, next_midnight))
				elif set_ is not None:
					intervals.append((midnight, set_))

		clipped = []
		y_start = self.yemamaStart
		y_end = self.yemamaEnd
		for start, end in intervals:
			if end <= y_start or start >= y_end:
				continue
			clipped.append((max(start, y_start), min(end, y_end)))

		moon_phase_day = Moon(
			self.location,
			DateTime(Date.fromordinal(self.hebrewDate.ordinal), Time(0, 0, 0))
		)
		phase_type = moon_phase_day.illumination.phaseType

		bands = []
		for start_dt, end_dt in clipped:
			start_ev = Event(start_dt)
			end_ev = Event(end_dt)
			start_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start_dt.time))
			end_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end_dt.time))
			bands.append(Band(start_ev, end_ev, phase_type))

		return bands

	@property
	def hours(self):
		nightLen = self.nightLength
		dayLen = self.dayLength
		yStart = self.yemamaStart
		dStart = self.dayStart
		result = []
		for h in range(12):
			result.append(yStart + int((h / 12.0) * nightLen))
		for h in range(12, 24):
			result.append(dStart + int(((h - 12) / 12.0) * dayLen))
		return result




	@property
	def observanceBands(self):
		from HebrewHoliday import HebrewHoliday, HebrewHolidayId

		holiday = HebrewHoliday(self.hebrewDate)
		isRest = holiday.id in (
			HebrewHolidayId.ROSH_HASHANA,
			HebrewHolidayId.ROSH_HASHANA_II,
			HebrewHolidayId.YOM_KIPPUR,
			HebrewHolidayId.SUKKOT,
			HebrewHolidayId.PESACH,
			HebrewHolidayId.PESACH_VII,
			HebrewHolidayId.SHAVUOT,
			HebrewHolidayId.SIMCHAT_TORAH_SHEMINI_ATZERETH,
			HebrewHolidayId.FAST_OF_AV,
			HebrewHolidayId.YOM_HAATZMAUT,
		) or self.hebrewDate.dayofweek == 7

		# Chol Hamoed (and Hoshana Raba) days fall inside the moed window
		# but aren't already flagged as full rest days.
		isYomTov = holiday.moed is not None and not isRest

		isMajorFast = holiday.id in (
			HebrewHolidayId.YOM_KIPPUR,
			HebrewHolidayId.FAST_OF_AV,
		)

		isMinorFast = holiday.id in (
			HebrewHolidayId.FAST_OF_TAMMUZ,
			HebrewHolidayId.TZOM_GEDALIAH,
			HebrewHolidayId.FAST_OF_TEVET,
			HebrewHolidayId.FAST_OF_ESTHER,
		)

		bands = []
		start = self.yemamaStart
		orig_end = self.yemamaEnd
		end = orig_end

		next_rest_band = None
		if self.custom.dayDefinition == DayDefinition.MA:
			next_hdate = self.hebrewDate.next
			next_holiday = HebrewHoliday(next_hdate)
			next_isRest = next_holiday.id in (
				HebrewHolidayId.ROSH_HASHANA,
				HebrewHolidayId.ROSH_HASHANA_II,
				HebrewHolidayId.YOM_KIPPUR,
				HebrewHolidayId.SUKKOT,
				HebrewHolidayId.PESACH,
				HebrewHolidayId.PESACH_VII,
				HebrewHolidayId.SHAVUOT,
				HebrewHolidayId.SIMCHAT_TORAH_SHEMINI_ATZERETH,
				HebrewHolidayId.FAST_OF_AV,
				HebrewHolidayId.YOM_HAATZMAUT,
			) or next_hdate.dayofweek == 7

			if next_isRest:
				end = self.sunset
				next_major = next_holiday.id in (HebrewHolidayId.YOM_KIPPUR, HebrewHolidayId.FAST_OF_AV)
				next_yomtov = next_holiday.moed is not None and not next_isRest
				if next_major:
					next_rest_band = (True, BandId.REST_STRIPED)
				elif next_yomtov:
					next_rest_band = (False, BandId.YOMTOV)
				else:
					next_rest_band = (False, BandId.REST)

		if not isRest and not isYomTov and not isMajorFast and not isMinorFast:
			pass

		elif isRest and not isMajorFast:
			start_ev = Event(start)
			end_ev = Event(end)
			start_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.time))
			end_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.time))
			bands.append(Band(start_ev, end_ev, id=BandId.REST))

		elif isYomTov:
			start_ev = Event(start)
			end_ev = Event(end)
			start_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.time))
			end_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.time))
			bands.append(Band(start_ev, end_ev, id=BandId.YOMTOV))

		elif isMajorFast:
			start_ev = Event(start)
			end_ev = Event(end)
			start_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.time))
			end_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.time))
			bands.append(Band(start_ev, end_ev, id=BandId.REST_STRIPED, stripe=True))

		elif isMinorFast:
			pre_fast_end = self.dawn
			post_fast_start = self.nightfall

			if start < pre_fast_end:
				start_ev = Event(start)
				end_ev = Event(pre_fast_end)
				start_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.time))
				end_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(pre_fast_end.time))
				bands.append(Band(start_ev, end_ev, id=BandId.FAST))
			if post_fast_start < end:
				start_ev = Event(post_fast_start)
				end_ev = Event(end)
				start_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(post_fast_start.time))
				end_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.time))
				bands.append(Band(start_ev, end_ev, id=BandId.FAST))
			fast_start = self.dawn
			fast_end = self.nightfall
			if fast_start < fast_end:
				start_ev = Event(fast_start)
				end_ev = Event(fast_end)
				start_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(fast_start.time))
				end_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(fast_end.time))
				bands.append(Band(start_ev, end_ev, id=BandId.FAST_STRIPED, stripe=True))

			if not bands:
				start_ev = Event(start)
				end_ev = Event(end)
				start_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start.time))
				end_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end.time))
				bands.append(Band(start_ev, end_ev, id=BandId.FAST_STRIPED, stripe=True))

		if next_rest_band is not None:
			stripe, band_id = next_rest_band
			start_ev = Event(self.sunset)
			end_ev = Event(orig_end)
			start_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(start_ev.datetime.time))
			end_ev.localHebrewDateTime = LocalHebrewDateTime(self, self.hour(end_ev.datetime.time))
			bands.append(Band(start_ev, end_ev, id=band_id, stripe=stripe))

		return bands





class ZmanNames:
	NAMES_EN = [
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
		"Moonset",
	]
	NAMES_HE = [
		"",
		"תחילת היום",
		"צאת הכוכבים",
		"חצות הלילה",
		"עלות השחר",
		"משיכיר",
		"הנץ החמה",
		"סוף זמן שמע",
		"סוף זמן תפילה",
		"חצות היום",
		"מנחה גדולה",
		"מנחה קטנה",
		"פלג המנחה",
		"שקיעה",
		"סוף היום",
		"זריחת ירח",
		"שקיעת ירח",
	]

	def __init__(self, lang, zman):
		self.lang = lang
		self.zman = zman

	def name(self):
		if self.lang == 'en':
			return self.NAMES_EN[self.zman]
		return self.NAMES_HE[self.zman]


class HalakhicBandNames:
	NAMES_EN = [
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
		"Watch 3",
	]
	NAMES_HE = [
		"",
		"צאת הכוכבים",
		"עלות השחר",
		"משיכיר",
		"שמע",
		"תפילה",
		"מנחה גדולה",
		"מנחה קטנה",
		"פלג המנחה",
		"אשמורה א׳",
		"אשמורה ב׳",
		"אשמורה ג׳",
	]

	def __init__(self, lang, band):
		self.lang = lang
		self.band = band

	def name(self):
		if self.lang == 'en':
			return self.NAMES_EN[self.band]
		return self.NAMES_HE[self.band]


class DayNightBandNames:
	NAMES_EN = [
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
		"Afternoon",
	]
	NAMES_HE = [
		"",
		"שקיעה עד צאת",
		"אשמורה א׳ (3)",
		"אשמורה ב׳ (3)",
		"אשמורה ג׳ (3)",
		"אשמורה א׳ (4)",
		"אשמורה ב׳ (4)",
		"אשמורה ג׳ (4)",
		"אשמורה ד׳ (4)",
		"עלות השחר עד נץ",
		"בוקר",
		"צהריים",
	]

	def __init__(self, lang, band):
		self.lang = lang
		self.band = band

	def name(self):
		if self.lang == 'en':
			return self.NAMES_EN[self.band]
		return self.NAMES_HE[self.band]