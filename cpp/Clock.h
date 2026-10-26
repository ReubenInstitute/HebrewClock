/* Clock.h
 * Ported from LIBS/Clock.py -- Clock base class and StandardClock only.
 * (HebrewClock and the __main__ demo/test block are intentionally not ported.)
 */
#ifndef CLOCK_H
#define CLOCK_H

#include <memory>
#include <stdint.h>
#include <vector>

#include "Custom.h"
#include "Date.h"
#include "Display.h"
#include "Geo.h"
#include "LocalHebrewDate.h"

class VisualBand {
public:
	double startAngle;
	double endAngle;
	uint32_t color;
	bool stripe;

	VisualBand(double startAngle, double endAngle, uint32_t color, bool stripe = false);
};

class Clock {
public:
	static const uint32_t BACKGROUND_COLOR = 0xFFFFFF;
	static const uint32_t TICK_COLOR = 0x333333;
	static const uint32_t HAND_COLOR = 0x222222;
	static const uint32_t SECOND_HAND_COLOR = 0xCC0000;
	static const uint32_t FACE_COLOR = 0x333333;

	static const uint32_t DAY_LABEL = 0xC0C0C0;
	static const uint32_t DAY_LIGHT_LABEL = 0xE0E0E0;
	static const uint32_t WATCH_1_3_LABEL = 0x3A3A3A;
	static const uint32_t WATCH_1_3_LIGHT_LABEL = 0x5A5A5A;
	static const uint32_t WATCH_2_3_LABEL = 0x6A6A6A;
	static const uint32_t WATCH_2_3_LIGHT_LABEL = 0x8A8A8A;
	static const uint32_t WATCH_3_3_LABEL = 0x9A9A9A;
	static const uint32_t WATCH_3_3_LIGHT_LABEL = 0xBABABA;
	static const uint32_t WATCH_1_4_LABEL = 0x3A3A3A;
	static const uint32_t WATCH_1_4_LIGHT_LABEL = 0x5A5A5A;
	static const uint32_t WATCH_2_4_LABEL = 0x5A5A5A;
	static const uint32_t WATCH_2_4_LIGHT_LABEL = 0x7A7A7A;
	static const uint32_t WATCH_3_4_LABEL = 0x7A7A7A;
	static const uint32_t WATCH_3_4_LIGHT_LABEL = 0x9A9A9A;
	static const uint32_t WATCH_4_4_LABEL = 0x9A9A9A;
	static const uint32_t WATCH_4_4_LIGHT_LABEL = 0xBABABA;

	static uint32_t _halakhicColor(const Band& band);
	static uint32_t _observanceColor(const Band& band);
	static uint32_t _moonColor(const Band& band);
	static uint32_t _sunColor(const Band& band);

	explicit Clock(int size = 240);
	virtual ~Clock() {}

	virtual void draw(Display& d) = 0;

protected:
	int size;
	double MOON_RING_RADIUS;
	double MOON_RING_THICKNESS;
	double SUN_RING_RADIUS;
	double SUN_RING_THICKNESS;
	double HALAKHIC_RING_RADIUS;
	double HALAKHIC_RING_THICKNESS;
	double OBSERVANCE_RING_RADIUS;
	double OBSERVANCE_RING_THICKNESS;
	double DIAL_RADIUS;
	double HOUR_SHORT_TICK_LENGTH;
	double HOUR_MEDIUM_TICK_LENGTH;
	double HOUR_LONG_TICK_LENGTH;
	double CAP_RADIUS;
	double RIM_THICKNESS;
	double HOUR_HAND_LENGTH;
	double HOUR_HAND_TAIL;
	double HOUR_HAND_THICKNESS;
	double SECOND_HAND_THICKNESS;

	void _draw_tick(Display& d, int cx, int cy, double angleDeg, double length, int thickness, uint32_t color);
	void _draw_hand(Display& d, int cx, int cy, double length, double tail, double angleDeg, int thickness, uint32_t color);
	void _draw_face(Display& d, int cx, int cy);
};

class StandardClock : public Clock {
public:
	static const char* const FONT_ASSISTANT;
	static const uint32_t MINUTE_TICK_COLOR = 0xAAAAAA;
	static const uint32_t RING_TRACK_COLOR = 0xDCDCDC;
	static constexpr double SEAM_GAP_DEGREES = 30.0;

	StandardClock(DateTime datetime, Location location, Custom custom, int size = 240);

	void update(DateTime datetime);

	double hourHandAngle() const;
	double minuteHandAngle() const;
	double secondHandAngle() const;
	std::vector<double> hourMajorMarks() const;
	std::vector<double> hourMarks() const;
	std::vector<double> minuteMarks() const;
	DateTime windowStart() const;
	DateTime windowEnd() const;

	std::vector<Band> halakhicBands() const;
	std::vector<VisualBand> visualHalakhicBands() const;
	std::vector<Band> sunBands() const;
	std::vector<VisualBand> visualSunBands() const;
	std::vector<Band> moonBands() const;
	std::vector<VisualBand> visualMoonBands() const;
	std::vector<Band> observanceBands() const;
	std::vector<VisualBand> visibleObservanceBands() const;
	std::vector<double> visibleSeasonalHourMarks() const;

	void drawVisibleSunBands();
	void drawVisibleHalakhicBands();
	void drawVisibleMoonBands();
	void drawVisibleObservanceBands();
	/* Ported from Python's drawHalakhicBands(), which builds a list of
	 * per-half-day Display canvases and returns them -- but every caller
	 * (including update(), below) discards the return value, so only the
	 * drawing side effect (computed into throwaway canvases) is observable.
	 * Kept void here since threading an unused return value through would
	 * add nothing. */
	void drawHalakhicBands();
	void drawDigits();
	void drawMarks();
	void drawHands();
	void drawFace();

	void draw(Display& d) override;

private:
	double LABEL_RADIUS;
	int FONT_SIZE;
	double MINUTE_TICK_LENGTH;
	double MINUTE_HAND_LENGTH;
	double MINUTE_HAND_TAIL;
	double MINUTE_HAND_THICKNESS;
	double SECOND_HAND_LENGTH;
	double SECOND_HAND_TAIL;

	Location location;
	Custom custom;
	DateTime datetime;

	std::unique_ptr<LocalHebrewDate> localHebrewDate;
	bool hasLastHebrewDate;
	HebrewDate lastHebrewDate;

	bool faceComputed;
	std::vector<uint8_t> face;

	Display* d;
	int _cx;
	int _cy;

	std::vector<VisualBand> _toVisualBands(const std::vector<Band>& bands,
											uint32_t (*colorFn)(const Band&),
											bool useStripe) const;
};

#endif
