/* Geo.cpp
 * Ported from LIBS/Geo.py
 */
#include "Geo.h"

#include <math.h>
#include <string.h>

#define GEO_PI 3.14159265358979323846

Location::Location(double latitude, double longitude)
{
	this->latitude = latitude;
	this->longitude = longitude;
}

double Location::distance(const Location& other) const
{
	double dLat = (other.latitude - latitude) * GEO_PI / 180.0;
	double dLon = (other.longitude - longitude) * GEO_PI / 180.0;
	double sinDLat = sin(dLat / 2.0);
	double sinDLon = sin(dLon / 2.0);
	double a = sinDLat * sinDLat +
		cos(latitude * GEO_PI / 180.0) * cos(other.latitude * GEO_PI / 180.0) *
		sinDLon * sinDLon;
	double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
	return GEO_EARTH_RADIUS * c;
}

double Location::direction(const Location& other) const
{
	double lat1 = latitude * GEO_PI / 180.0;
	double lat2 = other.latitude * GEO_PI / 180.0;
	double dLon = (other.longitude - longitude) * GEO_PI / 180.0;
	double y = sin(dLon) * cos(lat2);
	double x = cos(lat1) * sin(lat2) -
		sin(lat1) * cos(lat2) * cos(dLon);
	double bearing = atan2(y, x) * 180.0 / GEO_PI;
	return fmod(bearing + 360.0, 360.0);
}

bool Location::operator==(const Location& other) const
{
	return latitude == other.latitude && longitude == other.longitude;
}

bool Location::operator!=(const Location& other) const
{
	return !(*this == other);
}

const Location GEO_DIASPORA_NORTH(33.317, 35.767);
const Location GEO_DIASPORA_SOUTH(29.483, 34.9);
const Location GEO_DIASPORA_EAST(32.933, 35.883);
const Location GEO_DIASPORA_WEST(31.22, 34.268);

Place::Place(const char* id, double latitude, double longitude, double area)
{
	this->id = id;
	this->latitude = latitude;
	this->longitude = longitude;
	this->area = area;
}

Location Place::location() const
{
	return Location(latitude, longitude);
}

bool Place::isDiaspora() const
{
	double centerLat = (GEO_DIASPORA_NORTH.latitude + GEO_DIASPORA_SOUTH.latitude) / 2.0;
	double centerLon = (GEO_DIASPORA_EAST.longitude + GEO_DIASPORA_WEST.longitude) / 2.0;
	double semiMajor = (GEO_DIASPORA_NORTH.latitude - GEO_DIASPORA_SOUTH.latitude) / 2.0;
	double semiMinor = (GEO_DIASPORA_EAST.longitude - GEO_DIASPORA_WEST.longitude) / 2.0;
	double x = latitude - centerLat;
	double y = longitude - centerLon;
	return ((x * x) / (semiMajor * semiMajor) + (y * y) / (semiMinor * semiMinor)) > 1.0;
}

bool Place::operator==(const Place& other) const
{
	return strcmp(id, other.id) == 0;
}

bool Place::operator!=(const Place& other) const
{
	return !(*this == other);
}

const Place& Place::fromCoordinates(double latitude, double longitude)
{
	for (int i = 0; i < Place::ALL_COUNT; i++) {
		const Place* p = Place::ALL[i];
		if (fabs(p->latitude - latitude) < 1e-6 && fabs(p->longitude - longitude) < 1e-6) {
			return *p;
		}
	}
	return Place::NONE;
}

const Place& Place::closest(const Location& loc)
{
	const Place* closestPlace = Place::ALL[0];
	double minDist = loc.distance(closestPlace->location());
	for (int i = 1; i < Place::ALL_COUNT; i++) {
		const Place* p = Place::ALL[i];
		double d = loc.distance(p->location());
		if (d < minDist) {
			minDist = d;
			closestPlace = p;
		}
	}
	return *closestPlace;
}

/* ---------- Static Place instances (matches Geo.py's Place.<NAME> assignments) ---------- */

const Place Place::NONE("NONE", 0.0, 0.0, 0.0);
const Place Place::OFAKIM("OFAKIM", 31.3167, 34.6167, 16.35);
const Place Place::OR_HAGANUZ("OR_HAGANUZ", 33.0, 35.44, 0.32);
const Place Place::EILAT("EILAT", 29.5577, 34.9519, 101.49);
const Place Place::ELAD("ELAD", 32.0523, 34.951, 3.49);
const Place Place::AMIRIM("AMIRIM", 32.937, 35.45, 0.76);
const Place Place::ARIEL("ARIEL", 32.1037, 35.1774, 14.48);
const Place Place::ASHDOD("ASHDOD", 31.8044, 34.6553, 63.92);
const Place Place::ASHKELON("ASHKELON", 31.6688, 34.5743, 52.32);
const Place Place::BEER_YAAKOV("BEER_YAAKOV", 31.94, 34.84, 9.46);
const Place Place::BEER_SHEVA("BEER_SHEVA", 31.253, 34.7915, 117.39);
const Place Place::BEIT_SHEAN("BEIT_SHEAN", 32.497, 35.498, 10.97);
const Place Place::BEIT_SHEMESH("BEIT_SHEMESH", 31.7473, 34.9876, 38.29);
const Place Place::BNEI_BRAK("BNEI_BRAK", 32.0834, 34.8331, 7.35);
const Place Place::BAR_YOCHAI("BAR_YOCHAI", 32.998, 35.446, 0.78);
const Place Place::BAT_YAM("BAT_YAM", 32.017, 34.75, 9.41);
const Place Place::GIVAT_SHMUEL("GIVAT_SHMUEL", 32.077, 34.849, 2.58);
const Place Place::JISH("JISH", 33.024, 35.442, 6.92);
const Place Place::GAN_YAVNE("GAN_YAVNE", 31.787, 34.706, 4.47);
const Place Place::GANEI_TIKVA("GANEI_TIKVA", 32.08, 34.92, 2.16);
const Place Place::GIVATAYIM("GIVATAYIM", 32.072, 34.812, 3.24);
const Place Place::DIMONA("DIMONA", 31.0694, 35.033, 220.46);
const Place Place::DALTON("DALTON", 33.015, 35.485, 1.31);
const Place Place::HOD_HASHARON("HOD_HASHARON", 32.15, 34.89, 19.26);
const Place Place::HERZLIYA("HERZLIYA", 32.1627, 34.8446, 24.07);
const Place Place::HADERA("HADERA", 32.436, 34.916, 56.28);
const Place Place::HOLON("HOLON", 32.0114, 34.7794, 19.04);
const Place Place::HAIFA("HAIFA", 32.794, 34.9896, 72.93);
const Place Place::HARISH("HARISH", 32.462, 35.048, 9.4);
const Place Place::TIBERIAS("TIBERIAS", 32.7922, 35.5312, 16.19);
const Place Place::TIRAT_CARMEL("TIRAT_CARMEL", 32.77, 34.97, 6.71);
const Place Place::YAVNE("YAVNE", 31.877, 34.744, 29.18);
const Place Place::YOKNEAM_ILLIT("YOKNEAM_ILLIT", 32.66, 35.11, 8.31);
const Place Place::YEHUD_MONOSSON("YEHUD_MONOSSON", 32.03, 34.89, 5.73);
const Place Place::JERUSALEM("JERUSALEM", 31.7683, 35.2137, 125.55);
const Place Place::KARMIEL("KARMIEL", 32.919, 35.302, 22.03);
const Place Place::KFAR_SABA("KFAR_SABA", 32.175, 34.906, 14.48);
const Place Place::KFAR_YONA("KFAR_YONA", 32.32, 34.93, 11.51);
const Place Place::KIRYAT_ATA("KIRYAT_ATA", 32.81, 35.11, 24.16);
const Place Place::KIRYAT_BIALIK("KIRYAT_BIALIK", 32.83, 35.08, 8.48);
const Place Place::KIRYAT_GAT("KIRYAT_GAT", 31.61, 34.771, 15.85);
const Place Place::KIRYAT_MALAKHI("KIRYAT_MALAKHI", 31.73, 34.75, 4.58);
const Place Place::KIRYAT_MOTZKIN("KIRYAT_MOTZKIN", 32.83, 35.08, 3.84);
const Place Place::KIRYAT_ONO("KIRYAT_ONO", 32.06, 34.85, 4.6);
const Place Place::KIRYAT_SHMONA("KIRYAT_SHMONA", 33.207, 35.571, 14.38);
const Place Place::KIRYAT_YAM("KIRYAT_YAM", 32.85, 35.07, 11.34);
const Place Place::KATZRIN("KATZRIN", 32.992, 35.69, 4.0);
const Place Place::LOD("LOD", 31.953, 34.893, 14.79);
const Place Place::MAALOT_TARSHIHA("MAALOT_TARSHIHA", 33.02, 35.29, 9.26);
const Place Place::MODIIN("MODIIN", 31.8815, 35.0106, 48.33);
const Place Place::MODIIN_ILLIT("MODIIN_ILLIT", 31.929, 35.043, 6.0);
const Place Place::MIGDAL_HAEMEK("MIGDAL_HAEMEK", 32.67, 35.24, 8.71);
const Place Place::MAALE_ADUMIM("MAALE_ADUMIM", 31.777, 35.298, 8.0);
const Place Place::MERON("MERON", 32.9837, 35.4508, 1.17);
const Place Place::NAHARIYA("NAHARIYA", 33.0119, 35.098, 13.83);
const Place Place::NESS_ZIONA("NESS_ZIONA", 31.93, 34.8, 15.68);
const Place Place::NETANYA("NETANYA", 32.3323, 34.8571, 34.75);
const Place Place::NETIVOT("NETIVOT", 31.421, 34.586, 16.22);
const Place Place::NESHER("NESHER", 32.76, 35.04, 12.94);
const Place Place::NOF_HAGALIL("NOF_HAGALIL", 32.71, 35.33, 32.86);
const Place Place::SIFSUFA("SIFSUFA", 33.012, 35.43, 1.09);
const Place Place::AKKO("AKKO", 32.927, 35.083, 18.09);
const Place Place::AFULA("AFULA", 32.6083, 35.288, 29.17);
const Place Place::ARAD("ARAD", 31.2583, 35.214, 126.13);
const Place Place::OR_AKIVA("OR_AKIVA", 32.51, 34.92, 5.55);
const Place Place::OR_YEHUDA("OR_YEHUDA", 32.03, 34.86, 6.73);
const Place Place::FAROD("FAROD", 32.932, 35.43, 1.2);
const Place Place::PETAH_TIKVA("PETAH_TIKVA", 32.0849, 34.8877, 35.77);
const Place Place::SAFED("SAFED", 32.9646, 35.4969, 29.94);
const Place Place::KADITA("KADITA", 33.002, 35.47, 0.9);
const Place Place::RAHAT("RAHAT", 31.39, 34.75, 33.48);
const Place Place::ROSH_HAAYIN("ROSH_HAAYIN", 32.095, 34.952, 15.86);
const Place Place::RISHON_LEZION("RISHON_LEZION", 31.973, 34.7925, 61.91);
const Place Place::REHOVOT("REHOVOT", 31.8948, 34.8096, 23.76);
const Place Place::RAMLA("RAMLA", 31.929, 34.866, 13.39);
const Place Place::RAMAT_GAN("RAMAT_GAN", 32.0834, 34.8106, 16.39);
const Place Place::RAMAT_HASHARON("RAMAT_HASHARON", 32.15, 34.84, 16.73);
const Place Place::RAANANA("RAANANA", 32.1833, 34.87, 14.86);
const Place Place::SDEROT("SDEROT", 31.525, 34.593, 10.66);
const Place Place::SHEFER("SHEFER", 32.937, 35.44, 0.35);
const Place Place::TEL_AVIV("TEL_AVIV", 32.0853, 34.7818, 57.14);
const Place Place::UMAN("UMAN", 48.7519, 30.2193, 10.0);
const Place Place::AMSTERDAM("AMSTERDAM", 52.3676, 4.9041, 219.0);
const Place Place::BANGKOK("BANGKOK", 13.7563, 100.5018, 1569.0);
const Place Place::BUDAPEST("BUDAPEST", 47.4979, 19.0402, 525.0);
const Place Place::DUBAI("DUBAI", 25.2048, 55.2708, 4114.0);
const Place Place::LONDON("LONDON", 51.5074, -0.1278, 1572.0);
const Place Place::NEW_YORK("NEW_YORK", 40.7128, -74.006, 1214.0);
const Place Place::PARIS("PARIS", 48.8566, 2.3522, 105.4);
const Place Place::KYIV("KYIV", 50.4501, 30.5234, 839.0);
const Place Place::ROME("ROME", 41.9028, 12.4964, 1285.0);


/* ---------- Place::ALL (matches Geo.py's Place.all(), NONE excluded) ---------- */

const Place* const Place::ALL[Place::ALL_COUNT] = {
	&Place::OFAKIM,
	&Place::OR_HAGANUZ,
	&Place::EILAT,
	&Place::ELAD,
	&Place::AMIRIM,
	&Place::ARIEL,
	&Place::ASHDOD,
	&Place::ASHKELON,
	&Place::BEER_YAAKOV,
	&Place::BEER_SHEVA,
	&Place::BEIT_SHEAN,
	&Place::BEIT_SHEMESH,
	&Place::BNEI_BRAK,
	&Place::BAR_YOCHAI,
	&Place::BAT_YAM,
	&Place::GIVAT_SHMUEL,
	&Place::JISH,
	&Place::GAN_YAVNE,
	&Place::GANEI_TIKVA,
	&Place::GIVATAYIM,
	&Place::DIMONA,
	&Place::DALTON,
	&Place::HOD_HASHARON,
	&Place::HERZLIYA,
	&Place::HADERA,
	&Place::HOLON,
	&Place::HAIFA,
	&Place::HARISH,
	&Place::TIBERIAS,
	&Place::TIRAT_CARMEL,
	&Place::YAVNE,
	&Place::YOKNEAM_ILLIT,
	&Place::YEHUD_MONOSSON,
	&Place::JERUSALEM,
	&Place::KARMIEL,
	&Place::KFAR_SABA,
	&Place::KFAR_YONA,
	&Place::KIRYAT_ATA,
	&Place::KIRYAT_BIALIK,
	&Place::KIRYAT_GAT,
	&Place::KIRYAT_MALAKHI,
	&Place::KIRYAT_MOTZKIN,
	&Place::KIRYAT_ONO,
	&Place::KIRYAT_SHMONA,
	&Place::KIRYAT_YAM,
	&Place::KATZRIN,
	&Place::LOD,
	&Place::MAALOT_TARSHIHA,
	&Place::MODIIN,
	&Place::MODIIN_ILLIT,
	&Place::MIGDAL_HAEMEK,
	&Place::MAALE_ADUMIM,
	&Place::MERON,
	&Place::NAHARIYA,
	&Place::NESS_ZIONA,
	&Place::NETANYA,
	&Place::NETIVOT,
	&Place::NESHER,
	&Place::NOF_HAGALIL,
	&Place::SIFSUFA,
	&Place::AKKO,
	&Place::AFULA,
	&Place::ARAD,
	&Place::OR_AKIVA,
	&Place::OR_YEHUDA,
	&Place::FAROD,
	&Place::PETAH_TIKVA,
	&Place::SAFED,
	&Place::KADITA,
	&Place::RAHAT,
	&Place::ROSH_HAAYIN,
	&Place::RISHON_LEZION,
	&Place::REHOVOT,
	&Place::RAMLA,
	&Place::RAMAT_GAN,
	&Place::RAMAT_HASHARON,
	&Place::RAANANA,
	&Place::SDEROT,
	&Place::SHEFER,
	&Place::TEL_AVIV,
	&Place::UMAN,
	&Place::AMSTERDAM,
	&Place::BANGKOK,
	&Place::BUDAPEST,
	&Place::DUBAI,
	&Place::LONDON,
	&Place::NEW_YORK,
	&Place::PARIS,
	&Place::KYIV,
	&Place::ROME,
};

