import math
import numpy as np
from PIL import Image, ImageDraw, ImageFont


class Display:
	_font_cache = {}
	_char_cache = {}

	def __init__(self, size=240):
		self.WIDTH = size
		self.HEIGHT = size
		self.buffer = np.zeros((self.HEIGHT, self.WIDTH, 4), dtype=np.uint8)
		self.clear()

	def clear(self):
		self.buffer[:] = 0

	def fill(self, color=0x000000):
		r = (color >> 16) & 0xFF
		g = (color >> 8) & 0xFF
		b = color & 0xFF
		self.buffer[:] = (r, g, b, 255)

	# ------------------------------------------------------------------
	#  compositing helpers (vectorized)
	# ------------------------------------------------------------------

	def _blendView(self, view, fr, fg, fb, alpha):
		"""Alpha-blend a flat color into a (h, w, 4) view. view is modified in place."""
		touched = alpha > 0.0
		if not touched.any():
			return
		dst_a = view[..., 3].astype(np.float32) / 255.0
		out_a = alpha + dst_a * (1.0 - alpha)
		safe = np.where(out_a > 1e-6, out_a, 1.0)
		w_src = alpha / safe
		w_dst = dst_a * (1.0 - alpha) / safe
		out_r = w_src * fr + w_dst * view[..., 0].astype(np.float32)
		out_g = w_src * fg + w_dst * view[..., 1].astype(np.float32)
		out_b = w_src * fb + w_dst * view[..., 2].astype(np.float32)
		np.copyto(view[..., 0], np.clip(out_r + 0.5, 0, 255).astype(np.uint8), where=touched)
		np.copyto(view[..., 1], np.clip(out_g + 0.5, 0, 255).astype(np.uint8), where=touched)
		np.copyto(view[..., 2], np.clip(out_b + 0.5, 0, 255).astype(np.uint8), where=touched)
		np.copyto(view[..., 3], np.clip(out_a * 255.0 + 0.5, 0, 255).astype(np.uint8), where=touched)

	def _blendImage(self, view, src):
		"""Alpha-blend an RGBA source array (h, w, 4) into an RGBA view of the same shape."""
		src_a = src[..., 3].astype(np.float32) / 255.0
		touched = src_a > 0.0
		if not touched.any():
			return
		dst_a = view[..., 3].astype(np.float32) / 255.0
		out_a = src_a + dst_a * (1.0 - src_a)
		safe = np.where(out_a > 1e-6, out_a, 1.0)
		w_src = src_a / safe
		w_dst = dst_a * (1.0 - src_a) / safe
		out_r = w_src * src[..., 0].astype(np.float32) + w_dst * view[..., 0].astype(np.float32)
		out_g = w_src * src[..., 1].astype(np.float32) + w_dst * view[..., 1].astype(np.float32)
		out_b = w_src * src[..., 2].astype(np.float32) + w_dst * view[..., 2].astype(np.float32)
		np.copyto(view[..., 0], np.clip(out_r + 0.5, 0, 255).astype(np.uint8), where=touched)
		np.copyto(view[..., 1], np.clip(out_g + 0.5, 0, 255).astype(np.uint8), where=touched)
		np.copyto(view[..., 2], np.clip(out_b + 0.5, 0, 255).astype(np.uint8), where=touched)
		np.copyto(view[..., 3], np.clip(out_a * 255.0 + 0.5, 0, 255).astype(np.uint8), where=touched)

	# ------------------------------------------------------------------
	#  primitives
	# ------------------------------------------------------------------

	def drawLine(self, x1, y1, x2, y2, thickness, color):
		hw = thickness * 0.5
		fr = (color >> 16) & 0xFF
		fg = (color >> 8) & 0xFF
		fb = color & 0xFF

		m = hw + 2.0
		xmin = max(0, int(min(x1, x2) - m))
		xmax = min(self.WIDTH - 1, int(max(x1, x2) + m))
		ymin = max(0, int(min(y1, y2) - m))
		ymax = min(self.HEIGHT - 1, int(max(y1, y2) + m))
		if xmin > xmax or ymin > ymax:
			return

		pxg = (np.arange(xmin, xmax + 1, dtype=np.float32) + 0.5)[None, :]
		pyg = (np.arange(ymin, ymax + 1, dtype=np.float32) + 0.5)[:, None]

		sx1 = np.float32(x1 + 0.5)
		sy1 = np.float32(y1 + 0.5)
		sx2 = np.float32(x2 + 0.5)
		sy2 = np.float32(y2 + 0.5)
		segdx = sx2 - sx1
		segdy = sy2 - sy1
		L2 = segdx * segdx + segdy * segdy

		if L2 <= 1e-8:
			dist = np.sqrt((pxg - sx1) ** 2 + (pyg - sy1) ** 2)
		else:
			t = np.clip(((pxg - sx1) * segdx + (pyg - sy1) * segdy) / L2, 0.0, 1.0)
			qx = sx1 + t * segdx
			qy = sy1 + t * segdy
			dist = np.sqrt((pxg - qx) ** 2 + (pyg - qy) ** 2)

		r = dist - hw
		alpha = np.clip((1.0 - r) * 0.5, 0.0, 1.0)
		view = self.buffer[ymin:ymax + 1, xmin:xmax + 1]
		self._blendView(view, fr, fg, fb, alpha)

	def drawArc(self, cx, cy, radius, startAngle, endAngle, thickness, color, stripe=False, stripeColor=0xffffff):
		hw = thickness * 0.5
		fr = (color >> 16) & 0xFF
		fg = (color >> 8) & 0xFF
		fb = color & 0xFF
		radius_f = np.float32(radius)
		hw_f = np.float32(hw)

		m = hw + 2.0
		xmin = max(0, int(cx - radius - m))
		xmax = min(self.WIDTH - 1, int(cx + radius + m))
		ymin = max(0, int(cy - radius - m))
		ymax = min(self.HEIGHT - 1, int(cy + radius + m))
		if xmin > xmax or ymin > ymax:
			return

		pxg = (np.arange(xmin, xmax + 1) - cx).astype(np.float32)[None, :]
		pyg = (np.arange(ymin, ymax + 1) - cy).astype(np.float32)[:, None]

		r = np.sqrt(pxg * pxg + pyg * pyg)
		dr = np.abs(r - radius_f) - hw_f

		span_deg = endAngle - startAngle
		if span_deg >= 360.0:
			alpha = np.clip((1.0 - dr) * 0.5, 0.0, 1.0)
		else:
			two_pi = np.float32(2.0 * np.pi)
			sr = np.float32(np.radians(startAngle)) % two_pi
			er = np.float32(np.radians(endAngle)) % two_pi

			theta = np.arctan2(pyg, pxg) % two_pi
			if sr <= er:
				in_wedge = (theta >= sr) & (theta <= er)
			else:
				in_wedge = (theta >= sr) | (theta <= er)

			innerR = radius_f - hw_f
			outerR = radius_f + hw_f
			cs, sn = np.float32(np.cos(sr)), np.float32(np.sin(sr))
			ce, se = np.float32(np.cos(er)), np.float32(np.sin(er))

			ax1 = innerR * cs
			ay1 = innerR * sn
			ax2 = outerR * cs
			ay2 = outerR * sn
			adx = ax2 - ax1
			ady = ay2 - ay1
			aL2 = adx * adx + ady * ady
			at = np.clip(((pxg - ax1) * adx + (pyg - ay1) * ady) / aL2, 0.0, 1.0)
			aqx = ax1 + at * adx
			aqy = ay1 + at * ady
			d_cap_a = np.sqrt((pxg - aqx) ** 2 + (pyg - aqy) ** 2)

			bx1 = innerR * ce
			by1 = innerR * se
			bx2 = outerR * ce
			by2 = outerR * se
			bdx = bx2 - bx1
			bdy = by2 - by1
			bL2 = bdx * bdx + bdy * bdy
			bt = np.clip(((pxg - bx1) * bdx + (pyg - by1) * bdy) / bL2, 0.0, 1.0)
			bqx = bx1 + bt * bdx
			bqy = by1 + bt * bdy
			d_cap_b = np.sqrt((pxg - bqx) ** 2 + (pyg - bqy) ** 2)

			cap_sdf = np.minimum(d_cap_a, d_cap_b)
			sdf = np.where(in_wedge, dr, cap_sdf)
			alpha = np.clip((1.0 - sdf) * 0.5, 0.0, 1.0)

		view = self.buffer[ymin:ymax + 1, xmin:xmax + 1]
		self._blendView(view, fr, fg, fb, alpha)

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
		radius_f = np.float32(radius)

		margin = 2
		xmin = max(0, int(cx - radius - margin))
		xmax = min(self.WIDTH - 1, int(cx + radius + margin))
		ymin = max(0, int(cy - radius - margin))
		ymax = min(self.HEIGHT - 1, int(cy + radius + margin))
		if xmin > xmax or ymin > ymax:
			return

		pxg = (np.arange(xmin, xmax + 1) - cx).astype(np.float32)[None, :]
		pyg = (np.arange(ymin, ymax + 1) - cy).astype(np.float32)[:, None]

		dist = np.sqrt(pxg * pxg + pyg * pyg)
		r = dist - radius_f
		alpha = np.clip((1.0 - r) * 0.5, 0.0, 1.0)

		view = self.buffer[ymin:ymax + 1, xmin:xmax + 1]
		self._blendView(view, fr, fg, fb, alpha)

	def _renderChar(self, char, font_filename, fontsize, rotation):
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
		rot_angle = -rotation
		rotated = img.rotate(rot_angle, expand=False, resample=Image.BICUBIC)
		final_size = int(fontsize * 1.2)
		resized = rotated.resize((final_size, final_size), Image.LANCZOS)
		return np.array(resized, dtype=np.uint8)

	def drawCurvedChar(self, cx, cy, radius, angle, char, font_filename, fontsize, rotation=0):
		render_rot = angle + 90.0 - rotation
		cache_key = (char, font_filename, fontsize, render_rot)
		img = Display._char_cache.get(cache_key)
		if img is None:
			img = self._renderChar(char, font_filename, fontsize, render_rot)
			Display._char_cache[cache_key] = img

		h, w = img.shape[:2]
		x = int(cx + radius * math.cos(math.radians(angle)) - w // 2)
		y = int(cy + radius * math.sin(math.radians(angle)) - h // 2)

		x0 = max(0, x)
		y0 = max(0, y)
		x1 = min(self.WIDTH, x + w)
		y1 = min(self.HEIGHT, y + h)
		if x0 >= x1 or y0 >= y1:
			return

		view = self.buffer[y0:y1, x0:x1]
		src = img[y0 - y:y1 - y, x0 - x:x1 - x]
		self._blendImage(view, src)

	def save(self, filename):
		Image.fromarray(self.buffer, 'RGBA').save(filename)