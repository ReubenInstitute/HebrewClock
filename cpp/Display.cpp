#include "Display.h"

#include <algorithm>
#include <math.h>
#include <string.h>
#include <map>
#include <string>

#ifndef ARDUINO
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Display::Display(int size) : WIDTH(size), HEIGHT(size), buffer((size_t)size * size * 4) {
	clear();
}

void Display::clear() {
	std::fill(buffer.begin(), buffer.end(), (uint8_t)0);
}

void Display::fill(uint32_t color) {
	uint8_t r = (color >> 16) & 0xFF;
	uint8_t g = (color >> 8) & 0xFF;
	uint8_t b = color & 0xFF;
	for (size_t i = 0; i < buffer.size(); i += 4) {
		buffer[i]     = r;
		buffer[i + 1] = g;
		buffer[i + 2] = b;
		buffer[i + 3] = 255;
	}
}

void Display::_blend(int idx, uint8_t fr, uint8_t fg, uint8_t fb, double alpha) {
	if (alpha <= 0.0) return;
	if (alpha > 1.0) alpha = 1.0;

	double ba = buffer[idx + 3] / 255.0;
	double outA = alpha + ba * (1.0 - alpha);

	if (outA <= 0.0) {
		buffer[idx] = 0;
		buffer[idx + 1] = 0;
		buffer[idx + 2] = 0;
		buffer[idx + 3] = 0;
		return;
	}

	if (alpha >= 1.0) {
		buffer[idx] = fr;
		buffer[idx + 1] = fg;
		buffer[idx + 2] = fb;
		buffer[idx + 3] = 255;
		return;
	}

	double br = buffer[idx];
	double bg = buffer[idx + 1];
	double bb = buffer[idx + 2];
	double remainder = ba * (1.0 - alpha);

	double outR = (fr * alpha + br * remainder) / outA;
	double outG = (fg * alpha + bg * remainder) / outA;
	double outB = (fb * alpha + bb * remainder) / outA;

	buffer[idx]     = (uint8_t)(outR + 0.5);
	buffer[idx + 1] = (uint8_t)(outG + 0.5);
	buffer[idx + 2] = (uint8_t)(outB + 0.5);
	buffer[idx + 3] = (uint8_t)(outA * 255.0 + 0.5);
}

double Display::_pointToSegmentDistance(double px, double py, double x1, double y1, double x2, double y2) {
	double dx = x2 - x1;
	double dy = y2 - y1;
	double lenSq = dx * dx + dy * dy;
	if (lenSq == 0.0) {
		double ex = px - x1;
		double ey = py - y1;
		return sqrt(ex * ex + ey * ey);
	}
	double t = ((px - x1) * dx + (py - y1) * dy) / lenSq;
	if (t < 0.0) t = 0.0;
	if (t > 1.0) t = 1.0;
	double projx = x1 + t * dx;
	double projy = y1 + t * dy;
	double ex = px - projx;
	double ey = py - projy;
	return sqrt(ex * ex + ey * ey);
}

double Display::_pointToArcDistance(double px, double py, double cx, double cy, double radius,
									 double startAngle, double endAngle, double halfWidth) {
	if (endAngle - startAngle >= 360.0) {
		double dx = px - cx;
		double dy = py - cy;
		double distToCenter = sqrt(dx * dx + dy * dy);
		return fabs(distToCenter - radius);
	}

	double dx = px - cx;
	double dy = py - cy;
	double distToCenter = sqrt(dx * dx + dy * dy);

	double sr = startAngle * M_PI / 180.0;
	double er = endAngle * M_PI / 180.0;
	sr = fmod(sr, 2.0 * M_PI);
	if (sr < 0.0) sr += 2.0 * M_PI;
	er = fmod(er, 2.0 * M_PI);
	if (er < 0.0) er += 2.0 * M_PI;

	bool inRange;
	if (distToCenter == 0.0) {
		double a0 = 0.0;
		if (startAngle <= endAngle) {
			inRange = (a0 >= startAngle && a0 <= endAngle);
		} else {
			inRange = (a0 >= startAngle || a0 <= endAngle);
		}
	} else {
		double angle = atan2(dy, dx);
		if (angle < 0.0) angle += 2.0 * M_PI;

		if (sr <= er) {
			inRange = (angle >= sr && angle <= er);
		} else {
			inRange = (angle >= sr || angle <= er);
		}
	}

	if (inRange) {
		return fabs(distToCenter - radius);
	}

	double innerR = radius - halfWidth;
	double outerR = radius + halfWidth;

	double sxi = cx + innerR * cos(sr);
	double syi = cy + innerR * sin(sr);
	double sxo = cx + outerR * cos(sr);
	double syo = cy + outerR * sin(sr);

	double exi = cx + innerR * cos(er);
	double eyi = cy + innerR * sin(er);
	double exo = cx + outerR * cos(er);
	double eyo = cy + outerR * sin(er);

	double d1 = _pointToSegmentDistance(px, py, sxi, syi, sxo, syo);
	double d2 = _pointToSegmentDistance(px, py, exi, eyi, exo, eyo);
	double dmin = d1 < d2 ? d1 : d2;

	return dmin + halfWidth;
}

void Display::drawLine(int x1, int y1, int x2, int y2, int thickness, uint32_t color) {
	uint8_t fr = (color >> 16) & 0xFF;
	uint8_t fg = (color >> 8) & 0xFF;
	uint8_t fb = color & 0xFF;

	double hw = thickness * 0.5;
	int xmin = (x1 < x2 ? x1 : x2) - (int)(hw + 1.0);
	int xmax = (x1 > x2 ? x1 : x2) + (int)(hw + 1.0);
	int ymin = (y1 < y2 ? y1 : y2) - (int)(hw + 1.0);
	int ymax = (y1 > y2 ? y1 : y2) + (int)(hw + 1.0);

	if (xmin < 0) xmin = 0;
	if (ymin < 0) ymin = 0;
	if (xmax >= WIDTH)  xmax = WIDTH - 1;
	if (ymax >= HEIGHT) ymax = HEIGHT - 1;

	for (int y = ymin; y <= ymax; ++y) {
		for (int x = xmin; x <= xmax; ++x) {
			double px = x + 0.5;
			double py = y + 0.5;
			double dist = _pointToSegmentDistance(px, py, x1 + 0.5, y1 + 0.5, x2 + 0.5, y2 + 0.5);
			double r = dist - hw;
			double alpha;
			if (r <= -1.0) alpha = 1.0;
			else if (r >= 1.0) alpha = 0.0;
			else alpha = 1.0 - (r + 1.0) * 0.5;

			if (alpha <= 0.0) continue;

			int idx = (y * WIDTH + x) * 4;
			_blend(idx, fr, fg, fb, alpha);
		}
	}
}

void Display::drawArc(int cx, int cy, double radius, double startAngle, double endAngle,
					   int thickness, uint32_t color, bool stripe, uint32_t stripeColor) {
	uint8_t fr = (color >> 16) & 0xFF;
	uint8_t fg = (color >> 8) & 0xFF;
	uint8_t fb = color & 0xFF;
	double hw = thickness * 0.5;
	int margin = (int)(hw + 1.0);
	int xmin = (int)(cx - radius - margin);
	int xmax = (int)(cx + radius + margin);
	int ymin = (int)(cy - radius - margin);
	int ymax = (int)(cy + radius + margin);
	if (xmin < 0) xmin = 0;
	if (ymin < 0) ymin = 0;
	if (xmax >= WIDTH)  xmax = WIDTH - 1;
	if (ymax >= HEIGHT) ymax = HEIGHT - 1;

	for (int y = ymin; y <= ymax; ++y) {
		for (int x = xmin; x <= xmax; ++x) {
			double px = x + 0.5;
			double py = y + 0.5;
			double dist = _pointToArcDistance(px, py, cx + 0.5, cy + 0.5, radius, startAngle, endAngle, hw);
			double r = dist - hw;
			double alpha;
			if (r <= -1.0) alpha = 1.0;
			else if (r >= 1.0) alpha = 0.0;
			else alpha = 1.0 - (r + 1.0) * 0.5;
			if (alpha <= 0.0) continue;
			int idx = (y * WIDTH + x) * 4;
			_blend(idx, fr, fg, fb, alpha);
		}
	}

	if (stripe) {
		double halfWidth = thickness / 2.0;
		double innerR = radius - halfWidth;
		double outerR = radius + halfWidth;
		double stripe_spacing_deg = 6.0;
		double gridOrigin = -90.0;
		double n0 = ceil((startAngle - gridOrigin) / stripe_spacing_deg);
		double a = gridOrigin + n0 * stripe_spacing_deg;
		while (a <= endAngle) {
			double rad = a * M_PI / 180.0;
			double x_inner = cx + innerR * cos(rad);
			double y_inner = cy + innerR * sin(rad);
			double x_outer = cx + outerR * cos(rad);
			double y_outer = cy + outerR * sin(rad);
			double mx = (x_inner + x_outer) / 2.0;
			double my = (y_inner + y_outer) / 2.0;
			double rot_angle = 45.0 * M_PI / 180.0;
			double cos_r = cos(rot_angle);
			double sin_r = sin(rot_angle);
			double dx_inner = x_inner - mx;
			double dy_inner = y_inner - my;
			double x_inner_rot = mx + dx_inner * cos_r - dy_inner * sin_r;
			double y_inner_rot = my + dx_inner * sin_r + dy_inner * cos_r;
			double dx_outer = x_outer - mx;
			double dy_outer = y_outer - my;
			double x_outer_rot = mx + dx_outer * cos_r - dy_outer * sin_r;
			double y_outer_rot = my + dx_outer * sin_r + dy_outer * cos_r;
			drawLine((int)x_inner_rot, (int)y_inner_rot, (int)x_outer_rot, (int)y_outer_rot, 1, stripeColor);
			a += stripe_spacing_deg;
		}
	}
}

void Display::drawCircle(int cx, int cy, double radius, int thickness, uint32_t color) {
	drawArc(cx, cy, radius, 0.0, 360.0, thickness, color);
}

void Display::fillCircle(int cx, int cy, double radius, uint32_t color) {
	uint8_t fr = (color >> 16) & 0xFF;
	uint8_t fg = (color >> 8) & 0xFF;
	uint8_t fb = color & 0xFF;

	int margin = 1;
	int xmin = (int)(cx - radius - margin);
	int xmax = (int)(cx + radius + margin);
	int ymin = (int)(cy - radius - margin);
	int ymax = (int)(cy + radius + margin);
	if (xmin < 0) xmin = 0;
	if (ymin < 0) ymin = 0;
	if (xmax >= WIDTH)  xmax = WIDTH - 1;
	if (ymax >= HEIGHT) ymax = HEIGHT - 1;

	for (int y = ymin; y <= ymax; ++y) {
		for (int x = xmin; x <= xmax; ++x) {
			double px = x + 0.5 - (cx + 0.5);
			double py = y + 0.5 - (cy + 0.5);
			double dist = sqrt(px * px + py * py);
			double r = dist - radius;
			double alpha;
			if (r <= -1.0) alpha = 1.0;
			else if (r >= 1.0) alpha = 0.0;
			else alpha = 1.0 - (r + 1.0) * 0.5;
			if (alpha <= 0.0) continue;
			int idx = (y * WIDTH + x) * 4;
			_blend(idx, fr, fg, fb, alpha);
		}
	}
}

/* ---------- Text rendering ----------
 * Display.py rasterizes the whole string with PIL/TTF, then rotates and
 * resizes the raster as one block. Here the FreeType outline is transformed
 * (rotated) directly before rasterization instead, which sidesteps
 * reimplementing image rotation/resampling and antialiases natively -- the
 * observable result (a centered, rotated, always-black text block) matches.
 */
#ifdef ARDUINO
/* FreeType isn't available on embedded targets; MCU builds don't render text. */
void Display::drawCurvedChar(int, int, double, double, const char*, const char*, int, double) {}
#else
namespace {

FT_Library ftLibrary() {
	static FT_Library library = [] {
		FT_Library lib = nullptr;
		FT_Init_FreeType(&lib);
		return lib;
	}();
	return library;
}

FT_Face ftFace(const char* fontFilename) {
	static std::map<std::string, FT_Face> cache;
	auto it = cache.find(fontFilename);
	if (it != cache.end()) return it->second;
	FT_Face face = nullptr;
	FT_Error err = FT_New_Face(ftLibrary(), fontFilename, 0, &face);
	if (err) face = nullptr;
	cache[fontFilename] = face;
	return face;
}

}   /* namespace */

void Display::drawCurvedChar(int cx, int cy, double radius, double angle, const char* text,
							  const char* fontFilename, int fontsize, double rotation) {
	FT_Face face = ftFace(fontFilename);
	if (!face) return;
	FT_Set_Pixel_Sizes(face, 0, (FT_UInt)fontsize);

	/* Screen convention throughout (x right, y down), matching Display.py's
	 * angle usage directly -- not the y-up convention FreeType uses internally. */
	double angleRad = angle * M_PI / 180.0;
	double targetX = cx + radius * cos(angleRad);
	double targetY = cy + radius * sin(angleRad);

	double rotDeg = -angle - 90.0 + rotation;
	double theta = rotDeg * M_PI / 180.0;
	double cosT = cos(theta);
	double sinT = sin(theta);

	int len = (int)strlen(text);
	if (len <= 0) return;

	/* First pass, untransformed: measure horizontal advances to find the
	 * unrotated block width, matching PIL's textbbox-based centering. */
	FT_Set_Transform(face, nullptr, nullptr);
	std::vector<double> penX(len + 1, 0.0);
	for (int i = 0; i < len; ++i) {
		FT_Load_Char(face, (FT_ULong)(unsigned char)text[i], FT_LOAD_DEFAULT);
		penX[i + 1] = penX[i] + face->glyph->advance.x / 64.0;
	}
	double blockCenterX = penX[len] / 2.0;
	double ascender = face->size->metrics.ascender / 64.0;
	double descender = face->size->metrics.descender / 64.0;
	double blockCenterYFont = (ascender + descender) / 2.0;   /* y-up, relative to baseline */

	for (int i = 0; i < len; ++i) {
		/* Local offset from block center, in screen (y-down) convention. */
		double localX = penX[i] - blockCenterX;
		double localY = blockCenterYFont;

		double rotX = localX * cosT + localY * sinT;
		double rotY = -localX * sinT + localY * cosT;
		double penScreenX = targetX + rotX;
		double penScreenY = targetY + rotY;

		/* Glyph outline transform mirrors the pen rotation above, but expressed
		 * in FreeType's y-up font space (hence the sign flip vs. the screen-space
		 * formula). */
		FT_Matrix matrix;
		matrix.xx = (FT_Fixed)lround(cosT * 0x10000L);
		matrix.xy = (FT_Fixed)lround(sinT * 0x10000L);
		matrix.yx = (FT_Fixed)lround(-sinT * 0x10000L);
		matrix.yy = (FT_Fixed)lround(cosT * 0x10000L);
		FT_Vector delta = {0, 0};
		FT_Set_Transform(face, &matrix, &delta);

		FT_Load_Char(face, (FT_ULong)(unsigned char)text[i], FT_LOAD_RENDER);
		FT_GlyphSlot slot = face->glyph;
		FT_Bitmap& bmp = slot->bitmap;

		int originX = (int)lround(penScreenX) + slot->bitmap_left;
		int originY = (int)lround(penScreenY) - slot->bitmap_top;

		for (unsigned int row = 0; row < bmp.rows; ++row) {
			for (unsigned int col = 0; col < bmp.width; ++col) {
				uint8_t a = bmp.buffer[row * bmp.pitch + col];
				if (a == 0) continue;
				int px = originX + (int)col;
				int py = originY + (int)row;
				if (px < 0 || px >= WIDTH || py < 0 || py >= HEIGHT) continue;
				int idx = (py * WIDTH + px) * 4;
				_blend(idx, 0, 0, 0, a / 255.0);
			}
		}
	}
}
#endif /* ARDUINO */

void Display::save(const char* filename) const {
	stbi_write_png(filename, WIDTH, HEIGHT, 4, buffer.data(), WIDTH * 4);
}
