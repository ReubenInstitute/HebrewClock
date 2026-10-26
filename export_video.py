import os
import sys
import time
import argparse
import ffmpeg
from PIL import Image, ImageDraw, ImageFont, ImageFilter

from Date import Date, Time, DateTime, WeekdayNames, MonthNames
from Geo import Place
from HebrewClock import Custom
from HebrewClock import StandardClock, TIMEZONE
from HebrewClock import Display
from HebrewDate import HebrewDate, HebrewWeekdayDayNames, HebrewWeekdayNightNames, HebrewMonthNames
from HebrewClock import HebrewYemama
from Hebrew import Number
from HebrewDate import HebrewHoliday, HebrewHolidayId, HolidayNames

from pathlib import Path

ROOT = Path(__file__).parent

LOCATION = Place.JERUSALEM.location()
PRESET = Custom.LOCAL
FPS = 30
STEP_SEC = 60

W, H = 720, 1280
CLOCK_SIZE = 403
CLOCK_POS = (158, 429)

font_en = ImageFont.truetype(ROOT / "../Fonts/Assistant-Regular.ttf", 28)
font_en = ImageFont.truetype(ROOT / "../Fonts/Assistant-Bold.ttf", 28)
#font_he = ImageFont.truetype(ROOT / "../Fonts/OpenSansHebrewCondensed-Regular.ttf", 28)
font_he = ImageFont.truetype(ROOT / "../Fonts/OpenSansHebrew-Regular.ttf", 25)
font_he = ImageFont.truetype(ROOT / "../Fonts/OpenSansHebrew-Bold.ttf", 25)


# Drop-shadow look for text labels: soft blur, no dilation (avoids a "boxy" halo).
SHADOW_OFFSET = (3, 3)
SHADOW_BLUR_RADIUS = 3
SHADOW_OPACITY = 170	# 0-255
SHADOW_PAD = SHADOW_BLUR_RADIUS * 3 + max(abs(SHADOW_OFFSET[0]), abs(SHADOW_OFFSET[1]))

# Label colors, chosen to read clearly over the bluish fog background video.
COLOR_HEBREW_DATE = (255, 205, 60)		# warm yellow
COLOR_HOLIDAY = (255, 70, 70)			# bright coral-red (brighter than usual so it still pops with a black shadow)
COLOR_GREGORIAN_DATE = (150, 235, 165)	# light mint green


def add_shadow(image):
	"""Return a bigger RGBA image: a soft black shadow, offset behind the given transparent cutout."""
	ox, oy = SHADOW_OFFSET
	pad = SHADOW_PAD
	w, h = image.size
	canvas = Image.new('RGBA', (w + pad * 2, h + pad * 2), (0, 0, 0, 0))

	shadow_mask = Image.new('L', canvas.size, 0)
	shadow_mask.paste(image.getchannel('A'), (pad + ox, pad + oy))
	shadow_mask = shadow_mask.filter(ImageFilter.GaussianBlur(SHADOW_BLUR_RADIUS))
	shadow_mask = shadow_mask.point(lambda a: a * SHADOW_OPACITY // 255)

	shadow_layer = Image.new('RGBA', canvas.size, (0, 0, 0, 0))
	shadow_layer.putalpha(shadow_mask)

	canvas.alpha_composite(shadow_layer)
	canvas.alpha_composite(image, (pad, pad))
	return canvas


def draw_text_with_shadow(canvas, xy, text, font, fill, anchor):
	"""Draw text onto canvas with a drop shadow, using the same xy/anchor semantics as ImageDraw.text."""
	x0, y0, x1, y1 = ImageDraw.Draw(canvas).textbbox(xy, text, font=font, anchor=anchor)
	temp = Image.new('RGBA', (x1 - x0, y1 - y0), (0, 0, 0, 0))
	ImageDraw.Draw(temp).text((xy[0] - x0, xy[1] - y0), text, fill=fill, font=font, anchor=anchor)
	shadowed = add_shadow(temp)
	canvas.paste(shadowed, (x0 - SHADOW_PAD, y0 - SHADOW_PAD), shadowed)


parser = argparse.ArgumentParser(description='Export a week-long yemama video')
parser.add_argument('date', help='yyyymmdd — a Saturday')
parser.add_argument('--test', action='store_true',
					help='Only render probes at 12:00 and 23:00; skip video assembly')
args = parser.parse_args()

input_str = args.date
test = args.test
year = int(input_str[:4])
month = int(input_str[4:6])
day = int(input_str[6:8])

input_date = Date(year, month, day)
target_hebrew = HebrewDate.fromdate(input_date + 1)
anchor = HebrewYemama(target_hebrew, LOCATION, Custom(PRESET))

start_utc = anchor.yemamaStart + (-2 * 3600)

yemamas = [anchor]
for _ in range(7):
	yemamas.append(yemamas[-1].nextYemama)

end_utc = yemamas[-1].yemamaEnd + (2 * 3600)

start = start_utc + TIMEZONE
start = DateTime(start.date, Time(start.time.hour, start.time.minute, 0))
end = end_utc + TIMEZONE

frames_dir = f"frames_{input_str}"
out_video = f"yemama_{input_str}.mp4"
music_file = f"{input_str}.mp3"		  # background track named by date

os.makedirs(frames_dir, exist_ok=True)

current = start
if test:
	# Snap to the first 12:00 or 23:00 mark at or after start
	noon = DateTime(current.date, Time(12, 0, 0))
	night = DateTime(current.date, Time(23, 0, 0))
	if current <= noon:
		current = noon
	elif current <= night:
		current = night
	else:
		current = DateTime(current.date.next, Time(12, 0, 0))
frame_no = 0
yemama_idx = 0

# Render clock at native 240px, then upscale to CLOCK_SIZE when compositing
clock = StandardClock(current, LOCATION, Custom(PRESET), size=240)

while current <= end:# and frame_no < 150:

	fname = os.path.join(frames_dir, f"frame_{frame_no:05d}.png")
	if test or not os.path.exists(fname):
#	if True:
		disp = Display(240)
		#disp.fill(0xffffff)
		dt = current
		t0 = time.time()
		clock.update(dt)
		clock.draw(disp)
		t1 = time.time()

		clock_img = Image.fromarray(disp.buffer, 'RGBA')
		clock_img = clock_img.resize((CLOCK_SIZE, CLOCK_SIZE), Image.LANCZOS)

		if test:
			canvas = Image.new('RGBA', (W, H), (0xff, 0xff, 0xff, 0xff))
		else:
			canvas = Image.new('RGBA', (W, H), (0, 0, 0, 0))

		canvas.paste(clock_img, CLOCK_POS, clock_img)

		# Walk the yemama chain forward as time crosses each nightfall
		while (yemama_idx + 1 < len(yemamas) and
			   dt >= yemamas[yemama_idx + 1].yemamaStart + TIMEZONE):
			yemama_idx += 1

		yemama = yemamas[yemama_idx]
		hebrew_date = yemama.hebrewDate

		# Hour in the current yemama, using UTC time
		h = yemama.hour((dt + (-TIMEZONE)).time)
		weekday_names = HebrewWeekdayNightNames if h < 12 else HebrewWeekdayDayNames

		# Hebrew date – English (left) and Hebrew (right)
		draw_text_with_shadow(canvas, (158, 179), weekday_names('en', hebrew_date.dayofweek).name(),
				  fill=COLOR_HEBREW_DATE, anchor="la", font=font_en)
		draw_text_with_shadow(canvas, (158, 219), f"{hebrew_date.day} {HebrewMonthNames('en', hebrew_date.year, hebrew_date.month).name()} {hebrew_date.year}",
				  fill=COLOR_HEBREW_DATE, anchor="la", font=font_en)
		draw_text_with_shadow(canvas, (561, 179), weekday_names('he', hebrew_date.dayofweek).name(),
				  fill=COLOR_HEBREW_DATE, anchor="ra", font=font_he)
		draw_text_with_shadow(canvas, (561, 219), f"{Number(hebrew_date.day).fancy_text} {HebrewMonthNames('he', hebrew_date.year, hebrew_date.month).name()} {Number(hebrew_date.year).fancy_text}",
				  fill=COLOR_HEBREW_DATE, anchor="ra", font=font_he)


		holiday = HebrewHoliday(hebrew_date)
		if holiday.id != HebrewHolidayId.NONE:
			draw_text_with_shadow(canvas, (158, 259), HolidayNames('en', holiday.id).name(),
					fill=COLOR_HOLIDAY, anchor="la", font=font_en)
			draw_text_with_shadow(canvas, (561, 259), HolidayNames('he', holiday.id).name(),
					fill=COLOR_HOLIDAY, anchor="ra", font=font_he)


		# Gregorian date – English (left) and Hebrew (right)
		draw_text_with_shadow(canvas, (158, 299), WeekdayNames('en', dt.date.dayofweek).name(),
				  fill=COLOR_GREGORIAN_DATE, anchor="la", font=font_en)
		draw_text_with_shadow(canvas, (158, 339), f"{dt.date.day} {MonthNames('en', dt.date.month).name()}",
				  fill=COLOR_GREGORIAN_DATE, anchor="la", font=font_en)
		draw_text_with_shadow(canvas, (158, 379), f"{dt.date.year}",
				  fill=COLOR_GREGORIAN_DATE, anchor="la", font=font_en)
		draw_text_with_shadow(canvas, (561, 299), WeekdayNames('he', dt.date.dayofweek).name(),
				  fill=COLOR_GREGORIAN_DATE, anchor="ra", font=font_he)
		draw_text_with_shadow(canvas, (561, 339), f"{dt.date.day} {MonthNames('he', dt.date.month).name()}",
				  fill=COLOR_GREGORIAN_DATE, anchor="ra", font=font_he)
		draw_text_with_shadow(canvas, (561, 379), f"{dt.date.year}",
				  fill=COLOR_GREGORIAN_DATE, anchor="ra", font=font_he)

		canvas.save(fname)

		elapsed = t1 - t0
		print(f"Frame {frame_no:4d}  {dt.date.year:04d}-{dt.date.month:02d}-{dt.date.day:02d} {dt.time.hour:02d}:{dt.time.minute:02d}:{dt.time.second:02d}  time={elapsed:.3f}s")
	else:
		print(f"Skip Frame {frame_no:4d}")

	if test:
		if current.time.serial == 12 * 3600:
			current = DateTime(current.date, Time(23, 0, 0))
		else:
			current = DateTime(current.date.next, Time(12, 0, 0))
	else:
		current = current + STEP_SEC
	frame_no += 1

if test:
	print(f"Test mode: wrote {frame_no} probe frames to {frames_dir}/. Skipping video assembly.")
	sys.exit(0)

# ----- Assemble final video with background music -----
video_duration = frame_no / FPS

if not os.path.exists(music_file):
	raise FileNotFoundError(f"Missing background music: {music_file}")

background = ffmpeg.input('back-blue.mp4', stream_loop=-1)
overlay = ffmpeg.input(os.path.join(frames_dir, 'frame_%05d.png'), framerate=FPS)
music = ffmpeg.input(music_file)

video = ffmpeg.filter([background, overlay], 'overlay', shortest=1)

video = (
	video
	.filter('fade', t='in', st=0, d=0.5)
	.filter('fade', t='out', st=max(0, video_duration - 0.5), d=0.5)
)

audio = music.audio
audio = audio.filter('loudnorm', I=-16, LRA=11, TP=-1.5)
audio = audio.filter('afade', t='in', st=0, d=0.5)
audio = audio.filter('afade', t='out', st=max(0, video_duration - 1.5), d=1.5)

(
	ffmpeg
	.output(video, audio, out_video,
			vcodec='libx264',
			pix_fmt='yuv420p',
			acodec='aac',
			t=video_duration)
	.overwrite_output()
	.run()
)

print(f"Video saved to {out_video} ({frame_no} frames, {video_duration:.1f}s)")