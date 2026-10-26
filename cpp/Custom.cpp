/* Custom.cpp
 * Ported from LIBS/Custom.py
 */
#include "Custom.h"

Custom::Custom(Preset custom, int watches)
{
	dayDefinition = DayDefinition::MA;
	dawn = DawnCustom::DEG_16_1;
	misheyakir = MisheyakirCustom::MIN_50;
	nightfall = NightfallCustom::DEG_8_5;
	this->watches = watches;
	setCustom(custom);
}

void Custom::setCustom(Preset custom)
{
	if (custom < 0 || custom >= 10) {
		return;   /* mirrors Python's `if vals is None: return` for an unknown key */
	}
	const PresetValues& vals = PRESET_TABLE[custom];
	dayDefinition = vals.dayDef;
	dawn = vals.dawn;
	misheyakir = vals.misheyakir;
	nightfall = vals.nightfall;
}

const Custom::PresetValues Custom::PRESET_TABLE[10] = {
	/* LOCAL                     */ { DayDefinition::MA,  DawnCustom::DEG_16_1,   MisheyakirCustom::MIN_50,   NightfallCustom::DEG_8_5 },
	/* ASHKENAZI_GRA             */ { DayDefinition::GRA, DawnCustom::DEG_16_1,   MisheyakirCustom::DEG_11_5, NightfallCustom::DEG_8_5 },
	/* SEPHARDI                  */ { DayDefinition::GRA, DawnCustom::DEG_16_1,   MisheyakirCustom::DEG_11,   NightfallCustom::DEG_6_45 },
	/* CHABAD                    */ { DayDefinition::GRA, DawnCustom::DEG_16_9,   MisheyakirCustom::DEG_10_2, NightfallCustom::DEG_6 },
	/* MAGEN_AVRAHAM             */ { DayDefinition::MA,  DawnCustom::DEG_16_1,   MisheyakirCustom::DEG_11,   NightfallCustom::DEG_16_1 },
	/* RABBEINU_TAM_FIXED        */ { DayDefinition::MA,  DawnCustom::MIN_72,     MisheyakirCustom::MIN_50,   NightfallCustom::MIN_72 },
	/* RABBEINU_TAM_SEASONAL     */ { DayDefinition::MA,  DawnCustom::REL_MIN_72, MisheyakirCustom::MIN_50,   NightfallCustom::REL_MIN_72 },
	/* RABBEINU_TAM_STRINGENT_90 */ { DayDefinition::MA,  DawnCustom::REL_MIN_72, MisheyakirCustom::MIN_50,   NightfallCustom::REL_MIN_90 },
	/* BEN_ISH_CHAI              */ { DayDefinition::MA,  DawnCustom::DEG_19_8,   MisheyakirCustom::DEG_11,   NightfallCustom::DEG_7_1 },
	/* WESTERN_EUROPEAN          */ { DayDefinition::GRA, DawnCustom::DEG_18,     MisheyakirCustom::DEG_11_5, NightfallCustom::DEG_8_5 }
};
