/**
  ******************************************************************************
  * @file           : geofence.c
  * @brief          : Algorithms for geofencing
  ******************************************************************************
  * Imperial College Space Society
	* Medad Newman, Richard Ibbotson
	* Modified from TT7's code 
	* https://github.com/TomasTT7/TT7F-Float-Tracker
  *
  *
  ******************************************************************************
  */

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */

#include "geofence.h"
#include "LoRaMac.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */

/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */

/* Global variables definitions go here */

// GEOFENCE variables
/* The world is split into polygons e.g. EU863870_EUROPE_polygon. 
 * Multiple polygons can have the same LoRa region settings. E.g. LORAMAC_REGION_EU868.
 * Keeps track of which polygon the tracker is in, and if it changes to another polygon,
 * all LoRa settings are reinitialised when the balloon enters another polygon.
 * 
 */

/*!
 * Polygon region enumeration
 */
typedef enum polygon_t
{
	EU863870_PHILIPPINES_polygon,
	US902928_NAMERICA_polygon,
	AS923925_BRUNEI_polygon,
	AS923925_TAIWAN_polygon,
	AS923925_INDONESIA_polygon,
	AS923925_THAILAND_polygon,
	US902928_ARGENTINA_polygon,
	AU915928_BRAZIL_polygon,
	AU915928_CHILE_polygon,
	CN779787_CHINA_polygon,
	IN865867_INDIA_polygon,
	AS920923_JAPAN_polygon,
	KR920923_SKOREA_polygon,
	AS920923_MALAYSIASG_polygon,
	AU915928_AUSTRALIA_polygon,
	RU864870_RUSSIA_polygon,
	EU863870_EUROPE_polygon,
	US902928_GUAM_polygon,
	US902928_ANTARTICA_polygon,
	OUTSIDE_polygon
} Polygon_t;

geofence_status_t current_geofence_status =
	{
		.current_loramac_region = LORAMAC_REGION_EU868,
		.tx_permission = TX_OK,
};

Polygon_t curr_poly_region;

/* These are the fence polygons. There are several 
 * polygons that have the same frequency names but have a 
 * number after: EU863870_1F e.g to make them a distinct polygon 
 * Code is auto generated with python script.
 */

// GEOFENCE ARRAYS (longitude, latitude)
static const float EU863870_PHILIPPINES_F[14] = {
	119.35326,
	19.47621,
	115.48607,
	9.96808,
	118.25463,
	7.10011,
	123.39623,
	4.03883,
	132.14134,
	5.00261,
	127.35130,
	19.55905,
	119.35326,
	19.47621,

};

static const float US902928_NAMERICA_F[40] = {
	-168.70273,
	73.32477,
	-180.00000,
	46.37053,
	-180.00000,
	25.64941,
	-180.00000,
	24.28805,
	-180.00000,
	4.08145,
	-180.00000,
	1.28292,
	-180.00000,
	-20.25916,
	-73.01078,
	-60.51533,
	-76.49889,
	-44.50321,
	-70.88094,
	-17.08505,
	-67.49103,
	-22.91785,
	-57.68823,
	-19.31401,
	-68.90763,
	4.40695,
	-51.18327,
	5.64711,
	-43.90451,
	53.55910,
	-71.84486,
	72.12693,
	-73.66926,
	78.90700,
	-51.52817,
	84.26362,
	-109.26311,
	84.50567,
	-168.70273,
	73.32477,

};

static const float AS923925_BRUNEI_F[12] = {
	114.06400,
	4.89831,
	114.52543,
	3.95084,
	115.46751,
	4.26588,
	115.18186,
	5.06796,
	114.73417,
	5.25396,
	114.06400,
	4.89831,

};

static const float AS923925_TAIWAN_F[14] = {
	112.43139,
	21.52394,
	115.44164,
	19.27934,
	122.73656,
	20.74515,
	123.20570,
	26.19993,
	121.80764,
	26.38139,
	116.62817,
	22.88688,
	112.43139,
	21.52394,

};

static const float AS923925_INDONESIA_F[30] = {
	92.01692,
	5.86666,
	103.00325,
	-10.41292,
	120.58137,
	-11.62069,
	128.31575,
	-9.54732,
	139.56575,
	-9.72062,
	140.15829,
	0.93777,
	130.95303,
	4.18266,
	122.51497,
	3.76485,
	117.76887,
	3.94023,
	112.49518,
	1.39063,
	108.40480,
	-0.48386,
	104.45524,
	-0.25706,
	98.09623,
	4.87674,
	94.88327,
	7.30090,
	92.01692,
	5.86666,

};

static const float AS923925_THAILAND_F[24] = {
	95.86952,
	12.49757,
	97.08270,
	7.02099,
	100.15887,
	6.43181,
	106.24529,
	5.29523,
	112.57342,
	9.93283,
	110.90612,
	16.62593,
	107.67732,
	20.91948,
	105.21258,
	23.02586,
	101.60622,
	22.81417,
	97.24373,
	19.18170,
	95.75481,
	15.85441,
	95.86952,
	12.49757,

};

static const float US902928_ARGENTINA_F[20] = {
	-67.49103,
	-22.91785,
	-70.28316,
	-31.75017,
	-71.60152,
	-45.96913,
	-72.21675,
	-51.90602,
	-66.76753,
	-55.44369,
	-52.95398,
	-39.95226,
	-47.43353,
	-33.89651,
	-57.19000,
	-30.69796,
	-57.68823,
	-19.31401,
	-67.49103,
	-22.91785,

};

static const float AU915928_BRAZIL_F[18] = {
	-68.90763,
	4.40695,
	-57.68823,
	-19.31401,
	-57.19000,
	-30.69796,
	-52.26812,
	-32.34613,
	-47.48924,
	-33.37801,
	-36.30188,
	-20.52197,
	-29.62669,
	-2.07565,
	-51.18327,
	5.64711,
	-68.90763,
	4.40695,

};

static const float AU915928_CHILE_F[16] = {
	-70.88094,
	-17.08505,
	-76.49889,
	-44.50321,
	-73.01078,
	-60.51533,
	-66.76753,
	-55.44369,
	-72.21675,
	-51.90602,
	-70.28316,
	-31.75017,
	-67.49103,
	-22.91785,
	-70.88094,
	-17.08505,

};

static const float CN779787_CHINA_F[40] = {
	82.29205,
	48.54875,
	67.87798,
	37.82644,
	82.86334,
	29.67296,
	87.89547,
	26.80085,
	96.70611,
	29.04099,
	108.65923,
	20.43133,
	110.80828,
	20.99835,
	116.86311,
	23.15489,
	121.31548,
	26.23843,
	121.84283,
	36.31883,
	123.86964,
	39.73139,
	129.92800,
	43.13565,
	136.08111,
	45.18528,
	133.33570,
	48.85359,
	120.61236,
	54.65108,
	114.98736,
	49.69904,
	99.81818,
	51.35649,
	88.22016,
	48.70281,
	84.92757,
	49.23314,
	82.29205,
	48.54875,

};

static const float IN865867_INDIA_F[16] = {
	74.81150,
	34.52082,
	67.28233,
	20.93704,
	75.16306,
	4.16647,
	82.85421,
	2.39383,
	85.70994,
	6.96542,
	90.36814,
	21.24408,
	87.89547,
	26.80085,
	74.81150,
	34.52082,

};

static const float AS920923_JAPAN_F[22] = {
	141.98268,
	50.05733,
	138.86120,
	46.78276,
	135.11622,
	43.28542,
	132.01787,
	41.73885,
	131.63668,
	38.20620,
	129.48488,
	35.15270,
	126.28398,
	32.57064,
	136.03907,
	22.60415,
	150.10157,
	33.80679,
	156.36378,
	43.60448,
	141.98268,
	50.05733,

};

static const float KR920923_SKOREA_F[18] = {
	122.94545,
	36.60046,
	122.59912,
	32.47361,
	126.13382,
	32.60534,
	129.46840,
	35.18414,
	131.42570,
	38.21762,
	131.67979,
	41.62004,
	127.96554,
	39.81694,
	128.33108,
	38.53374,
	122.94545,
	36.60046,

};

static const float AS920923_MALAYSIASG_F[30] = {
	97.26683,
	6.67666,
	99.27305,
	3.85647,
	102.22180,
	1.30487,
	104.34551,
	-0.06553,
	108.43935,
	-0.34457,
	110.23070,
	0.67130,
	114.97679,
	2.25276,
	119.32738,
	4.70897,
	116.95433,
	7.63591,
	115.18186,
	5.06796,
	115.46751,
	4.26588,
	114.52543,
	3.95084,
	114.06400,
	4.89831,
	102.01292,
	5.97782,
	97.26683,
	6.67666,

};

static const float AU915928_AUSTRALIA_F[26] = {
	119.84993,
	-43.64813,
	131.06762,
	-51.28254,
	145.93096,
	-55.22234,
	169.37107,
	-54.07644,
	179.46983,
	-48.46907,
	185.02757,
	-33.42431,
	171.92924,
	-16.91222,
	148.12775,
	-14.43550,
	141.14083,
	-8.43254,
	128.98378,
	-9.52913,
	109.15307,
	-14.41378,
	99.08073,
	-36.79314,
	119.84993,
	-43.64813,

};

static const float RU864870_RUSSIA_F[50] = {
	114.00630,
	77.20060,
	27.20759,
	70.95692,
	31.56924,
	63.02777,
	27.95082,
	59.36052,
	28.45041,
	56.30347,
	32.09602,
	53.03805,
	38.44658,
	50.46738,
	40.82452,
	48.92639,
	35.67013,
	45.21170,
	39.49369,
	43.16172,
	48.66651,
	41.91436,
	49.50708,
	46.73458,
	45.91863,
	48.39790,
	53.70648,
	49.72825,
	62.18802,
	50.25236,
	68.29710,
	51.37026,
	72.82657,
	52.77560,
	76.84853,
	51.31352,
	88.21380,
	48.68017,
	99.53778,
	51.41911,
	114.90963,
	49.99259,
	120.64929,
	54.65274,
	154.20179,
	44.98566,
	173.75796,
	69.91541,
	114.00630,
	77.20060,

};

static const float EU863870_EUROPE_F[80] = {
	-71.71870,
	78.35168,
	-59.04415,
	68.96731,
	-45.13904,
	53.93611,
	-43.36858,
	48.94212,
	-51.18327,
	5.64711,
	-29.67099,
	-1.46785,
	-36.94581,
	-21.44597,
	-66.93514,
	-53.89986,
	-17.65920,
	-63.75386,
	33.89876,
	-60.35518,
	88.84521,
	-57.64581,
	101.60152,
	-40.28590,
	108.30890,
	-14.01536,
	93.03493,
	-3.08195,
	78.27622,
	-0.69161,
	67.41226,
	21.01872,
	74.81150,
	34.52082,
	67.73073,
	38.13921,
	81.77174,
	48.33576,
	84.33079,
	49.20482,
	77.06897,
	51.19661,
	72.90484,
	52.70164,
	68.15619,
	51.22012,
	62.30322,
	50.09428,
	53.80241,
	49.57254,
	46.02093,
	48.56997,
	49.61500,
	46.92261,
	48.56072,
	41.66768,
	39.85040,
	43.02380,
	35.60168,
	45.18883,
	41.00384,
	48.89668,
	38.18940,
	50.86832,
	31.85052,
	53.07085,
	28.40434,
	56.34850,
	27.98583,
	59.48825,
	31.66246,
	63.29655,
	27.10220,
	71.16545,
	73.60816,
	78.68579,
	24.27672,
	85.98405,
	-71.71870,
	78.35168,

};

static const float US902928_GUAM_F[30] = {
	177.53889,
	-21.93288,
	179.24178,
	-20.49939,
	179.24502,
	46.05195,
	177.49681,
	59.81079,
	176.25674,
	62.21270,
	164.30361,
	55.75932,
	152.08682,
	35.86420,
	136.53018,
	21.34268,
	131.60830,
	27.81195,
	126.33486,
	26.95349,
	127.06306,
	20.52353,
	132.39932,
	4.18435,
	147.51650,
	-1.52463,
	158.32648,
	-4.22688,
	177.53889,
	-21.93288,

};

static const float US902928_ANTARTICA_F[30] = {
	-174.56046,
	-84.76164,
	-37.09953,
	-83.26629,
	84.54110,
	-84.63167,
	153.79891,
	-84.49848,
	175.94735,
	-84.39643,
	178.91691,
	-62.95768,
	148.01429,
	-55.17260,
	112.73509,
	-54.69288,
	66.83555,
	-60.44192,
	6.53924,
	-61.92870,
	-57.13859,
	-62.54845,
	-128.50578,
	-63.34784,
	-155.57609,
	-63.34784,
	-177.37296,
	-63.18970,
	-174.56046,
	-84.76164,

};

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* Definition of private datatypes go here */

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */

/* Function prototypes for private (static) functions go here */

static int32_t pointInPolygonF(int32_t polyCorners, const float *polygon, float latitude, float longitude);
Polygon_t get_polygon(float latitude, float longitude);
static void set_current_loramac_region(Polygon_t current_poly);

/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */

/* Functions definitions go here, organised into sections */

// Strategy: check if GPS coordinate is in any of the polygons. If so, look up what transmit frequency it uses.

LoRaMacRegion_t get_current_loramac_region()
{
	return current_geofence_status.current_loramac_region;
}

/*
	Adapted version of pointInPolygon() function from:	http://alienryderflex.com/polygon/
	
	Returns '0' if the point is outside of the polygon and '1' if it's inside.
	
	Uses FLOAT input for better accuracy.
*/
int32_t pointInPolygonF(int32_t polyCorners, const float *polygon, float latitude, float longitude)
{
	int32_t i;
	int32_t j = polyCorners * 2 - 2;
	int32_t oddNodes = 0;

	for (i = 0; i < polyCorners * 2; i += 2)
	{
		if (((polygon[i + 1] < latitude && polygon[j + 1] >= latitude) || ((polygon[j + 1] < latitude && polygon[i + 1] >= latitude) && (polygon[i] <= longitude || polygon[j] <= longitude))))
		{
			oddNodes ^= (polygon[i] + (latitude - polygon[i + 1]) / (polygon[j + 1] - polygon[i + 1]) * (polygon[j] - polygon[i]) < longitude);
		}

		j = i;
	}

	return oddNodes;
}

/*
	Changes GEOFENCE_LoRA_frequency and tx_permission global variables based on the input coordinates.
	FREQUENCIES:
						EU863870    = 0x01,
						AU915928    = 0x02,
						US902928    = 0x03,
						KR920923    = 0x04,
						IN865867    = 0x05,
						AS920923		= 0x06,
						CN779787		= 0x07,
						AS923925		= 0x08,
            RU864870    = 0x09
	Expected input FLOAT for latitude and longitude as in GPS_UBX_latitude_Float and GPS_UBX_longitude_Float.
*/

Polygon_t get_polygon(float latitude, float longitude)
{
	/* check if point is in polygon region */
	/* Autogenerated code with python script. hopefully no bugs! */
	if (pointInPolygonF(7, EU863870_PHILIPPINES_F, latitude, longitude) == 1)
	{
		return EU863870_PHILIPPINES_polygon;
	}

	else if (pointInPolygonF(20, US902928_NAMERICA_F, latitude, longitude) == 1)
	{
		return US902928_NAMERICA_polygon;
	}

	else if (pointInPolygonF(6, AS923925_BRUNEI_F, latitude, longitude) == 1)
	{
		return AS923925_BRUNEI_polygon;
	}

	else if (pointInPolygonF(7, AS923925_TAIWAN_F, latitude, longitude) == 1)
	{
		return AS923925_TAIWAN_polygon;
	}

	else if (pointInPolygonF(15, AS923925_INDONESIA_F, latitude, longitude) == 1)
	{
		return AS923925_INDONESIA_polygon;
	}

	else if (pointInPolygonF(12, AS923925_THAILAND_F, latitude, longitude) == 1)
	{
		return AS923925_THAILAND_polygon;
	}

	else if (pointInPolygonF(10, US902928_ARGENTINA_F, latitude, longitude) == 1)
	{
		return US902928_ARGENTINA_polygon;
	}

	else if (pointInPolygonF(9, AU915928_BRAZIL_F, latitude, longitude) == 1)
	{
		return AU915928_BRAZIL_polygon;
	}

	else if (pointInPolygonF(8, AU915928_CHILE_F, latitude, longitude) == 1)
	{
		return AU915928_CHILE_polygon;
	}

	else if (pointInPolygonF(20, CN779787_CHINA_F, latitude, longitude) == 1)
	{
		return CN779787_CHINA_polygon;
	}

	else if (pointInPolygonF(8, IN865867_INDIA_F, latitude, longitude) == 1)
	{
		return IN865867_INDIA_polygon;
	}

	else if (pointInPolygonF(11, AS920923_JAPAN_F, latitude, longitude) == 1)
	{
		return AS920923_JAPAN_polygon;
	}

	else if (pointInPolygonF(9, KR920923_SKOREA_F, latitude, longitude) == 1)
	{
		return KR920923_SKOREA_polygon;
	}

	else if (pointInPolygonF(15, AS920923_MALAYSIASG_F, latitude, longitude) == 1)
	{
		return AS920923_MALAYSIASG_polygon;
	}

	else if (pointInPolygonF(13, AU915928_AUSTRALIA_F, latitude, longitude) == 1)
	{
		return AU915928_AUSTRALIA_polygon;
	}

	else if (pointInPolygonF(25, RU864870_RUSSIA_F, latitude, longitude) == 1)
	{
		return RU864870_RUSSIA_polygon;
	}

	else if (pointInPolygonF(40, EU863870_EUROPE_F, latitude, longitude) == 1)
	{
		return EU863870_EUROPE_polygon;
	}

	else if (pointInPolygonF(15, US902928_GUAM_F, latitude, longitude) == 1)
	{
		return US902928_GUAM_polygon;
	}

	else if (pointInPolygonF(15, US902928_ANTARTICA_F, latitude, longitude) == 1)
	{
		return US902928_ANTARTICA_polygon;
	}

	return OUTSIDE_polygon;
}

static void set_current_loramac_region(Polygon_t current_poly)
{
	// Autogenerated code with python script. hopefully no bugs!

	switch (current_poly)
	{
	case EU863870_PHILIPPINES_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_EU868;
		break;
	case US902928_NAMERICA_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_US915;
		break;
	case AS923925_BRUNEI_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_AS923;
		break;
	case AS923925_TAIWAN_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_AS923;
		break;
	case AS923925_INDONESIA_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_AS923;
		break;
	case AS923925_THAILAND_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_AS923;
		break;
	case US902928_ARGENTINA_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_US915;
		break;
	case AU915928_BRAZIL_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_AU915;
		break;
	case AU915928_CHILE_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_AU915;
		break;
	case CN779787_CHINA_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_CN470;
		break;
	case IN865867_INDIA_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_IN865;
		break;
	case AS920923_JAPAN_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_AS923;
		break;
	case KR920923_SKOREA_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_KR920;
		break;
	case AS920923_MALAYSIASG_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_AS923;
		break;
	case AU915928_AUSTRALIA_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_AU915;
		break;
	case RU864870_RUSSIA_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_RU864;
		break;
	case EU863870_EUROPE_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_EU868;
		break;
	case US902928_GUAM_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_US915;
		break;
	case US902928_ANTARTICA_polygon:
		current_geofence_status.tx_permission = TX_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_US915;
		break;
	case OUTSIDE_polygon:
		current_geofence_status.tx_permission = TX_NOT_OK;
		current_geofence_status.current_loramac_region = LORAMAC_REGION_EU868;
		break;
	}
}

void update_geofence_position(float latitude, float longitude)
{
	/* get our current polygon */
	curr_poly_region = get_polygon(latitude, longitude);

	/* now set the current lora region settings based on 
	   * new or unchanged polygon we are in.
	   */
	set_current_loramac_region(curr_poly_region);
}
