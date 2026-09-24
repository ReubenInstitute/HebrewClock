import math

from Date import Date, Time, DateTime
from HebrewDate import HebrewDate
from LocalHebrewDate import (LocalHebrewDate, Band,
                             HalakhicBandNames, DayNightBandNames, BandId)
from Custom import Custom
from Geo import Location
from Moon import MoonPhaseColor

from Display import Display

TIMEZONE = 3 * 3600


class VisualBand:
	def __init__(self, startAngle, endAngle, color, stripe=False):
		self.startAngle = startAngle
		self.endAngle = endAngle
		self.color = color
		self.stripe = stripe


class Clock:
	SUN_BAND_COLORS = {
		BandId.NIGHT: 0x4A4A55,
		BandId.DAY: 0xFFD700,
	}

	HALAKHIC_BAND_COLORS = {
		BandId.NIGHTFALL: 0x8A6A4A,
		BandId.DAWN: 0x375C7B,
		BandId.MISHEYAKIR: 0x567D9F,
		BandId.SHEMA: 0x829DB5,
		BandId.AMIDAH: 0xAEBECB,
		BandId.MINCHA_GEDOLAH: 0x7A8C6D,
		BandId.MINCHA_KETANA: 0xCBB88C,
		BandId.PLAG_MINCHA: 0xB08A5C,
		BandId.WATCH1: 0x9A9A9A,
		BandId.WATCH2: 0x1A1A1A,
		BandId.WATCH3: 0x9A9A9A,
	}

	OBSERVANCE_BAND_COLORS = {
		BandId.REST: 0xA05050,
		BandId.REST_STRIPED: 0xA05050,
		BandId.YOMTOV: 0xD07070,
		BandId.FAST: 0xE6C300,
		BandId.FAST_STRIPED: 0xE6C300,
	}

	@classmethod
	def _halakhicColor(cls, band):
		return cls.HALAKHIC_BAND_COLORS[band.id]

	@classmethod
	def _observanceColor(cls, band):
		return cls.OBSERVANCE_BAND_COLORS[band.id]

	@classmethod
	def _moonColor(cls, band):
		return MoonPhaseColor[band.id.name]

	@classmethod
	def _sunColor(cls, band):
		return cls.SUN_BAND_COLORS[band.id]

	BACKGROUND_COLOR = 0xFFFFFF
	TICK_COLOR = 0x333333
	HAND_COLOR = 0x222222
	SECOND_HAND_COLOR = 0xCC0000
	FACE_COLOR = 0x333333

	DAY_LABEL = 0xC0C0C0
	DAY_LIGHT_LABEL = 0xE0E0E0
	WATCH_1_3_LABEL = 0x3A3A3A
	WATCH_1_3_LIGHT_LABEL = 0x5A5A5A
	WATCH_2_3_LABEL = 0x6A6A6A
	WATCH_2_3_LIGHT_LABEL = 0x8A8A8A
	WATCH_3_3_LABEL = 0x9A9A9A
	WATCH_3_3_LIGHT_LABEL = 0xBABABA
	WATCH_1_4_LABEL = 0x3A3A3A
	WATCH_1_4_LIGHT_LABEL = 0x5A5A5A
	WATCH_2_4_LABEL = 0x5A5A5A
	WATCH_2_4_LIGHT_LABEL = 0x7A7A7A
	WATCH_3_4_LABEL = 0x7A7A7A
	WATCH_3_4_LIGHT_LABEL = 0x9A9A9A
	WATCH_4_4_LABEL = 0x9A9A9A
	WATCH_4_4_LIGHT_LABEL = 0xBABABA

	def __init__(self, size=240):
		self.size = size
		self.MOON_RING_RADIUS = 53 / 240 * size
		self.MOON_RING_THICKNESS = 3 / 240 * size
		self.SUN_RING_RADIUS = 59 / 240 * size
		self.SUN_RING_THICKNESS = 3 / 240 * size
		self.HALAKHIC_RING_RADIUS = 66 / 240 * size
		self.HALAKHIC_RING_THICKNESS = 6 / 240 * size
		self.OBSERVANCE_RING_RADIUS = 75 / 240 * size
		self.OBSERVANCE_RING_THICKNESS = 6 / 240 * size
		self.DIAL_RADIUS = 100 / 240 * size
		self.HOUR_SHORT_TICK_LENGTH = 10 / 240 * size
		self.HOUR_MEDIUM_TICK_LENGTH = 16 / 240 * size
		self.HOUR_LONG_TICK_LENGTH = 20 / 240 * size
		self.CAP_RADIUS = 3 / 240 * size
		self.RIM_THICKNESS = 2 / 240 * size
		self.HOUR_HAND_LENGTH = 60 / 240 * size
		self.HOUR_HAND_TAIL = 10 / 240 * size
		self.HOUR_HAND_THICKNESS = 4 / 240 * size
		self.SECOND_HAND_THICKNESS = 1 / 240 * size

	def _draw_tick(self, d, cx, cy, angleDeg, length, thickness, color):
		rad = math.radians(angleDeg)
		xOut = cx + int(self.DIAL_RADIUS * math.sin(rad))
		yOut = cy - int(self.DIAL_RADIUS * math.cos(rad))
		innerRadius = self.DIAL_RADIUS - length
		xIn  = cx + int(innerRadius * math.sin(rad))
		yIn  = cy - int(innerRadius * math.cos(rad))
		d.drawLine(xIn, yIn, xOut, yOut, thickness, color)

	def _draw_hand(self, d, cx, cy, length, tail, angleDeg, thickness, color):
		rad = math.radians(angleDeg)
		tipX = cx + int(length * math.sin(rad))
		tipY = cy - int(length * math.cos(rad))
		tailX = cx - int(tail * math.sin(rad))
		tailY = cy + int(tail * math.cos(rad))
		d.drawLine(tailX, tailY, tipX, tipY, thickness, color)

	def _draw_face(self, d, cx, cy):
		d.drawCircle(cx, cy, self.CAP_RADIUS, self.CAP_RADIUS * 2, self.FACE_COLOR)
		d.drawCircle(cx, cy, self.DIAL_RADIUS, self.RIM_THICKNESS, self.FACE_COLOR)

	def draw(self, d):
		raise NotImplementedError


class StandardClock(Clock):
	FONT_ASSISTANT = "Assistant-Regular.ttf"
	MINUTE_TICK_COLOR = 0xAAAAAA
	RING_TRACK_COLOR = 0xDCDCDC
	SEAM_GAP_DEGREES = 30.0

	def __init__(self, datetime, location, custom, size=240):
		super().__init__(size)
		self.LABEL_RADIUS = self.DIAL_RADIUS + 8 / 240 * size
		self.FONT_SIZE = int(round(18 / 240 * size))
		self.MINUTE_TICK_LENGTH = 6 / 240 * size
		self.MINUTE_HAND_LENGTH = 84 / 240 * size
		self.MINUTE_HAND_TAIL = 16 / 240 * size
		self.MINUTE_HAND_THICKNESS = 2 / 240 * size
		self.SECOND_HAND_LENGTH = 96 / 240 * size
		self.SECOND_HAND_TAIL = 24 / 240 * size
		self.location = location
		self.custom = custom
		self.localHebrewDate = None
		self._lastHebrewDate = None
		self.face = None
		self.update(datetime)

	def update(self, datetime):
		self.datetime = datetime
		hebrewDate = HebrewDate.fromdate(datetime.date)
		if self.localHebrewDate is None or hebrewDate != self.localHebrewDate.hebrewDate:
			self.localHebrewDate = LocalHebrewDate(hebrewDate, self.location, self.custom)
		if hebrewDate != self._lastHebrewDate:
			self.drawHalakhicBands()
			self._lastHebrewDate = hebrewDate


	@property
	def hourHandAngle(self):
		t = self.datetime.time
		totalHours = t.hour % 12 + t.minute / 60.0 + t.second / 3600.0
		return totalHours * 30.0

	@property
	def minuteHandAngle(self):
		t = self.datetime.time
		return t.minute * 6.0 + t.second * 0.1

	@property
	def secondHandAngle(self):
		t = self.datetime.time
		return t.second * 6.0

	@property
	def hourMajorMarks(self):
		return [h * 30.0 for h in range(0, 12, 3)]

	@property
	def hourMarks(self):
		return [h * 30.0 for h in range(12) if h % 3 != 0]

	@property
	def minuteMarks(self):
		return [m * 6.0 for m in range(60) if m % 5 != 0]

	@property
	def windowStart(self):
		return self.datetime + (-12 * 3600)

	@property
	def windowEnd(self):
		return self.datetime + (12 * 3600)

	@property
	def halakhicBands(self):
		current = self.localHebrewDate
		yemamaStart = current.yemamaStart + TIMEZONE
		yemamaEnd = current.yemamaEnd + TIMEZONE
		yemamot = []
		if self.windowStart < yemamaStart:
			yemamot.append(current.previousYemama)
		yemamot.append(current)
		if self.windowEnd > yemamaEnd:
			yemamot.append(current.nextYemama)
		all_bands = []
		for y in yemamot:
			all_bands.extend(y.halakhicBands)
		return all_bands

	@property
	def visualHalakhicBands(self):
		now = self.datetime
		result = []
		for b in self.halakhicBands:
			startLocal = b.start.datetime + TIMEZONE
			endLocal = b.end.datetime + TIMEZONE
			startSec = (startLocal.date.ordinal - now.date.ordinal) * 86400 + \
					   (startLocal.time.serial - now.time.serial)
			endSec = (endLocal.date.ordinal - now.date.ordinal) * 86400 + \
					 (endLocal.time.serial - now.time.serial)
			startAngle = startSec / 120.0
			endAngle = endSec / 120.0
			result.append(VisualBand(startAngle, endAngle, self._halakhicColor(b)))
		return result

	@property
	def sunBands(self):
		current = self.localHebrewDate
		yemamot = [current.previousYemama, current, current.nextYemama]
		all_bands = []
		for y in yemamot:
			all_bands.extend(y.sunBands)
		return all_bands

	@property
	def visualSunBands(self):
		now = self.datetime
		result = []
		for b in self.sunBands:
			startLocal = b.start.datetime + TIMEZONE
			endLocal = b.end.datetime + TIMEZONE
			startSec = (startLocal.date.ordinal - now.date.ordinal) * 86400 + \
					   (startLocal.time.serial - now.time.serial)
			endSec = (endLocal.date.ordinal - now.date.ordinal) * 86400 + \
					 (endLocal.time.serial - now.time.serial)
			startAngle = startSec / 120.0
			endAngle = endSec / 120.0
			result.append(VisualBand(startAngle, endAngle, self._sunColor(b)))
		return result

	@property
	def moonBands(self):
		current = self.localHebrewDate
		yemamot = [current.previousYemama, current, current.nextYemama]
		all_bands = []
		for y in yemamot:
			all_bands.extend(y.moonBands)
		return all_bands

	@property
	def visualMoonBands(self):
		now = self.datetime
		result = []
		for b in self.moonBands:
			startLocal = b.start.datetime + TIMEZONE
			endLocal = b.end.datetime + TIMEZONE
			startSec = (startLocal.date.ordinal - now.date.ordinal) * 86400 + \
					   (startLocal.time.serial - now.time.serial)
			endSec = (endLocal.date.ordinal - now.date.ordinal) * 86400 + \
					 (endLocal.time.serial - now.time.serial)
			startAngle = startSec / 120.0
			endAngle = endSec / 120.0
			result.append(VisualBand(startAngle, endAngle, self._moonColor(b)))
		return result

	@property
	def observanceBands(self):
		current = self.localHebrewDate
		yemamot = [current.previousYemama, current, current.nextYemama]
		all_bands = []
		for y in yemamot:
			all_bands.extend(y.observanceBands)
		return all_bands

	@property
	def visibleObservanceBands(self):
		now = self.datetime
		result = []
		for b in self.observanceBands:
			startLocal = b.start.datetime + TIMEZONE
			endLocal = b.end.datetime + TIMEZONE
			startSec = (startLocal.date.ordinal - now.date.ordinal) * 86400 + \
					   (startLocal.time.serial - now.time.serial)
			endSec = (endLocal.date.ordinal - now.date.ordinal) * 86400 + \
					 (endLocal.time.serial - now.time.serial)
			startAngle = startSec / 120.0
			endAngle = endSec / 120.0
			result.append(VisualBand(startAngle, endAngle, self._observanceColor(b), b.stripe))
		return result

	@property
	def visibleSeasonalHourMarks(self):
		now = self.datetime
		current = self.localHebrewDate
		yemamot = [current.previousYemama, current, current.nextYemama]
		lo = -180.0 + self.SEAM_GAP_DEGREES / 2.0
		hi = 180.0 - self.SEAM_GAP_DEGREES / 2.0
		marks = []
		for y in yemamot:
			for dt in y.hours:
				local_dt = dt + TIMEZONE
				delta_sec = ((local_dt.date.ordinal - now.date.ordinal) * 86400 +
						     (local_dt.time.serial - now.time.serial))
				angle = delta_sec / 120.0
				if lo <= angle <= hi:
					marks.append(angle)
		return marks

	def drawVisibleSunBands(self):
		d = self.d
		cx = self._cx
		cy = self._cy
		hand_angle = self.hourHandAngle
		lo = -180.0 + self.SEAM_GAP_DEGREES / 2.0
		hi = 180.0 - self.SEAM_GAP_DEGREES / 2.0
		for vb in self.visualSunBands:
			clippedStart = max(vb.startAngle, lo)
			clippedEnd = min(vb.endAngle, hi)
			if clippedStart >= clippedEnd:
				continue
			abs_start = (clippedStart + hand_angle) % 360.0
			abs_end   = (clippedEnd + hand_angle) % 360.0
			pil_start = abs_start - 90.0
			pil_end   = abs_end - 90.0
			if pil_end < pil_start:
				pil_end += 360.0
			if pil_start == pil_end:
				continue
			d.drawArc(cx, cy, self.SUN_RING_RADIUS, pil_start, pil_end,
					  self.SUN_RING_THICKNESS, vb.color)

	def drawVisibleHalakhicBands(self):
		d = self.d
		cx = self._cx
		cy = self._cy
		hand_angle = self.hourHandAngle
		lo = -180.0 + self.SEAM_GAP_DEGREES / 2.0
		hi = 180.0 - self.SEAM_GAP_DEGREES / 2.0
		trackStart = (lo + hand_angle) % 360.0
		trackEnd = (hi + hand_angle) % 360.0
		pil_track_start = trackStart - 90.0
		pil_track_end = trackEnd - 90.0
		if pil_track_end < pil_track_start:
			pil_track_end += 360.0
		d.drawArc(cx, cy, self.HALAKHIC_RING_RADIUS, pil_track_start, pil_track_end,
				  self.HALAKHIC_RING_THICKNESS, self.RING_TRACK_COLOR)
		for vb in self.visualHalakhicBands:
			clippedStart = max(vb.startAngle, lo)
			clippedEnd = min(vb.endAngle, hi)
			if clippedStart >= clippedEnd:
				continue
			abs_start = (clippedStart + hand_angle) % 360.0
			abs_end   = (clippedEnd + hand_angle) % 360.0
			pil_start = abs_start - 90.0
			pil_end   = abs_end - 90.0
			if pil_end < pil_start:
				pil_end += 360.0
			if pil_start == pil_end:
				continue
			d.drawArc(cx, cy, self.HALAKHIC_RING_RADIUS, pil_start, pil_end,
					  self.HALAKHIC_RING_THICKNESS, vb.color)
		half_thick = self.HALAKHIC_RING_THICKNESS / 2.0
		r_inner = self.HALAKHIC_RING_RADIUS - half_thick
		r_outer = self.HALAKHIC_RING_RADIUS + half_thick
		for angle in self.visibleSeasonalHourMarks:
			abs_angle = (angle + hand_angle) % 360.0
			rad = math.radians(abs_angle - 90.0)
			x1 = cx + int(r_inner * math.cos(rad))
			y1 = cy + int(r_inner * math.sin(rad))
			x2 = cx + int(r_outer * math.cos(rad))
			y2 = cy + int(r_outer * math.sin(rad))
			d.drawLine(x1, y1, x2, y2, 1, 0xFFFFFF)

	def drawVisibleMoonBands(self):
		d = self.d
		cx = self._cx
		cy = self._cy
		hand_angle = self.hourHandAngle
		lo = -180.0 + self.SEAM_GAP_DEGREES / 2.0
		hi = 180.0 - self.SEAM_GAP_DEGREES / 2.0
		for vb in self.visualMoonBands:
			clippedStart = max(vb.startAngle, lo)
			clippedEnd = min(vb.endAngle, hi)
			if clippedStart >= clippedEnd:
				continue
			abs_start = (clippedStart + hand_angle) % 360.0
			abs_end   = (clippedEnd + hand_angle) % 360.0
			pil_start = abs_start - 90.0
			pil_end   = abs_end - 90.0
			if pil_end < pil_start:
				pil_end += 360.0
			if pil_start == pil_end:
				continue
			d.drawArc(cx, cy, self.MOON_RING_RADIUS, pil_start, pil_end,
					  self.MOON_RING_THICKNESS, vb.color)

	def drawVisibleObservanceBands(self):
		d = self.d
		cx = self._cx
		cy = self._cy
		hand_angle = self.hourHandAngle
		lo = -180.0 + self.SEAM_GAP_DEGREES / 2.0
		hi = 180.0 - self.SEAM_GAP_DEGREES / 2.0
		for vb in self.visibleObservanceBands:
			clippedStart = max(vb.startAngle, lo)
			clippedEnd = min(vb.endAngle, hi)
			if clippedStart >= clippedEnd:
				continue
			abs_start = (clippedStart + hand_angle) % 360.0
			abs_end   = (clippedEnd + hand_angle) % 360.0
			pil_start = abs_start - 90.0
			pil_end   = abs_end - 90.0
			if pil_end < pil_start:
				pil_end += 360.0
			if pil_start == pil_end:
				continue
			d.drawArc(cx, cy, self.OBSERVANCE_RING_RADIUS, pil_start, pil_end,
					self.OBSERVANCE_RING_THICKNESS, vb.color, stripe=vb.stripe)

	def drawHalakhicBands(self):
		bands = self.localHebrewDate.halakhicBands
		datetimes = []
		for b in bands:
			start = b.start.datetime + TIMEZONE
			end = b.end.datetime + TIMEZONE
			datetimes.append((start, end, self._halakhicColor(b)))

		if not datetimes:
			return []

		min_idx = None
		max_idx = None
		for s, e, c in datetimes:
			for dt in (s, e):
				idx = dt.date.ordinal * 2 + (0 if dt.time.serial < 43200 else 1)
				if min_idx is None or idx < min_idx:
					min_idx = idx
				if max_idx is None or idx > max_idx:
					max_idx = idx

		canvases = []
		for _ in range(min_idx, max_idx + 1):
			canvas = Display(self.size)
			canvas.fill(self.BACKGROUND_COLOR)
			canvases.append(canvas)

		cx = cy = self.size // 2
		for s, e, color in datetimes:
			current = s
			while current < e:
				if current.time.serial < 43200:
					seg_end = DateTime(current.date, Time(12, 0, 0))
				else:
					seg_end = DateTime(Date.fromordinal(current.date.ordinal + 1), Time(0, 0, 0))
				piece_end = min(e, seg_end)
				if piece_end > current:
					seg_idx = current.date.ordinal * 2 + (0 if current.time.serial < 43200 else 1)
					canvas_idx = seg_idx - min_idx
					canvas = canvases[canvas_idx]

					if current.time.serial < 43200:
						seg_start_seconds = 0
					else:
						seg_start_seconds = 43200
					start_seconds = current.time.serial - seg_start_seconds
					end_seconds = piece_end.time.serial - seg_start_seconds
					start_angle = start_seconds / 120.0
					end_angle = end_seconds / 120.0
					pil_start = start_angle - 90.0
					pil_end = end_angle - 90.0
					if pil_end <= pil_start:
						pil_end += 360.0
					if pil_start == pil_end:
						continue
					canvas.drawArc(cx, cy, self.HALAKHIC_RING_RADIUS,
								   pil_start, pil_end,
								   self.HALAKHIC_RING_THICKNESS, color)
				current = piece_end
		return canvases

	def drawDigits(self):
		d = self.d
		cx = self._cx
		cy = self._cy
		hourHand = self.hourHandAngle
		opposite = (hourHand + 180.0) % 360.0
		gap = self.SEAM_GAP_DEGREES / 2.0
		for mark in range(12):
			markAngle = mark * 30.0
			diff = (markAngle - opposite + 180.0) % 360.0 - 180.0
			if abs(diff) < gap:
				continue
			delta = markAngle - hourHand
			if delta > 180.0:
				delta -= 360.0
			elif delta <= -180.0:
				delta += 360.0
			delta_sec = delta * 120
			total_sec = (self.datetime.time.serial + delta * 120) % 86400
			hour24 = int(round(total_sec / 3600.0)) % 24
			digit = str(hour24)
			posAngle = markAngle - 90.0
			if markAngle == 90.0:
				rotation = 90.0
			elif markAngle == 270.0:
				rotation = -90.0
			elif 90.0 < markAngle < 270.0:
				rotation = 180.0
			else:
				rotation = 0.0
			d.drawCurvedChar(cx, cy, self.LABEL_RADIUS, posAngle,
			                 digit, self.FONT_ASSISTANT, self.FONT_SIZE, rotation)


	def drawMarks(self):
		d = self.d
		cx = self._cx
		cy = self._cy
		for angleDeg in self.minuteMarks:
			self._draw_tick(d, cx, cy, angleDeg, self.MINUTE_TICK_LENGTH, 1, self.MINUTE_TICK_COLOR)
		for angleDeg in self.hourMarks:
			self._draw_tick(d, cx, cy, angleDeg, self.HOUR_SHORT_TICK_LENGTH, 2, self.TICK_COLOR)
		for angleDeg in self.hourMajorMarks:
			self._draw_tick(d, cx, cy, angleDeg, self.HOUR_LONG_TICK_LENGTH, 2, self.TICK_COLOR)

	def drawHands(self):
		d = self.d
		cx = self._cx
		cy = self._cy
		self._draw_hand(d, cx, cy, self.HOUR_HAND_LENGTH, self.HOUR_HAND_TAIL,
						self.hourHandAngle, self.HOUR_HAND_THICKNESS, self.HAND_COLOR)
		self._draw_hand(d, cx, cy, self.MINUTE_HAND_LENGTH, self.MINUTE_HAND_TAIL,
						self.minuteHandAngle, self.MINUTE_HAND_THICKNESS, self.HAND_COLOR)

	def drawFace(self):
		self._draw_face(self.d, self._cx, self._cy)

	def draw(self, d):
		self.d = d
		self._cx = d.WIDTH // 2
		self._cy = d.HEIGHT // 2
		if self.face is None:
			temp = Display(self.size)
			temp.fillCircle(self._cx, self._cy, self.DIAL_RADIUS, self.BACKGROUND_COLOR)
			old_d = self.d
			self.d = temp
			self.drawMarks()
			self.d = old_d
			self.face = bytes(temp.buffer)
		d.buffer[:] = self.face
		self.drawVisibleSunBands()
		self.drawVisibleMoonBands()
		self.drawVisibleHalakhicBands()
		self.drawVisibleObservanceBands()
		self.drawDigits()
		self.drawHands()
		self.drawFace()


class HebrewClock(Clock):
	def __init__(self, datetime, location, custom, size=240):
		super().__init__(size)
		self.SECOND_HAND_LENGTH = 28 / 240 * size
		self.SECOND_HAND_TAIL = 12 / 240 * size
		self.location = location
		self.custom = custom
		self.localHebrewDate = None
		self.face = None
		self.update(datetime)

	def update(self, datetime):
		self.datetime = datetime
		hebrewDate = HebrewDate.fromdate(datetime.date)
		if self.localHebrewDate is None or hebrewDate != self.localHebrewDate.hebrewDate:
			self.localHebrewDate = LocalHebrewDate(hebrewDate, self.location, self.custom)

	@property
	def _utcTime(self):
		return (self.datetime + (-TIMEZONE)).time

	@property
	def hourHandAngle(self):
		current = self.localHebrewDate
		h = current.hour(self._utcTime)
		return current.angle(h)

	@property
	def secondHandAngle(self):
		current = self.localHebrewDate
		h = current.hour(self._utcTime)
		hourFrac = h - int(h)
		minuteFrac = (hourFrac * 60.0) % 1.0
		return minuteFrac * 360.0

	@property
	def hourMajorMarks(self):
		current = self.localHebrewDate
		return [current.angle(h) for h in range(0, 24, 6)]

	@property
	def hourMiddleMarks(self):
		current = self.localHebrewDate
		return [current.angle(h) for h in range(3, 24, 6)]

	@property
	def hourMarks(self):
		current = self.localHebrewDate
		return [current.angle(h) for h in range(24) if h % 3 != 0]

	@property
	def halakhicBands(self):
		current = self.localHebrewDate
		h = current.hour(self._utcTime)
		prev = current.previousYemama
		next = current.nextYemama
		all_bands = []
		all_bands.extend(prev.halakhicBands)
		all_bands.extend(current.halakhicBands)
		all_bands.extend(next.halakhicBands)
		return all_bands

	@property
	def visualHalakhicBands(self):
		current = self.localHebrewDate
		currentHour = current.hour(self._utcTime)
		result = []
		for b in self.halakhicBands:
			start = b.start.localHebrewDateTime
			end   = b.end.localHebrewDateTime
			startAngle = start.localHebrewDate.angle(start.hour)
			endAngle   = end.localHebrewDate.angle(end.hour)
			result.append(VisualBand(startAngle, endAngle, self._halakhicColor(b)))
		return result

	@property
	def moonBands(self):
		current = self.localHebrewDate
		h = current.hour(self._utcTime)
		prev = current.previousYemama
		next = current.nextYemama
		all_bands = []
		all_bands.extend(prev.moonBands)
		all_bands.extend(current.moonBands)
		all_bands.extend(next.moonBands)
		return all_bands

	@property
	def visualMoonBands(self):
		current = self.localHebrewDate
		currentHour = current.hour(self._utcTime)
		result = []
		for b in self.moonBands:
			start = b.start.localHebrewDateTime
			end   = b.end.localHebrewDateTime
			startAngle = start.localHebrewDate.angle(start.hour)
			endAngle   = end.localHebrewDate.angle(end.hour)
			result.append(VisualBand(startAngle, endAngle, self._moonColor(b)))
		return result

	@property
	def observanceBands(self):
		current = self.localHebrewDate
		h = current.hour(self._utcTime)
		prev = current.previousYemama
		next = current.nextYemama
		all_bands = []
		all_bands.extend(prev.observanceBands)
		all_bands.extend(current.observanceBands)
		all_bands.extend(next.observanceBands)
		return all_bands

	@property
	def visibleObservanceBands(self):
		current = self.localHebrewDate
		currentHour = current.hour(self._utcTime)
		result = []
		for b in self.observanceBands:
			start = b.start.localHebrewDateTime
			end   = b.end.localHebrewDateTime
			startAngle = start.localHebrewDate.angle(start.hour)
			endAngle   = end.localHebrewDate.angle(end.hour)
			result.append(VisualBand(startAngle, endAngle, self._observanceColor(b), b.stripe))
		return result

	def drawVisibleHalakhicBands(self):
		d = self.d
		cx = self._cx
		cy = self._cy
		for vb in self.visualHalakhicBands:
			a = vb.startAngle % 360.0
			b = vb.endAngle % 360.0
			if a == b:
				continue
			pil_start = a - 90.0
			pil_end   = b - 90.0
			if pil_end < pil_start:
				pil_end += 360.0
			d.drawArc(cx, cy, self.HALAKHIC_RING_RADIUS, pil_start, pil_end,
					self.HALAKHIC_RING_THICKNESS, vb.color)

	def drawVisibleObservanceBands(self):
		d = self.d
		cx = self._cx
		cy = self._cy
		for vb in self.visibleObservanceBands:
			a = vb.startAngle % 360.0
			b = vb.endAngle % 360.0
			if a == b:
				continue
			pil_start = a - 90.0
			pil_end   = b - 90.0
			if pil_end < pil_start:
				pil_end += 360.0
			d.drawArc(cx, cy, self.OBSERVANCE_RING_RADIUS, pil_start, pil_end,
					 self.OBSERVANCE_RING_THICKNESS, vb.color, stripe=vb.stripe)

	def drawVisibleMoonBands(self):
		d = self.d
		cx = self._cx
		cy = self._cy
		for vb in self.visualMoonBands:
			a = vb.startAngle % 360.0
			b = vb.endAngle % 360.0
			if a == b:
				continue
			pil_start = a - 90.0
			pil_end   = b - 90.0
			if pil_end < pil_start:
				pil_end += 360.0
			d.drawArc(cx, cy, self.MOON_RING_RADIUS, pil_start, pil_end,
					  self.MOON_RING_THICKNESS, vb.color)

	def drawMarks(self):
		d = self.d
		cx = self._cx
		cy = self._cy
		for angleDeg in self.hourMarks:
			self._draw_tick(d, cx, cy, angleDeg, self.HOUR_SHORT_TICK_LENGTH, 2, self.TICK_COLOR)
		for angleDeg in self.hourMiddleMarks:
			self._draw_tick(d, cx, cy, angleDeg, self.HOUR_MEDIUM_TICK_LENGTH, 2, self.TICK_COLOR)
		for angleDeg in self.hourMajorMarks:
			self._draw_tick(d, cx, cy, angleDeg, self.HOUR_LONG_TICK_LENGTH, 2, self.TICK_COLOR)

	def drawHands(self):
		d = self.d
		cx = self._cx
		cy = self._cy
		self._draw_hand(d, cx, cy, self.HOUR_HAND_LENGTH, self.HOUR_HAND_TAIL,
						self.hourHandAngle, self.HOUR_HAND_THICKNESS, self.HAND_COLOR)
		self._draw_hand(d, cx, cy, self.SECOND_HAND_LENGTH, self.SECOND_HAND_TAIL,
						self.secondHandAngle, self.SECOND_HAND_THICKNESS, self.SECOND_HAND_COLOR)

	def drawFace(self):
		self._draw_face(self.d, self._cx, self._cy)

	def draw(self, d):
		self.d = d
		self._cx = d.WIDTH // 2
		self._cy = d.HEIGHT // 2
		if self.face is None:
			temp = Display(self.size)
			temp.fillCircle(self._cx, self._cy, self.DIAL_RADIUS, self.BACKGROUND_COLOR)
			old_d = self.d
			self.d = temp
			self.drawMarks()
			self.d = old_d
			self.face = bytes(temp.buffer)
		d.buffer[:] = self.face
		self.drawVisibleMoonBands()
		self.drawVisibleHalakhicBands()
		self.drawVisibleObservanceBands()
		self.drawHands()
		self.drawFace()


if __name__ == "__main__":
	import time as _systime
	from Date import Date, Time
	from HebrewDate import HebrewDate
	from Geo import Place

	now = _systime.localtime()
	dt = DateTime(Date(now.tm_year, now.tm_mon, now.tm_mday),
				   Time(now.tm_hour, now.tm_min, now.tm_sec))
	location = Place.JERUSALEM.location()

	for preset, label in [(Custom.LOCAL, "LOCAL (MA, 16.1°, 50min, 8.5°)"),
						   (Custom.ASHKENAZI_GRA, "ASHKENAZI_GRA (GRA, 16.1°, 11.5°, 8.5°)")]:
		custom = Custom(preset)
		print(f"Custom: {label}")
		print()
		sc = StandardClock(dt, location, custom)
		print("StandardClock halakhic bands (civil window):")
		print(f"  Window: {sc.windowStart}  →  {sc.windowEnd}")
		for b in sc.halakhicBands:
			print(f"  {HalakhicBandNames('en', b.id).name():20s}  {b.start.datetime}  →  {b.end.datetime}  color=0x{sc._halakhicColor(b):06X}")
		print()
		print("StandardClock visual halakhic bands (angles):")
		for vb in sc.visualHalakhicBands:
			print(f"  {vb.startAngle:8.2f}° → {vb.endAngle:8.2f}°  color=0x{vb.color:06X}")
		print()
		print("StandardClock sun bands (civil window):")
		for b in sc.sunBands:
			print(f"  {'sun':20s}  {b.start.datetime}  →  {b.end.datetime}  color=0x{sc._sunColor(b):06X}")
		print()
		print("StandardClock visual sun bands (angles):")
		for vb in sc.visualSunBands:
			print(f"  {vb.startAngle:8.2f}° → {vb.endAngle:8.2f}°  color=0x{vb.color:06X}")
		print()
		hc = HebrewClock(dt, location, custom)
		print("HebrewClock bands (Hebrew window):")
		current = hc.localHebrewDate
		h = current.hour(hc._utcTime)
		prev = current.previousYemama
		next = current.nextYemama
		win_start = (prev.hebrewDate.ordinal, h)
		win_end   = (next.hebrewDate.ordinal, h)
		print(f"  Current Hebrew hour: {h:.3f}  (date {current.hebrewDate})")
		print(f"  Window: ({prev.hebrewDate}, {h:.3f})  →  ({next.hebrewDate}, {h:.3f})")
		for b in hc.halakhicBands:
			start = b.start.localHebrewDateTime
			end   = b.end.localHebrewDateTime
			print(f"  {HalakhicBandNames('en', b.id).name():20s}  ({start.localHebrewDate.hebrewDate}, {start.hour:.3f})  →  ({end.localHebrewDate.hebrewDate}, {end.hour:.3f})  color=0x{hc._halakhicColor(b):06X}")
		print()
		print("HebrewClock visual bands (angles):")
		for vb in hc.visualHalakhicBands:
			print(f"  {vb.startAngle:8.2f}° → {vb.endAngle:8.2f}°  color=0x{vb.color:06X}")
		print()
		print("Moon bands (Hebrew window):")
		for b in hc.moonBands:
			start = b.start.localHebrewDateTime
			end   = b.end.localHebrewDateTime
			print(f"  moon band  ({start.localHebrewDate.hebrewDate}, {start.hour:.3f})  →  ({end.localHebrewDate.hebrewDate}, {end.hour:.3f})  color=0x{hc._moonColor(b):06X}")
		print()

	for preset, name in [(Custom.LOCAL, "local"), (Custom.ASHKENAZI_GRA, "gra")]:
		custom = Custom(preset)
		print(f"Generating images for {name}...")
		sc = StandardClock(dt, location, custom)
		std_disp = Display()
		sc.draw(std_disp)
		std_disp.save(f"standard_{name}.png")
		hc = HebrewClock(dt, location, custom)
		heb_disp = Display()
		hc.draw(heb_disp)
		heb_disp.save(f"hebrew_{name}.png")
	print("Images saved using C++ style rendering.")