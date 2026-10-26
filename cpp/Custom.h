/* Custom.h
 * Ported from LIBS/Custom.py
 */
#ifndef CUSTOM_H
#define CUSTOM_H

enum class DayDefinition {
	GRA = 0,
	MA = 1
};

enum class DawnCustom {
	DEG_16_1 = 0,
	DEG_16_9 = 1,
	DEG_18 = 2,
	DEG_19_8 = 3,
	MIN_72 = 4,
	REL_MIN_72 = 5
};

enum class MisheyakirCustom {
	DEG_11_5 = 0,
	DEG_11 = 1,
	DEG_10_2 = 2,
	MIN_50 = 3,
	DEG_7_65 = 4
};

enum class NightfallCustom {
	DEG_5_95 = 0,
	DEG_6 = 1,
	DEG_6_45 = 2,
	DEG_7_1 = 3,
	DEG_8 = 4,
	DEG_8_5 = 5,
	MIN_72 = 6,
	REL_MIN_72 = 7,
	DEG_16_1 = 8,
	REL_MIN_90 = 9,
	DEG_18 = 10,
	DEG_19_8 = 11
};

class Custom {
public:
	/* Python has these as plain int class attributes on Custom itself
	 * (Custom.LOCAL, Custom.ASHKENAZI_GRA, ...), used as preset-table
	 * selector/keys rather than as a standalone named-value domain (unlike
	 * DayDefinition/DawnCustom/etc, which are real Python IntEnums above).
	 * Ported as a plain (unscoped) enum nested *inside* Custom rather than
	 * `enum class`, specifically so `Custom::LOCAL` reads exactly like
	 * Python's `Custom.LOCAL` and so it implicitly converts to int for use
	 * as a PRESET_TABLE index -- a deliberate exception to the usual
	 * enum-class convention, noted here since it departs from the pattern. */
	enum Preset {
		LOCAL = 0,
		ASHKENAZI_GRA = 1,
		SEPHARDI = 2,
		CHABAD = 3,
		MAGEN_AVRAHAM = 4,
		RABBEINU_TAM_FIXED = 5,
		RABBEINU_TAM_SEASONAL = 6,
		RABBEINU_TAM_STRINGENT_90 = 7,
		BEN_ISH_CHAI = 8,
		WESTERN_EUROPEAN = 9
	};

	DayDefinition dayDefinition;
	DawnCustom dawn;
	MisheyakirCustom misheyakir;
	NightfallCustom nightfall;
	int watches;

	Custom(Preset custom = LOCAL, int watches = 3);

	void setCustom(Preset custom);

private:
	struct PresetValues {
		DayDefinition dayDef;
		DawnCustom dawn;
		MisheyakirCustom misheyakir;
		NightfallCustom nightfall;
	};

	static const PresetValues PRESET_TABLE[10];
};

#endif
