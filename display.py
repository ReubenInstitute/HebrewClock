import math
from PIL import Image, ImageDraw, ImageFont


class Display:
	_font_cache = {}

	def __init__(self, size=240):
		self.WIDTH = size
		self.HEIGHT = size
		self.buffer = bytearray(self.WIDTH * self.HEIGHT * 4)
		self.clear()

	def clear(self):
		for i in range(0, len(self.buffer), 4):
			self.buffer[i]	   = 0
			self.buffer[i + 1] = 0
			self.buffer[i + 2] = 0
			self.buffer[i + 3] = 0

	def fill(self, color=0x000000):
		r = (color >> 16) & 0xFF
		g = (color >> 8) & 0xFF
		b = color & 0xFF
		for i in range(0, len(self.buffer), 4):
			self.buffer[i]	   = r
			self.buffer[i + 1] = g
			self.buffer[i + 2] = b
			self.buffer[i + 3] = 255

	def _blend(self, idx, fr, fg, fb, alpha):
		if alpha <= 0.0:
			return
		if alpha > 1.0:
			alpha = 1.0

		ba = self.buffer[idx + 3] / 255.0
		outA = alpha + ba * (1.0 - alpha)

		if outA <= 0.0:
			self.buffer[idx]	 = 0
			self.buffer[idx + 1] = 0
			self.buffer[idx + 2] = 0
			self.buffer[idx + 3] = 0
			return

		if alpha >= 1.0:
			self.buffer[idx]	 = fr
			self.buffer[idx + 1] = fg
			self.buffer[idx + 2] = fb
			self.buffer[idx + 3] = 255
			return

		br = self.buffer[idx]
		bg = self.buffer[idx + 1]
		bb = self.buffer[idx + 2]
		remainder = ba * (1.0 - alpha)

		outR = (fr * alpha + br * remainder) / outA
		outG = (fg * alpha + bg * remainder) / outA
		outB = (fb * alpha + bb * remainder) / outA

		self.buffer[idx]	 = int(outR + 0.5)
		self.buffer[idx + 1] = int(outG + 0.5)
		self.buffer[idx + 2] = int(outB + 0.5)
		self.buffer[idx + 3] = int(outA * 255.0 + 0.5)

	@staticmethod
	def _pointToSegmentDistance(px, py, x1, y1, x2, y2):
		dx = x2 - x1
		dy = y2 - y1
		lenSq = dx * dx + dy * dy
		if lenSq == 0.0:
			ex = px - x1
			ey = py - y1
			return math.sqrt(ex * ex + ey * ey)
		t = ((px - x1) * dx + (py - y1) * dy) / lenSq
		if t < 0.0:
			t = 0.0
		if t > 1.0:
			t = 1.0
		projx = x1 + t * dx
		projy = y1 + t * dy
		ex = px - projx
		ey = py - projy
		return math.sqrt(ex * ex + ey * ey)

	@staticmethod
	def _pointToArcDistance(px, py, cx, cy, radius, startAngle, endAngle, halfWidth):
		if endAngle - startAngle >= 360.0:
			dx = px - cx
			dy = py - cy
			distToCenter = math.sqrt(dx*dx + dy*dy)
			return abs(distToCenter - radius)

		dx = px - cx
		dy = py - cy
		distToCenter = math.sqrt(dx * dx + dy * dy)

		sr = math.radians(startAngle)
		er = math.radians(endAngle)
		sr = sr % (2.0 * math.pi)
		er = er % (2.0 * math.pi)

		if distToCenter == 0.0:
			a0 = 0.0
			if startAngle <= endAngle:
				inRange = (a0 >= startAngle and a0 <= endAngle)
			else:
				inRange = (a0 >= startAngle or a0 <= endAngle)
		else:
			angle = math.atan2(dy, dx)
			if angle < 0.0:
				angle += 2.0 * math.pi

			if sr <= er:
				inRange = (angle >= sr and angle <= er)
			else:
				inRange = (angle >= sr or angle <= er)

		if inRange:
			return abs(distToCenter - radius)

		innerR = radius - halfWidth
		outerR = radius + halfWidth

		sxi = cx + innerR * math.cos(sr)
		syi = cy + innerR * math.sin(sr)
		sxo = cx + outerR * math.cos(sr)
		syo = cy + outerR * math.sin(sr)

		exi = cx + innerR * math.cos(er)
		eyi = cy + innerR * math.sin(er)
		exo = cx + outerR * math.cos(er)
		eyo = cy + outerR * math.sin(er)

		d1 = Display._pointToSegmentDistance(px, py, sxi, syi, sxo, syo)
		d2 = Display._pointToSegmentDistance(px, py, exi, eyi, exo, eyo)
		dmin = d1 if d1 < d2 else d2

		return dmin + halfWidth

	def drawLine(self, x1, y1, x2, y2, thickness, color):
		fr = (color >> 16) & 0xFF
		fg = (color >> 8) & 0xFF
		fb = color & 0xFF

		hw = thickness * 0.5
		xmin = min(x1, x2) - int(hw + 1.0)
		xmax = max(x1, x2) + int(hw + 1.0)
		ymin = min(y1, y2) - int(hw + 1.0)
		ymax = max(y1, y2) + int(hw + 1.0)

		if xmin < 0: xmin = 0
		if ymin < 0: ymin = 0
		if xmax >= self.WIDTH:  xmax = self.WIDTH - 1
		if ymax >= self.HEIGHT: ymax = self.HEIGHT - 1

		for y in range(ymin, ymax + 1):
			for x in range(xmin, xmax + 1):
				px = x + 0.5
				py = y + 0.5
				dist = self._pointToSegmentDistance(px, py,
													x1 + 0.5, y1 + 0.5,
													x2 + 0.5, y2 + 0.5)
				r = dist - hw
				if r <= -1.0:
					alpha = 1.0
				elif r >= 1.0:
					alpha = 0.0
				else:
					alpha = 1.0 - (r + 1.0) * 0.5

				if alpha <= 0.0:
					continue

				idx = (y * self.WIDTH + x) * 4
				self._blend(idx, fr, fg, fb, alpha)

	def drawArc(self, cx, cy, radius, startAngle, endAngle, thickness, color, stripe=False, stripeColor=0xffffff):
		fr = (color >> 16) & 0xFF
		fg = (color >> 8) & 0xFF
		fb = color & 0xFF
		hw = thickness * 0.5
		margin = int(hw + 1.0)
		xmin = int(cx - radius - margin)
		xmax = int(cx + radius + margin)
		ymin = int(cy - radius - margin)
		ymax = int(cy + radius + margin)
		if xmin < 0: xmin = 0
		if ymin < 0: ymin = 0
		if xmax >= self.WIDTH:  xmax = self.WIDTH - 1
		if ymax >= self.HEIGHT: ymax = self.HEIGHT - 1
		for y in range(ymin, ymax + 1):
			for x in range(xmin, xmax + 1):
				px = x + 0.5
				py = y + 0.5
				dist = self._pointToArcDistance(px, py, cx + 0.5, cy + 0.5, float(radius), startAngle, endAngle, hw)
				r = dist - hw
				if r <= -1.0:
					alpha = 1.0
				elif r >= 1.0:
					alpha = 0.0
				else:
					alpha = 1.0 - (r + 1.0) * 0.5
				if alpha <= 0.0:
					continue
				idx = (y * self.WIDTH + x) * 4
				self._blend(idx, fr, fg, fb, alpha)

		if stripe:
			halfWidth = thickness / 2.0
			innerR = radius - halfWidth
			outerR = radius + halfWidth
			stripe_spacing_deg = 6.0
			gridOrigin = -90.0
			n0 = math.ceil((startAngle - gridOrigin) / stripe_spacing_deg)
			a = gridOrigin + n0 * stripe_spacing_deg
			while a <= endAngle:
				rad = math.radians(a)
				x_inner = cx + innerR * math.cos(rad)
				y_inner = cy + innerR * math.sin(rad)
				x_outer = cx + outerR * math.cos(rad)
				y_outer = cy + outerR * math.sin(rad)
				mx = (x_inner + x_outer) / 2.0
				my = (y_inner + y_outer) / 2.0
				rot_angle = math.radians(45.0)
				cos_r = math.cos(rot_angle)
				sin_r = math.sin(rot_angle)
				dx_inner = x_inner - mx
				dy_inner = y_inner - my
				x_inner_rot = mx + dx_inner * cos_r - dy_inner * sin_r
				y_inner_rot = my + dx_inner * sin_r + dy_inner * cos_r
				dx_outer = x_outer - mx
				dy_outer = y_outer - my
				x_outer_rot = mx + dx_outer * cos_r - dy_outer * sin_r
				y_outer_rot = my + dx_outer * sin_r + dy_outer * cos_r
				self.drawLine(int(x_inner_rot), int(y_inner_rot),
					int(x_outer_rot), int(y_outer_rot), 1, stripeColor)
				a += stripe_spacing_deg

	def drawCircle(self, cx, cy, radius, thickness, color):
		self.drawArc(cx, cy, radius, 0.0, 360.0, thickness, color)

	def fillCircle(self, cx, cy, radius, color):
		fr = (color >> 16) & 0xFF
		fg = (color >> 8) & 0xFF
		fb = color & 0xFF

		margin = 1
		xmin = int(cx - radius - margin)
		xmax = int(cx + radius + margin)
		ymin = int(cy - radius - margin)
		ymax = int(cy + radius + margin)
		if xmin < 0: xmin = 0
		if ymin < 0: ymin = 0
		if xmax >= self.WIDTH:  xmax = self.WIDTH - 1
		if ymax >= self.HEIGHT: ymax = self.HEIGHT - 1

		for y in range(ymin, ymax + 1):
			for x in range(xmin, xmax + 1):
				px = x + 0.5 - (cx + 0.5)
				py = y + 0.5 - (cy + 0.5)
				dist = math.sqrt(px * px + py * py)
				r = dist - radius
				if r <= -1.0:
					alpha = 1.0
				elif r >= 1.0:
					alpha = 0.0
				else:
					alpha = 1.0 - (r + 1.0) * 0.5
				if alpha <= 0.0:
					continue
				idx = (y * self.WIDTH + x) * 4
				self._blend(idx, fr, fg, fb, alpha)

	def drawCurvedChar(self, cx, cy, radius, angle, char, font_filename, fontsize, rotation=0):
		scale = 4
		big_size = int(fontsize * scale)
		cache_key = (font_filename, big_size)
		font = self._font_cache.get(cache_key)
		if font is None:
			font = ImageFont.truetype(font_filename, big_size)
			self._font_cache[cache_key] = font
		img = Image.new('RGBA', (big_size * 2, big_size * 2), (0, 0, 0, 0))
		draw = ImageDraw.Draw(img)
		bbox = draw.textbbox((0, 0), char, font=font)
		tw = bbox[2] - bbox[0]
		th = bbox[3] - bbox[1]
		tx = (img.width - tw) // 2 - bbox[0]
		ty = (img.height - th) // 2 - bbox[1]
		draw.text((tx, ty), char, fill=(0, 0, 0, 255), font=font)
		rot_angle = -angle - 90.0 + rotation
		rotated = img.rotate(rot_angle, expand=False, resample=Image.BICUBIC)
		final_size = int(fontsize * 1.2)
		resized = rotated.resize((final_size, final_size), Image.LANCZOS)
		x = int(cx + radius * math.cos(math.radians(angle)) - final_size // 2)
		y = int(cy + radius * math.sin(math.radians(angle)) - final_size // 2)
		for iy in range(resized.height):
			for ix in range(resized.width):
				r, g, b, a = resized.getpixel((ix, iy))
				if a == 0:
					continue
				px = x + ix
				py = y + iy
				if 0 <= px < self.WIDTH and 0 <= py < self.HEIGHT:
					idx = (py * self.WIDTH + px) * 4
					self._blend(idx, r, g, b, a / 255.0)

	def save(self, filename):
		from PIL import Image
		img = Image.frombytes('RGBA', (self.WIDTH, self.HEIGHT), bytes(self.buffer))
		img.save(filename)