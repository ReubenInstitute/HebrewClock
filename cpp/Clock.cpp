#include "Clock.h"

#include <algorithm>
#include <math.h>
#include <string>

namespace {
const long TIMEZONE = 3 * 3600;
}

/* ---------- VisualBand ---------- */

VisualBand::VisualBand(double startAngle, double endAngle, uint32_t color, bool stripe)
	: startAngle(startAngle), endAngle(endAngle), color(color), stripe(stripe) {}

/* ---------- Clock ---------- */

uint32_t Clock::_halakhicColor(const Band& band) {
	switch (static_cast<BandId>(band.id)) {
		case BandId::NIGHTFALL: return 0x8A6A4A;
		case BandId::DAWN: return 0x375C7B;
		case BandId::MISHEYAKIR: return 0x567D9F;
		case BandId::SHEMA: return 0x829DB5;
		case BandId::AMIDAH: return 0xAEBECB;
		case BandId::MINCHA_GEDOLAH: return 0x7A8C6D;
		case BandId::MINCHA_KETANA: return 0xCBB88C;
		case BandId::PLAG_MINCHA: return 0xB08A5C;
		case BandId::WATCH1: return 0x9A9A9A;
		case BandId::WATCH2: return 0x1A1A1A;
		case BandId::WATCH3: return 0x9A9A9A;
		default: return 0x000000;
	}
}

uint32_t Clock::_observanceColor(const Band& band) {
	switch (static_cast<BandId>(band.id)) {
		case BandId::REST: return 0xA05050;
		case BandId::REST_STRIPED: return 0xA05050;
		case BandId::YOMTOV: return 0xD07070;
		case BandId::FAST: return 0xE6C300;
		case BandId::FAST_STRIPED: return 0xE6C300;
		default: return 0x000000;
	}
}

uint32_t Clock::_moonColor(const Band& band) {
	switch (static_cast<MoonPhaseType>(band.id)) {
		case MoonPhaseType::NEW_MOON: return static_cast<uint32_t>(MoonPhaseColor::NEW_MOON);
		case MoonPhaseType::WAXING_CRESCENT: return static_cast<uint32_t>(MoonPhaseColor::WAXING_CRESCENT);
		case MoonPhaseType::FIRST_QUARTER: return static_cast<uint32_t>(MoonPhaseColor::FIRST_QUARTER);
		case MoonPhaseType::WAXING_GIBBOUS: return static_cast<uint32_t>(MoonPhaseColor::WAXING_GIBBOUS);
		case MoonPhaseType::FULL_MOON: return static_cast<uint32_t>(MoonPhaseColor::FULL_MOON);
		case MoonPhaseType::WANING_GIBBOUS: return static_cast<uint32_t>(MoonPhaseColor::WANING_GIBBOUS);
		case MoonPhaseType::LAST_QUARTER: return static_cast<uint32_t>(MoonPhaseColor::LAST_QUARTER);
		case MoonPhaseType::WANING_CRESCENT: return static_cast<uint32_t>(MoonPhaseColor::WANING_CRESCENT);
		default: return 0x000000;
	}
}

uint32_t Clock::_sunColor(const Band& band) {
	switch (static_cast<BandId>(band.id)) {
		case BandId::NIGHT: return 0x4A4A55;
		case BandId::DAY: return 0xFFD700;
		default: return 0x000000;
	}
}

Clock::Clock(int size) : size(size) {
	MOON_RING_RADIUS = 53.0 / 240.0 * size;
	MOON_RING_THICKNESS = 3.0 / 240.0 * size;
	SUN_RING_RADIUS = 59.0 / 240.0 * size;
	SUN_RING_THICKNESS = 3.0 / 240.0 * size;
	HALAKHIC_RING_RADIUS = 66.0 / 240.0 * size;
	HALAKHIC_RING_THICKNESS = 6.0 / 240.0 * size;
	OBSERVANCE_RING_RADIUS = 75.0 / 240.0 * size;
	OBSERVANCE_RING_THICKNESS = 6.0 / 240.0 * size;
	DIAL_RADIUS = 100.0 / 240.0 * size;
	HOUR_SHORT_TICK_LENGTH = 10.0 / 240.0 * size;
	HOUR_MEDIUM_TICK_LENGTH = 16.0 / 240.0 * size;
	HOUR_LONG_TICK_LENGTH = 20.0 / 240.0 * size;
	CAP_RADIUS = 3.0 / 240.0 * size;
	RIM_THICKNESS = 2.0 / 240.0 * size;
	HOUR_HAND_LENGTH = 60.0 / 240.0 * size;
	HOUR_HAND_TAIL = 10.0 / 240.0 * size;
	HOUR_HAND_THICKNESS = 4.0 / 240.0 * size;
	SECOND_HAND_THICKNESS = 1.0 / 240.0 * size;
}

void Clock::_draw_tick(Display& d, int cx, int cy, double angleDeg, double length, int thickness, uint32_t color) {
	double rad = angleDeg * M_PI / 180.0;
	int xOut = cx + (int)(DIAL_RADIUS * sin(rad));
	int yOut = cy - (int)(DIAL_RADIUS * cos(rad));
	double innerRadius = DIAL_RADIUS - length;
	int xIn = cx + (int)(innerRadius * sin(rad));
	int yIn = cy - (int)(innerRadius * cos(rad));
	d.drawLine(xIn, yIn, xOut, yOut, thickness, color);
}

void Clock::_draw_hand(Display& d, int cx, int cy, double length, double tail, double angleDeg, int thickness, uint32_t color) {
	double rad = angleDeg * M_PI / 180.0;
	int tipX = cx + (int)(length * sin(rad));
	int tipY = cy - (int)(length * cos(rad));
	int tailX = cx - (int)(tail * sin(rad));
	int tailY = cy + (int)(tail * cos(rad));
	d.drawLine(tailX, tailY, tipX, tipY, thickness, color);
}

void Clock::_draw_face(Display& d, int cx, int cy) {
	d.drawCircle(cx, cy, CAP_RADIUS, (int)(CAP_RADIUS * 2), FACE_COLOR);
	d.drawCircle(cx, cy, DIAL_RADIUS, (int)RIM_THICKNESS, FACE_COLOR);
}

/* ---------- StandardClock ---------- */

const char* const StandardClock::FONT_ASSISTANT = "Assistant-Regular.ttf";

StandardClock::StandardClock(DateTime datetime, Location location, Custom custom, int size)
	: Clock(size), location(location), custom(custom),
	  hasLastHebrewDate(false), faceComputed(false), d(nullptr), _cx(0), _cy(0) {
	LABEL_RADIUS = DIAL_RADIUS + 8.0 / 240.0 * size;
	FONT_SIZE = (int)lround(18.0 / 240.0 * size);
	MINUTE_TICK_LENGTH = 6.0 / 240.0 * size;
	MINUTE_HAND_LENGTH = 84.0 / 240.0 * size;
	MINUTE_HAND_TAIL = 16.0 / 240.0 * size;
	MINUTE_HAND_THICKNESS = 2.0 / 240.0 * size;
	SECOND_HAND_LENGTH = 96.0 / 240.0 * size;
	SECOND_HAND_TAIL = 24.0 / 240.0 * size;
	update(datetime);
}

void StandardClock::update(DateTime newDatetime) {
	datetime = newDatetime;
	HebrewDate hebrewDate = HebrewDate::fromdate(datetime.date);
	if (!localHebrewDate || hebrewDate != localHebrewDate->hebrewDate) {
		localHebrewDate.reset(new LocalHebrewDate(hebrewDate, location, custom));
	}
	if (!hasLastHebrewDate || hebrewDate != lastHebrewDate) {
		drawHalakhicBands();
		lastHebrewDate = hebrewDate;
		hasLastHebrewDate = true;
	}
}

double StandardClock::hourHandAngle() const {
	const Time& t = datetime.time;
	double totalHours = (t.hour % 12) + t.minute / 60.0 + t.second / 3600.0;
	return totalHours * 30.0;
}

double StandardClock::minuteHandAngle() const {
	const Time& t = datetime.time;
	return t.minute * 6.0 + t.second * 0.1;
}

double StandardClock::secondHandAngle() const {
	const Time& t = datetime.time;
	return t.second * 6.0;
}

std::vector<double> StandardClock::hourMajorMarks() const {
	std::vector<double> marks;
	for (int h = 0; h < 12; h += 3) marks.push_back(h * 30.0);
	return marks;
}

std::vector<double> StandardClock::hourMarks() const {
	std::vector<double> marks;
	for (int h = 0; h < 12; ++h) {
		if (h % 3 != 0) marks.push_back(h * 30.0);
	}
	return marks;
}

std::vector<double> StandardClock::minuteMarks() const {
	std::vector<double> marks;
	for (int m = 0; m < 60; ++m) {
		if (m % 5 != 0) marks.push_back(m * 6.0);
	}
	return marks;
}

DateTime StandardClock::windowStart() const {
	return datetime + (-12L * 3600);
}

DateTime StandardClock::windowEnd() const {
	return datetime + (12L * 3600);
}

std::vector<Band> StandardClock::halakhicBands() const {
	const LocalHebrewDate& current = *localHebrewDate;
	DateTime yemamaStart = current.yemamaStart() + TIMEZONE;
	DateTime yemamaEnd = current.yemamaEnd() + TIMEZONE;
	std::vector<LocalHebrewDate> yemamot;
	if (windowStart() < yemamaStart) {
		yemamot.push_back(current.previousYemama());
	}
	yemamot.push_back(current);
	if (windowEnd() > yemamaEnd) {
		yemamot.push_back(current.nextYemama());
	}
	std::vector<Band> all_bands;
	for (const LocalHebrewDate& y : yemamot) {
		Band arr[LOCAL_HEBREW_DATE_MAX_HALAKHIC_BANDS];
		int count = y.halakhicBands(arr);
		for (int i = 0; i < count; ++i) all_bands.push_back(arr[i]);
	}
	return all_bands;
}

std::vector<VisualBand> StandardClock::_toVisualBands(const std::vector<Band>& bands,
													   uint32_t (*colorFn)(const Band&),
													   bool useStripe) const {
	std::vector<VisualBand> result;
	for (const Band& b : bands) {
		DateTime startLocal = b.start.datetime + TIMEZONE;
		DateTime endLocal = b.end.datetime + TIMEZONE;
		long startSec = (startLocal.date.ordinal - datetime.date.ordinal) * 86400L +
						 (startLocal.time.serial - datetime.time.serial);
		long endSec = (endLocal.date.ordinal - datetime.date.ordinal) * 86400L +
					   (endLocal.time.serial - datetime.time.serial);
		double startAngle = startSec / 120.0;
		double endAngle = endSec / 120.0;
		result.push_back(VisualBand(startAngle, endAngle, colorFn(b), useStripe ? b.stripe : false));
	}
	return result;
}

std::vector<VisualBand> StandardClock::visualHalakhicBands() const {
	return _toVisualBands(halakhicBands(), &Clock::_halakhicColor, false);
}

std::vector<Band> StandardClock::sunBands() const {
	const LocalHebrewDate& current = *localHebrewDate;
	LocalHebrewDate yemamot[3] = {current.previousYemama(), current, current.nextYemama()};
	std::vector<Band> all_bands;
	for (const LocalHebrewDate& y : yemamot) {
		Band arr[LOCAL_HEBREW_DATE_MAX_SUN_BANDS];
		int count = y.sunBands(arr);
		for (int i = 0; i < count; ++i) all_bands.push_back(arr[i]);
	}
	return all_bands;
}

std::vector<VisualBand> StandardClock::visualSunBands() const {
	return _toVisualBands(sunBands(), &Clock::_sunColor, false);
}

std::vector<Band> StandardClock::moonBands() const {
	const LocalHebrewDate& current = *localHebrewDate;
	LocalHebrewDate yemamot[3] = {current.previousYemama(), current, current.nextYemama()};
	std::vector<Band> all_bands;
	for (const LocalHebrewDate& y : yemamot) {
		Band arr[LOCAL_HEBREW_DATE_MAX_MOON_BANDS];
		int count = y.moonBands(arr);
		for (int i = 0; i < count; ++i) all_bands.push_back(arr[i]);
	}
	return all_bands;
}

std::vector<VisualBand> StandardClock::visualMoonBands() const {
	return _toVisualBands(moonBands(), &Clock::_moonColor, false);
}

std::vector<Band> StandardClock::observanceBands() const {
	const LocalHebrewDate& current = *localHebrewDate;
	LocalHebrewDate yemamot[3] = {current.previousYemama(), current, current.nextYemama()};
	std::vector<Band> all_bands;
	for (const LocalHebrewDate& y : yemamot) {
		Band arr[LOCAL_HEBREW_DATE_MAX_OBSERVANCE_BANDS];
		int count = y.observanceBands(arr);
		for (int i = 0; i < count; ++i) all_bands.push_back(arr[i]);
	}
	return all_bands;
}

std::vector<VisualBand> StandardClock::visibleObservanceBands() const {
	return _toVisualBands(observanceBands(), &Clock::_observanceColor, true);
}

std::vector<double> StandardClock::visibleSeasonalHourMarks() const {
	const LocalHebrewDate& current = *localHebrewDate;
	LocalHebrewDate yemamot[3] = {current.previousYemama(), current, current.nextYemama()};
	double lo = -180.0 + SEAM_GAP_DEGREES / 2.0;
	double hi = 180.0 - SEAM_GAP_DEGREES / 2.0;
	std::vector<double> marks;
	for (const LocalHebrewDate& y : yemamot) {
		DateTime hours[24];
		y.hours(hours);
		for (int i = 0; i < 24; ++i) {
			DateTime local_dt = hours[i] + TIMEZONE;
			long delta_sec = (local_dt.date.ordinal - datetime.date.ordinal) * 86400L +
							  (local_dt.time.serial - datetime.time.serial);
			double angle = delta_sec / 120.0;
			if (lo <= angle && angle <= hi) marks.push_back(angle);
		}
	}
	return marks;
}

void StandardClock::drawVisibleSunBands() {
	int cx = _cx, cy = _cy;
	double hand_angle = hourHandAngle();
	double lo = -180.0 + SEAM_GAP_DEGREES / 2.0;
	double hi = 180.0 - SEAM_GAP_DEGREES / 2.0;
	for (const VisualBand& vb : visualSunBands()) {
		double clippedStart = std::max(vb.startAngle, lo);
		double clippedEnd = std::min(vb.endAngle, hi);
		if (clippedStart >= clippedEnd) continue;
		double abs_start = fmod(clippedStart + hand_angle, 360.0);
		if (abs_start < 0) abs_start += 360.0;
		double abs_end = fmod(clippedEnd + hand_angle, 360.0);
		if (abs_end < 0) abs_end += 360.0;
		double pil_start = abs_start - 90.0;
		double pil_end = abs_end - 90.0;
		if (pil_end < pil_start) pil_end += 360.0;
		if (pil_start == pil_end) continue;
		d->drawArc(cx, cy, SUN_RING_RADIUS, pil_start, pil_end, (int)SUN_RING_THICKNESS, vb.color);
	}
}

void StandardClock::drawVisibleHalakhicBands() {
	int cx = _cx, cy = _cy;
	double hand_angle = hourHandAngle();
	double lo = -180.0 + SEAM_GAP_DEGREES / 2.0;
	double hi = 180.0 - SEAM_GAP_DEGREES / 2.0;

	double trackStart = fmod(lo + hand_angle, 360.0);
	if (trackStart < 0) trackStart += 360.0;
	double trackEnd = fmod(hi + hand_angle, 360.0);
	if (trackEnd < 0) trackEnd += 360.0;
	double pil_track_start = trackStart - 90.0;
	double pil_track_end = trackEnd - 90.0;
	if (pil_track_end < pil_track_start) pil_track_end += 360.0;
	d->drawArc(cx, cy, HALAKHIC_RING_RADIUS, pil_track_start, pil_track_end, (int)HALAKHIC_RING_THICKNESS, RING_TRACK_COLOR);

	for (const VisualBand& vb : visualHalakhicBands()) {
		double clippedStart = std::max(vb.startAngle, lo);
		double clippedEnd = std::min(vb.endAngle, hi);
		if (clippedStart >= clippedEnd) continue;
		double abs_start = fmod(clippedStart + hand_angle, 360.0);
		if (abs_start < 0) abs_start += 360.0;
		double abs_end = fmod(clippedEnd + hand_angle, 360.0);
		if (abs_end < 0) abs_end += 360.0;
		double pil_start = abs_start - 90.0;
		double pil_end = abs_end - 90.0;
		if (pil_end < pil_start) pil_end += 360.0;
		if (pil_start == pil_end) continue;
		d->drawArc(cx, cy, HALAKHIC_RING_RADIUS, pil_start, pil_end, (int)HALAKHIC_RING_THICKNESS, vb.color);
	}

	double half_thick = HALAKHIC_RING_THICKNESS / 2.0;
	double r_inner = HALAKHIC_RING_RADIUS - half_thick;
	double r_outer = HALAKHIC_RING_RADIUS + half_thick;
	for (double angle : visibleSeasonalHourMarks()) {
		double abs_angle = fmod(angle + hand_angle, 360.0);
		if (abs_angle < 0) abs_angle += 360.0;
		double rad = (abs_angle - 90.0) * M_PI / 180.0;
		int x1 = cx + (int)(r_inner * cos(rad));
		int y1 = cy + (int)(r_inner * sin(rad));
		int x2 = cx + (int)(r_outer * cos(rad));
		int y2 = cy + (int)(r_outer * sin(rad));
		d->drawLine(x1, y1, x2, y2, 1, 0xFFFFFF);
	}
}

void StandardClock::drawVisibleMoonBands() {
	int cx = _cx, cy = _cy;
	double hand_angle = hourHandAngle();
	double lo = -180.0 + SEAM_GAP_DEGREES / 2.0;
	double hi = 180.0 - SEAM_GAP_DEGREES / 2.0;
	for (const VisualBand& vb : visualMoonBands()) {
		double clippedStart = std::max(vb.startAngle, lo);
		double clippedEnd = std::min(vb.endAngle, hi);
		if (clippedStart >= clippedEnd) continue;
		double abs_start = fmod(clippedStart + hand_angle, 360.0);
		if (abs_start < 0) abs_start += 360.0;
		double abs_end = fmod(clippedEnd + hand_angle, 360.0);
		if (abs_end < 0) abs_end += 360.0;
		double pil_start = abs_start - 90.0;
		double pil_end = abs_end - 90.0;
		if (pil_end < pil_start) pil_end += 360.0;
		if (pil_start == pil_end) continue;
		d->drawArc(cx, cy, MOON_RING_RADIUS, pil_start, pil_end, (int)MOON_RING_THICKNESS, vb.color);
	}
}

void StandardClock::drawVisibleObservanceBands() {
	int cx = _cx, cy = _cy;
	double hand_angle = hourHandAngle();
	double lo = -180.0 + SEAM_GAP_DEGREES / 2.0;
	double hi = 180.0 - SEAM_GAP_DEGREES / 2.0;
	for (const VisualBand& vb : visibleObservanceBands()) {
		double clippedStart = std::max(vb.startAngle, lo);
		double clippedEnd = std::min(vb.endAngle, hi);
		if (clippedStart >= clippedEnd) continue;
		double abs_start = fmod(clippedStart + hand_angle, 360.0);
		if (abs_start < 0) abs_start += 360.0;
		double abs_end = fmod(clippedEnd + hand_angle, 360.0);
		if (abs_end < 0) abs_end += 360.0;
		double pil_start = abs_start - 90.0;
		double pil_end = abs_end - 90.0;
		if (pil_end < pil_start) pil_end += 360.0;
		if (pil_start == pil_end) continue;
		d->drawArc(cx, cy, OBSERVANCE_RING_RADIUS, pil_start, pil_end, (int)OBSERVANCE_RING_THICKNESS, vb.color, vb.stripe);
	}
}

void StandardClock::drawHalakhicBands() {
	Band bandsArr[LOCAL_HEBREW_DATE_MAX_HALAKHIC_BANDS];
	int count = localHebrewDate->halakhicBands(bandsArr);

	struct Entry {
		DateTime start;
		DateTime end;
		uint32_t color;
	};
	std::vector<Entry> datetimes;
	for (int i = 0; i < count; ++i) {
		Entry e;
		e.start = bandsArr[i].start.datetime + TIMEZONE;
		e.end = bandsArr[i].end.datetime + TIMEZONE;
		e.color = _halakhicColor(bandsArr[i]);
		datetimes.push_back(e);
	}
	if (datetimes.empty()) return;

	long min_idx = 0, max_idx = 0;
	bool first = true;
	for (const Entry& e : datetimes) {
		const DateTime* pair[2] = {&e.start, &e.end};
		for (const DateTime* dtp : pair) {
			long idx = dtp->date.ordinal * 2 + (dtp->time.serial < 43200 ? 0 : 1);
			if (first) {
				min_idx = max_idx = idx;
				first = false;
			} else {
				if (idx < min_idx) min_idx = idx;
				if (idx > max_idx) max_idx = idx;
			}
		}
	}

	std::vector<Display> canvases;
	for (long i = min_idx; i <= max_idx; ++i) {
		Display canvas(size);
		canvas.fill(BACKGROUND_COLOR);
		canvases.push_back(canvas);
	}

	int cx = size / 2;
	int cy = size / 2;
	for (const Entry& e : datetimes) {
		DateTime current = e.start;
		while (current < e.end) {
			DateTime seg_end;
			if (current.time.serial < 43200) {
				seg_end = DateTime(current.date, Time(12, 0, 0));
			} else {
				seg_end = DateTime(Date::fromordinal(current.date.ordinal + 1), Time(0, 0, 0));
			}
			DateTime piece_end = (e.end < seg_end) ? e.end : seg_end;
			if (piece_end > current) {
				long seg_idx = current.date.ordinal * 2 + (current.time.serial < 43200 ? 0 : 1);
				long canvas_idx = seg_idx - min_idx;
				Display& canvas = canvases[canvas_idx];

				long seg_start_seconds = (current.time.serial < 43200) ? 0 : 43200;
				long start_seconds = current.time.serial - seg_start_seconds;
				long end_seconds = piece_end.time.serial - seg_start_seconds;
				double start_angle = start_seconds / 120.0;
				double end_angle = end_seconds / 120.0;
				double pil_start = start_angle - 90.0;
				double pil_end = end_angle - 90.0;
				if (pil_end <= pil_start) pil_end += 360.0;
				/* Python's `continue` here skips the unconditional `current = piece_end`
				 * below it, which would spin this while loop forever whenever a
				 * segment collapses to zero width -- ported with that advance kept
				 * unconditional instead of reproducing the hang. See port notes. */
				if (pil_start != pil_end) {
					canvas.drawArc(cx, cy, HALAKHIC_RING_RADIUS, pil_start, pil_end, (int)HALAKHIC_RING_THICKNESS, e.color);
				}
			}
			current = piece_end;
		}
	}
}

void StandardClock::drawDigits() {
	int cx = _cx, cy = _cy;
	double hourHand = hourHandAngle();
	double opposite = fmod(hourHand + 180.0, 360.0);
	if (opposite < 0) opposite += 360.0;
	double gap = SEAM_GAP_DEGREES / 2.0;
	for (int mark = 0; mark < 12; ++mark) {
		double markAngle = mark * 30.0;
		double diff = fmod(markAngle - opposite + 180.0, 360.0);
		if (diff < 0) diff += 360.0;
		diff -= 180.0;
		if (fabs(diff) < gap) continue;
		double delta = markAngle - hourHand;
		if (delta > 180.0) delta -= 360.0;
		else if (delta <= -180.0) delta += 360.0;
		double total_sec = fmod(datetime.time.serial + delta * 120.0, 86400.0);
		if (total_sec < 0) total_sec += 86400.0;
		int hour24 = ((int)lround(total_sec / 3600.0)) % 24;
		if (hour24 < 0) hour24 += 24;
		std::string digit = std::to_string(hour24);
		double posAngle = markAngle - 90.0;
		double rotation;
		if (markAngle == 90.0) rotation = 90.0;
		else if (markAngle == 270.0) rotation = -90.0;
		else if (markAngle > 90.0 && markAngle < 270.0) rotation = 180.0;
		else rotation = 0.0;
		d->drawCurvedChar(cx, cy, LABEL_RADIUS, posAngle, digit.c_str(), FONT_ASSISTANT, FONT_SIZE, rotation);
	}
}

void StandardClock::drawMarks() {
	int cx = _cx, cy = _cy;
	for (double angleDeg : minuteMarks()) {
		_draw_tick(*d, cx, cy, angleDeg, MINUTE_TICK_LENGTH, 1, MINUTE_TICK_COLOR);
	}
	for (double angleDeg : hourMarks()) {
		_draw_tick(*d, cx, cy, angleDeg, HOUR_SHORT_TICK_LENGTH, 2, TICK_COLOR);
	}
	for (double angleDeg : hourMajorMarks()) {
		_draw_tick(*d, cx, cy, angleDeg, HOUR_LONG_TICK_LENGTH, 2, TICK_COLOR);
	}
}

void StandardClock::drawHands() {
	int cx = _cx, cy = _cy;
	_draw_hand(*d, cx, cy, HOUR_HAND_LENGTH, HOUR_HAND_TAIL, hourHandAngle(), (int)HOUR_HAND_THICKNESS, HAND_COLOR);
	_draw_hand(*d, cx, cy, MINUTE_HAND_LENGTH, MINUTE_HAND_TAIL, minuteHandAngle(), (int)MINUTE_HAND_THICKNESS, HAND_COLOR);
}

void StandardClock::drawFace() {
	_draw_face(*d, _cx, _cy);
}

void StandardClock::draw(Display& disp) {
	d = &disp;
	_cx = disp.WIDTH / 2;
	_cy = disp.HEIGHT / 2;
	if (!faceComputed) {
		Display temp(size);
		temp.fillCircle(_cx, _cy, DIAL_RADIUS, BACKGROUND_COLOR);
		Display* old_d = d;
		d = &temp;
		drawMarks();
		d = old_d;
		face = temp.buffer;
		faceComputed = true;
	}
	disp.buffer = face;
	drawVisibleSunBands();
	drawVisibleMoonBands();
	drawVisibleHalakhicBands();
	drawVisibleObservanceBands();
	drawDigits();
	drawHands();
	drawFace();
}
