/* Geo.h
 * Ported from LIBS/Geo.py
 */
#ifndef GEO_H
#define GEO_H

#define GEO_EARTH_RADIUS 6371000.0

class Location {
public:
	double latitude;
	double longitude;

	Location(double latitude = 0.0, double longitude = 0.0);

	double distance(const Location& other) const;
	double direction(const Location& other) const;

	bool operator==(const Location& other) const;
	bool operator!=(const Location& other) const;
};

/* Module-level Location constants from Geo.py, used by Place::isDiaspora(). */
extern const Location GEO_DIASPORA_NORTH;
extern const Location GEO_DIASPORA_SOUTH;
extern const Location GEO_DIASPORA_EAST;
extern const Location GEO_DIASPORA_WEST;

class Place {
public:
	static const int ALL_COUNT = 90;

	const char* id;
	double latitude;
	double longitude;
	double area;

	Place(const char* id, double latitude, double longitude, double area);

	Location location() const;
	bool isDiaspora() const;

	bool operator==(const Place& other) const;
	bool operator!=(const Place& other) const;

	static const Place& fromCoordinates(double latitude, double longitude);
	static const Place& closest(const Location& loc);

	/* Ordered list matching Geo.py's Place.all(), NONE excluded (as in Python). */
	static const Place* const ALL[ALL_COUNT];

	static const Place NONE;
	static const Place OFAKIM;
	static const Place OR_HAGANUZ;
	static const Place EILAT;
	static const Place ELAD;
	static const Place AMIRIM;
	static const Place ARIEL;
	static const Place ASHDOD;
	static const Place ASHKELON;
	static const Place BEER_YAAKOV;
	static const Place BEER_SHEVA;
	static const Place BEIT_SHEAN;
	static const Place BEIT_SHEMESH;
	static const Place BNEI_BRAK;
	static const Place BAR_YOCHAI;
	static const Place BAT_YAM;
	static const Place GIVAT_SHMUEL;
	static const Place JISH;
	static const Place GAN_YAVNE;
	static const Place GANEI_TIKVA;
	static const Place GIVATAYIM;
	static const Place DIMONA;
	static const Place DALTON;
	static const Place HOD_HASHARON;
	static const Place HERZLIYA;
	static const Place HADERA;
	static const Place HOLON;
	static const Place HAIFA;
	static const Place HARISH;
	static const Place TIBERIAS;
	static const Place TIRAT_CARMEL;
	static const Place YAVNE;
	static const Place YOKNEAM_ILLIT;
	static const Place YEHUD_MONOSSON;
	static const Place JERUSALEM;
	static const Place KARMIEL;
	static const Place KFAR_SABA;
	static const Place KFAR_YONA;
	static const Place KIRYAT_ATA;
	static const Place KIRYAT_BIALIK;
	static const Place KIRYAT_GAT;
	static const Place KIRYAT_MALAKHI;
	static const Place KIRYAT_MOTZKIN;
	static const Place KIRYAT_ONO;
	static const Place KIRYAT_SHMONA;
	static const Place KIRYAT_YAM;
	static const Place KATZRIN;
	static const Place LOD;
	static const Place MAALOT_TARSHIHA;
	static const Place MODIIN;
	static const Place MODIIN_ILLIT;
	static const Place MIGDAL_HAEMEK;
	static const Place MAALE_ADUMIM;
	static const Place MERON;
	static const Place NAHARIYA;
	static const Place NESS_ZIONA;
	static const Place NETANYA;
	static const Place NETIVOT;
	static const Place NESHER;
	static const Place NOF_HAGALIL;
	static const Place SIFSUFA;
	static const Place AKKO;
	static const Place AFULA;
	static const Place ARAD;
	static const Place OR_AKIVA;
	static const Place OR_YEHUDA;
	static const Place FAROD;
	static const Place PETAH_TIKVA;
	static const Place SAFED;
	static const Place KADITA;
	static const Place RAHAT;
	static const Place ROSH_HAAYIN;
	static const Place RISHON_LEZION;
	static const Place REHOVOT;
	static const Place RAMLA;
	static const Place RAMAT_GAN;
	static const Place RAMAT_HASHARON;
	static const Place RAANANA;
	static const Place SDEROT;
	static const Place SHEFER;
	static const Place TEL_AVIV;
	static const Place UMAN;
	static const Place AMSTERDAM;
	static const Place BANGKOK;
	static const Place BUDAPEST;
	static const Place DUBAI;
	static const Place LONDON;
	static const Place NEW_YORK;
	static const Place PARIS;
	static const Place KYIV;
	static const Place ROME;

};

#endif
