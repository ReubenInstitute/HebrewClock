/* Display.h
 * Ported from LIBS/Display.py
 */
#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <vector>

class Display {
public:
	int WIDTH;
	int HEIGHT;
	std::vector<uint8_t> buffer;   /* RGBA, WIDTH*HEIGHT*4 bytes */

	explicit Display(int size = 240);

	void clear();
	void fill(uint32_t color = 0x000000);

	void drawLine(int x1, int y1, int x2, int y2, int thickness, uint32_t color);
	void drawArc(int cx, int cy, double radius, double startAngle, double endAngle,
				 int thickness, uint32_t color, bool stripe = false, uint32_t stripeColor = 0xFFFFFF);
	void drawCircle(int cx, int cy, double radius, int thickness, uint32_t color);
	void fillCircle(int cx, int cy, double radius, uint32_t color);

	/* text is rendered with FreeType (fontFilename loaded/cached per path); always black,
	 * matching Display.py's hardcoded fill=(0,0,0,255). */
	void drawCurvedChar(int cx, int cy, double radius, double angle, const char* text,
						 const char* fontFilename, int fontsize, double rotation = 0.0);

	void save(const char* filename) const;

private:
	void _blend(int idx, uint8_t fr, uint8_t fg, uint8_t fb, double alpha);

	static double _pointToSegmentDistance(double px, double py, double x1, double y1, double x2, double y2);
	static double _pointToArcDistance(double px, double py, double cx, double cy, double radius,
									   double startAngle, double endAngle, double halfWidth);
};

#endif
