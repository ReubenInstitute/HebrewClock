/* HebrewHoliday.cpp
 * Ported from LIBS/HebrewHoliday.py
 */
#include "HebrewHoliday.h"

HebrewHoliday::HebrewHoliday(HebrewDate hebrewDate)
	: hebrewDate(hebrewDate)
{
	int year = hebrewDate.year;
	int month = hebrewDate.month;
	int day = hebrewDate.day;

	id = HebrewHolidayId::NONE;

	if (month == static_cast<int>(HebrewMonth::NISAN)) {
		if (day == 15) {
			id = HebrewHolidayId::PESACH;
		} else if (day == 21) {
			id = HebrewHolidayId::PESACH_VII;
		} else {
			/* 27 Nisan can only ever fall on Sun/Tue/Thu/Fri.
			 * Friday -> advance to Thursday (26); Sunday -> postpone to Monday (28);
			 * Tue/Thu -> observed on the 27th itself. */
			HebrewDate testDate(year, static_cast<int>(HebrewMonth::NISAN), 27);
			if (testDate.dayofweek() == Weekday::FRIDAY) {
				if (day == 26) {
					id = HebrewHolidayId::YOM_HASHOAH;
				}
			} else if (year >= 5757 && testDate.dayofweek() == Weekday::SUNDAY) {
				if (day == 28) {
					id = HebrewHolidayId::YOM_HASHOAH;
				}
			} else if (day == 27) {
				id = HebrewHolidayId::YOM_HASHOAH;
			}
		}
	}

	if (month == static_cast<int>(HebrewMonth::IYAR)) {
		/* 4 Iyar can only ever fall on Sun/Tue/Thu/Fri.
		 * Tuesday-Wednesday: normative, no shift (4/5 Iyar).
		 * Thursday-Friday: advance one day to avoid running into Shabbat (3/4 Iyar).
		 * Friday-Saturday: advance two days (2/3 Iyar).
		 * Sunday-Monday (since 5764): postpone one day to avoid Motzei-Shabbat
		 *   preparations (5/6 Iyar); before the reform, observed 4/5 Iyar as normal. */
		Weekday nominal4 = HebrewDate(year, static_cast<int>(HebrewMonth::IYAR), 4).dayofweek();
		int zikaronDay, atzmautDay;
		if (nominal4 == Weekday::THURSDAY) {
			zikaronDay = 3; atzmautDay = 4;
		} else if (nominal4 == Weekday::FRIDAY) {
			zikaronDay = 2; atzmautDay = 3;
		} else if (nominal4 == Weekday::SUNDAY && year >= 5764) {
			zikaronDay = 5; atzmautDay = 6;
		} else {
			zikaronDay = 4; atzmautDay = 5;
		}

		if (day == zikaronDay) {
			id = HebrewHolidayId::YOM_HAZIKARON;
		} else if (day == atzmautDay) {
			id = HebrewHolidayId::YOM_HAATZMAUT;
		}

		if (day == 14) {
			id = HebrewHolidayId::PESACH_SHENI;
		} else if (day == 18) {
			id = HebrewHolidayId::LAG_BOMER;
		} else if (day == 28) {
			id = HebrewHolidayId::YOM_YERUSHALAYIM;
		}
	}

	if (month == static_cast<int>(HebrewMonth::SIVAN)) {
		if (day == 6) {
			id = HebrewHolidayId::SHAVUOT;
		}
	}

	if (month == static_cast<int>(HebrewMonth::TAMMUZ)) {
		/* 17 Tammuz never falls on Friday, but can fall on Shabbat; postpone to Sunday. */
		HebrewDate fastDate(year, static_cast<int>(HebrewMonth::TAMMUZ), 17);
		if (fastDate.dayofweek() == Weekday::SATURDAY) {
			if (day == 18) {
				id = HebrewHolidayId::FAST_OF_TAMMUZ;
			}
		} else if (day == 17) {
			id = HebrewHolidayId::FAST_OF_TAMMUZ;
		}
	}

	if (month == static_cast<int>(HebrewMonth::AV)) {
		/* 9 Av never falls on Friday, but can fall on Shabbat; postpone to Sunday. */
		HebrewDate fastDate(year, static_cast<int>(HebrewMonth::AV), 9);
		if (fastDate.dayofweek() == Weekday::SATURDAY) {
			if (day == 10) {
				id = HebrewHolidayId::FAST_OF_AV;
			}
		} else if (day == 9) {
			id = HebrewHolidayId::FAST_OF_AV;
		}
		if (day == 15) {
			id = HebrewHolidayId::TU_BAV;
		}
	}

	if (month == static_cast<int>(HebrewMonth::TISHREI)) {
		if (day == 1) {
			id = HebrewHolidayId::ROSH_HASHANA;
		} else if (day == 2) {
			id = HebrewHolidayId::ROSH_HASHANA_II;
		} else {
			/* 3 Tishrei never falls on Friday, but can fall on Shabbat; postpone to Sunday. */
			HebrewDate tzomDate(year, static_cast<int>(HebrewMonth::TISHREI), 3);
			if (tzomDate.dayofweek() == Weekday::SATURDAY) {
				if (day == 4) {
					id = HebrewHolidayId::TZOM_GEDALIAH;
				}
			} else if (day == 3) {
				id = HebrewHolidayId::TZOM_GEDALIAH;
			}

			if (day == 10) {
				id = HebrewHolidayId::YOM_KIPPUR;
			} else if (day == 15) {
				id = HebrewHolidayId::SUKKOT;
			} else if (day == 21) {
				id = HebrewHolidayId::HOSHANA_RABA;
			} else if (day == 22) {
				id = HebrewHolidayId::SIMCHAT_TORAH_SHEMINI_ATZERETH;
			}
		}
	}

	if (month == static_cast<int>(HebrewMonth::KISLEV) || month == static_cast<int>(HebrewMonth::TEVET)) {
		if (month == static_cast<int>(HebrewMonth::KISLEV) && day >= 25) {
			id = static_cast<HebrewHolidayId>(static_cast<int>(HebrewHolidayId::CHANUKKA_I) + (day - 25));
		}

		if (month == static_cast<int>(HebrewMonth::TEVET)) {
			if (day == 10) {
				id = HebrewHolidayId::FAST_OF_TEVET;
			}
			int kislevDays = HebrewDate::numdaysinmonth(year, static_cast<int>(HebrewMonth::KISLEV));
			if (kislevDays == 29 && day <= 3) {
				id = static_cast<HebrewHolidayId>(static_cast<int>(HebrewHolidayId::CHANUKKA_I) + (4 + day));
			}
			if (kislevDays == 30 && day <= 2) {
				id = static_cast<HebrewHolidayId>(static_cast<int>(HebrewHolidayId::CHANUKKA_I) + (5 + day));
			}
		}
	}

	if (month == static_cast<int>(HebrewMonth::SHEVAT) && day == 15) {
		id = HebrewHolidayId::TU_BSHEVAT;
	}

	int monthEsther = HebrewDate::numMonthInYear(year);
	if (month == monthEsther) {
		/* 13 Adar never falls on Friday, but can fall on Shabbat; advance to Thursday. */
		HebrewDate fastDate(year, monthEsther, 13);
		if (fastDate.dayofweek() == Weekday::SATURDAY) {
			if (day == 11) {
				id = HebrewHolidayId::FAST_OF_ESTHER;
			}
		} else if (day == 13) {
			id = HebrewHolidayId::FAST_OF_ESTHER;
		}
		if (day == 14) {
			id = HebrewHolidayId::PURIM;
		} else if (day == 15) {
			id = HebrewHolidayId::SHUSHAN_PURIM;
		}
	}
}

bool HebrewHoliday::is_shabbat() const
{
	return hebrewDate.dayofweek() == Weekday::SATURDAY;
}

bool HebrewHoliday::is_rest_day() const
{
	if (hebrewDate.dayofweek() == Weekday::SATURDAY) {
		return true;
	}
	return id == HebrewHolidayId::ROSH_HASHANA ||
		id == HebrewHolidayId::ROSH_HASHANA_II ||
		id == HebrewHolidayId::YOM_KIPPUR ||
		id == HebrewHolidayId::SUKKOT ||
		id == HebrewHolidayId::PESACH ||
		id == HebrewHolidayId::PESACH_VII ||
		id == HebrewHolidayId::SHAVUOT ||
		id == HebrewHolidayId::SIMCHAT_TORAH_SHEMINI_ATZERETH;
}

bool HebrewHoliday::is_festive() const
{
	return id == HebrewHolidayId::PURIM ||
		id == HebrewHolidayId::SHUSHAN_PURIM ||
		id == HebrewHolidayId::LAG_BOMER ||
		id == HebrewHolidayId::TU_BAV ||
		id == HebrewHolidayId::CHANUKKA_I ||
		id == HebrewHolidayId::CHANUKKA_II ||
		id == HebrewHolidayId::CHANUKKA_III ||
		id == HebrewHolidayId::CHANUKKA_IV ||
		id == HebrewHolidayId::CHANUKKA_V ||
		id == HebrewHolidayId::CHANUKKA_VI ||
		id == HebrewHolidayId::CHANUKKA_VII ||
		id == HebrewHolidayId::CHANUKKA_VIII;
}

bool HebrewHoliday::is_fast() const
{
	return id == HebrewHolidayId::FAST_OF_TAMMUZ ||
		id == HebrewHolidayId::FAST_OF_AV ||
		id == HebrewHolidayId::TZOM_GEDALIAH ||
		id == HebrewHolidayId::FAST_OF_TEVET ||
		id == HebrewHolidayId::FAST_OF_ESTHER ||
		id == HebrewHolidayId::YOM_KIPPUR;
}

bool HebrewHoliday::is_national() const
{
	return id == HebrewHolidayId::YOM_HASHOAH ||
		id == HebrewHolidayId::YOM_HAZIKARON ||
		id == HebrewHolidayId::YOM_HAATZMAUT ||
		id == HebrewHolidayId::YOM_YERUSHALAYIM;
}

bool HebrewHoliday::is_solemn() const
{
	return id == HebrewHolidayId::YOM_HASHOAH || id == HebrewHolidayId::YOM_HAZIKARON;
}

bool HebrewHoliday::moed(HebrewHoliday& out) const
{
	int month = hebrewDate.month;
	int day = hebrewDate.day;
	if (month == static_cast<int>(HebrewMonth::NISAN) && day >= 15 && day <= 21) {
		out = HebrewHoliday(HebrewDate(hebrewDate.year, static_cast<int>(HebrewMonth::NISAN), 15));
		return true;
	}
	if (month == static_cast<int>(HebrewMonth::TISHREI) && day >= 15 && day <= 21) {
		out = HebrewHoliday(HebrewDate(hebrewDate.year, static_cast<int>(HebrewMonth::TISHREI), 15));
		return true;
	}
	return false;
}

bool HebrewHoliday::erev(HebrewHoliday& out) const
{
	HebrewHoliday nextHoliday(hebrewDate.next());
	if (nextHoliday.id == HebrewHolidayId::PESACH ||
	    nextHoliday.id == HebrewHolidayId::SHAVUOT ||
	    nextHoliday.id == HebrewHolidayId::ROSH_HASHANA ||
	    nextHoliday.id == HebrewHolidayId::YOM_KIPPUR ||
	    nextHoliday.id == HebrewHolidayId::SUKKOT ||
	    nextHoliday.id == HebrewHolidayId::FAST_OF_AV ||
	    nextHoliday.id == HebrewHolidayId::YOM_HAZIKARON ||
	    nextHoliday.id == HebrewHolidayId::YOM_HAATZMAUT) {
		out = nextHoliday;
		return true;
	}
	return false;
}

HebrewHolidayId HebrewHoliday::diaspora() const
{
	int month = hebrewDate.month;
	int day = hebrewDate.day;

	if (month == static_cast<int>(HebrewMonth::NISAN)) {
		if (day == 16) {
			return HebrewHolidayId::PESACH_II;
		}
		if (day == 22) {
			return HebrewHolidayId::PESACH_VIII;
		}
	}

	if (month == static_cast<int>(HebrewMonth::SIVAN) && day == 7) {
		return HebrewHolidayId::SHAVUOT_II;
	}

	if (month == static_cast<int>(HebrewMonth::TISHREI) && day == 16) {
		return HebrewHolidayId::SUKKOT_II;
	}

	return id;
}

void HebrewHoliday::holidays(int year, HebrewDate out[HEBREW_HOLIDAY_MAX_PER_YEAR], int& count)
{
	HebrewDate first(year, static_cast<int>(HebrewMonth::TISHREI), 1);
	HebrewDate last(year + 1, static_cast<int>(HebrewMonth::TISHREI), 1);
	count = 0;
	HebrewDate h = first;
	while (h.ordinal < last.ordinal && count < HEBREW_HOLIDAY_MAX_PER_YEAR) {
		if (HebrewHoliday(h).id != HebrewHolidayId::NONE) {
			out[count] = h;
			count++;
		}
		h = h.next();
	}
}

/* ---------- HolidayNames ---------- */

const char* HolidayNames::NAMES_EN[39] = {
	"",
	"Pesach",
	"Pesach II",
	"Pesach VII",
	"Pesach VIII",
	"Yom HaShoah",
	"Yom HaZikaron",
	"Yom HaAtzmaut",
	"Pesach Sheni",
	"Lag BaOmer",
	"Yom Yerushalayim",
	"Shavuot",
	"Shavuot II",
	"Fast of Tammuz",
	"Fast of Av",
	"Tu B'Av",
	"Rosh Hashanah",
	"Rosh Hashanah II",
	"Fast of Gedaliah",
	"Yom Kippur",
	"Sukkot",
	"Sukkot II",
	"Hoshana Rabbah",
	"Shemini Atzeret",
	"Simchat Torah",
	"Simchat Torah / Shemini Atzeret",
	"Chanukkah I",
	"Chanukkah II",
	"Chanukkah III",
	"Chanukkah IV",
	"Chanukkah V",
	"Chanukkah VI",
	"Chanukkah VII",
	"Chanukkah VIII",
	"Fast of Tevet",
	"Tu BiShvat",
	"Fast of Esther",
	"Purim",
	"Shushan Purim"
};

const char* HolidayNames::NAMES_HE[39] = {
	"",
	"\xd7\xa4\xd7\xa1\xd7\x97",
	"\xd7\xa4\xd7\xa1\xd7\x97 \xd7\xa9\xd7\xa0\xd7\x99 (\xd7\x92\xd7\x9c\xd7\x95\xd7\xaa)",
	"\xd7\xa4\xd7\xa1\xd7\x97 \xd7\xa9\xd7\x91\xd7\x99\xd7\xa2\xd7\x99",
	"\xd7\xa4\xd7\xa1\xd7\x97 \xd7\xa9\xd7\x9e\xd7\x99\xd7\xa0\xd7\x99 (\xd7\x92\xd7\x9c\xd7\x95\xd7\xaa)",
	"\xd7\x99\xd7\x95\xd7\x9d \xd7\x94\xd7\xa9\xd7\x95\xd7\x90\xd7\x94",
	"\xd7\x99\xd7\x95\xd7\x9d \xd7\x94\xd7\x96\xd7\x99\xd7\x9b\xd7\xa8\xd7\x95\xd7\x9f",
	"\xd7\x99\xd7\x95\xd7\x9d \xd7\x94\xd7\xa2\xd7\xa6\xd7\x9e\xd7\x90\xd7\x95\xd7\xaa",
	"\xd7\xa4\xd7\xa1\xd7\x97 \xd7\xa9\xd7\xa0\xd7\x99",
	"\xd7\x9c\xd7\xb4\xd7\x92 \xd7\x91\xd7\xa2\xd7\x95\xd7\x9e\xd7\xa8",
	"\xd7\x99\xd7\x95\xd7\x9d \xd7\x99\xd7\xa8\xd7\x95\xd7\xa9\xd7\x9c\xd7\x99\xd7\x9d",
	"\xd7\xa9\xd7\x91\xd7\x95\xd7\xa2\xd7\x95\xd7\xaa",
	"\xd7\xa9\xd7\x91\xd7\x95\xd7\xa2\xd7\x95\xd7\xaa \xd7\xa9\xd7\xa0\xd7\x99 (\xd7\x92\xd7\x9c\xd7\x95\xd7\xaa)",
	"\xd7\xaa\xd7\xa2\xd7\xa0\xd7\x99\xd7\xaa \xd7\xaa\xd7\x9e\xd7\x95\xd7\x96",
	"\xd7\xaa\xd7\xa2\xd7\xa0\xd7\x99\xd7\xaa \xd7\x90\xd7\x91",
	"\xd7\x98\xd7\xb4\xd7\x95 \xd7\x91\xd7\x90\xd7\x91",
	"\xd7\xa8\xd7\x90\xd7\xa9 \xd7\x94\xd7\xa9\xd7\xa0\xd7\x94",
	"\xd7\xa8\xd7\x90\xd7\xa9 \xd7\x94\xd7\xa9\xd7\xa0\xd7\x94 \xd7\x91\xd7\xb3",
	"\xd7\xa6\xd7\x95\xd7\x9d \xd7\x92\xd7\x93\xd7\x9c\xd7\x99\xd7\x94",
	"\xd7\x99\xd7\x95\xd7\x9d \xd7\x9b\xd7\x99\xd7\xa4\xd7\x95\xd7\xa8",
	"\xd7\xa1\xd7\x95\xd7\x9b\xd7\x95\xd7\xaa",
	"\xd7\xa1\xd7\x95\xd7\x9b\xd7\x95\xd7\xaa \xd7\xa9\xd7\xa0\xd7\x99 (\xd7\x92\xd7\x9c\xd7\x95\xd7\xaa)",
	"\xd7\x94\xd7\x95\xd7\xa9\xd7\xa2\xd7\xa0\xd7\x90 \xd7\xa8\xd7\x91\xd7\x94",
	"\xd7\xa9\xd7\x9e\xd7\x99\xd7\xa0\xd7\x99 \xd7\xa2\xd7\xa6\xd7\xa8\xd7\xaa",
	"\xd7\xa9\xd7\x9e\xd7\x97\xd7\xaa \xd7\xaa\xd7\x95\xd7\xa8\xd7\x94",
	"\xd7\xa9\xd7\x9e\xd7\x97\xd7\xaa \xd7\xaa\xd7\x95\xd7\xa8\xd7\x94 / \xd7\xa9\xd7\x9e\xd7\x99\xd7\xa0\xd7\x99 \xd7\xa2\xd7\xa6\xd7\xa8\xd7\xaa",
	"\xd7\x97\xd7\xa0\xd7\x95\xd7\x9b\xd7\x94 \xd7\x90\xd7\xb3",
	"\xd7\x97\xd7\xa0\xd7\x95\xd7\x9b\xd7\x94 \xd7\x91\xd7\xb3",
	"\xd7\x97\xd7\xa0\xd7\x95\xd7\x9b\xd7\x94 \xd7\x92\xd7\xb3",
	"\xd7\x97\xd7\xa0\xd7\x95\xd7\x9b\xd7\x94 \xd7\x93\xd7\xb3",
	"\xd7\x97\xd7\xa0\xd7\x95\xd7\x9b\xd7\x94 \xd7\x94\xd7\xb3",
	"\xd7\x97\xd7\xa0\xd7\x95\xd7\x9b\xd7\x94 \xd7\x95\xd7\xb3",
	"\xd7\x97\xd7\xa0\xd7\x95\xd7\x9b\xd7\x94 \xd7\x96\xd7\xb3",
	"\xd7\x97\xd7\xa0\xd7\x95\xd7\x9b\xd7\x94 \xd7\x97\xd7\xb3",
	"\xd7\xaa\xd7\xa2\xd7\xa0\xd7\x99\xd7\xaa \xd7\x98\xd7\x91\xd7\xaa",
	"\xd7\x98\xd7\xb4\xd7\x95 \xd7\x91\xd7\xa9\xd7\x91\xd7\x98",
	"\xd7\xaa\xd7\xa2\xd7\xa0\xd7\x99\xd7\xaa \xd7\x90\xd7\xa1\xd7\xaa\xd7\xa8",
	"\xd7\xa4\xd7\x95\xd7\xa8\xd7\x99\xd7\x9d",
	"\xd7\xa9\xd7\x95\xd7\xa9\xd7\x9f \xd7\xa4\xd7\x95\xd7\xa8\xd7\x99\xd7\x9d"
};

HolidayNames::HolidayNames(Lang lang, HebrewHolidayId id)
{
	this->lang = lang;
	this->id = id;
}

const char* HolidayNames::name() const
{
	int idx = static_cast<int>(id);
	return (lang == Lang::EN) ? NAMES_EN[idx] : NAMES_HE[idx];
}
