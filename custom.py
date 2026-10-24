from enum import IntEnum


class DayDefinition(IntEnum):
	GRA = 0
	MA = 1


class DawnCustom(IntEnum):
	DEG_16_1 = 0
	DEG_16_9 = 1
	DEG_18 = 2
	DEG_19_8 = 3
	MIN_72 = 4
	REL_MIN_72 = 5


class MisheyakirCustom(IntEnum):
	DEG_11_5 = 0
	DEG_11 = 1
	DEG_10_2 = 2
	MIN_50 = 3
	DEG_7_65 = 4


class NightfallCustom(IntEnum):
	DEG_5_95 = 0
	DEG_6 = 1
	DEG_6_45 = 2
	DEG_7_1 = 3
	DEG_8 = 4
	DEG_8_5 = 5
	MIN_72 = 6
	REL_MIN_72 = 7
	DEG_16_1 = 8
	REL_MIN_90 = 9
	DEG_18 = 10
	DEG_19_8 = 11


class Custom:
	LOCAL = 0
	ASHKENAZI_GRA = 1
	SEPHARDI = 2
	CHABAD = 3
	MAGEN_AVRAHAM = 4
	RABBEINU_TAM_FIXED = 5
	RABBEINU_TAM_SEASONAL = 6
	RABBEINU_TAM_STRINGENT_90 = 7
	BEN_ISH_CHAI = 8
	WESTERN_EUROPEAN = 9


	def __init__(self, custom=None, watches=3):
		self.dayDefinition = DayDefinition.MA
		self.dawn = DawnCustom.DEG_16_1
		self.misheyakir = MisheyakirCustom.MIN_50
		self.nightfall = NightfallCustom.DEG_8_5
		self.watches = watches
		self.setCustom(Custom.LOCAL if custom is None else custom)

	def setCustom(self, custom):
		vals = Custom._PRESET_VALUES.get(custom)
		if vals is None:
			return
		self.dayDefinition = vals["dayDef"]
		self.dawn = vals["dawn"]
		self.misheyakir = vals["misheyakir"]
		self.nightfall = vals["nightfall"]


Custom._PRESET_VALUES = {
	Custom.LOCAL: {
		"dawn": DawnCustom.DEG_16_1,
		"misheyakir": MisheyakirCustom.MIN_50,
		"nightfall": NightfallCustom.DEG_8_5,
		"dayDef": DayDefinition.MA,
	},
	Custom.ASHKENAZI_GRA: {
		"dawn": DawnCustom.DEG_16_1,
		"misheyakir": MisheyakirCustom.DEG_11_5,
		"nightfall": NightfallCustom.DEG_8_5,
		"dayDef": DayDefinition.GRA,
	},
	Custom.SEPHARDI: {
		"dawn": DawnCustom.DEG_16_1,
		"misheyakir": MisheyakirCustom.DEG_11,
		"nightfall": NightfallCustom.DEG_6_45,
		"dayDef": DayDefinition.GRA,
	},
	Custom.CHABAD: {
		"dawn": DawnCustom.DEG_16_9,
		"misheyakir": MisheyakirCustom.DEG_10_2,
		"nightfall": NightfallCustom.DEG_6,
		"dayDef": DayDefinition.GRA,
	},
	Custom.MAGEN_AVRAHAM: {
		"dawn": DawnCustom.DEG_16_1,
		"misheyakir": MisheyakirCustom.DEG_11,
		"nightfall": NightfallCustom.DEG_16_1,
		"dayDef": DayDefinition.MA,
	},
	Custom.RABBEINU_TAM_FIXED: {
		"dawn": DawnCustom.MIN_72,
		"misheyakir": MisheyakirCustom.MIN_50,
		"nightfall": NightfallCustom.MIN_72,
		"dayDef": DayDefinition.MA,
	},
	Custom.RABBEINU_TAM_SEASONAL: {
		"dawn": DawnCustom.REL_MIN_72,
		"misheyakir": MisheyakirCustom.MIN_50,
		"nightfall": NightfallCustom.REL_MIN_72,
		"dayDef": DayDefinition.MA,
	},
	Custom.RABBEINU_TAM_STRINGENT_90: {
		"dawn": DawnCustom.REL_MIN_72,
		"misheyakir": MisheyakirCustom.MIN_50,
		"nightfall": NightfallCustom.REL_MIN_90,
		"dayDef": DayDefinition.MA,
	},
	Custom.BEN_ISH_CHAI: {
		"dawn": DawnCustom.DEG_19_8,
		"misheyakir": MisheyakirCustom.DEG_11,
		"nightfall": NightfallCustom.DEG_7_1,
		"dayDef": DayDefinition.MA,
	},
	Custom.WESTERN_EUROPEAN: {
		"dawn": DawnCustom.DEG_18,
		"misheyakir": MisheyakirCustom.DEG_11_5,
		"nightfall": NightfallCustom.DEG_8_5,
		"dayDef": DayDefinition.GRA,
	},
}
