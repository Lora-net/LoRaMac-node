# Created by Medad Rufus Newman on 27/12/2019
# THis program auto genenerates the .c and .h files for geofencing for the picotracker Lora
# KML has to be exported from google earth, not google maps.

# Before using, install the following packages:
# pip install bs4
# pip install lxml

from bs4 import BeautifulSoup


class fence:
    def __init__(self, fence_name, frequency_band, coordinates_count, all_coords_str):
        self.fence_name = fence_name
        self.frequency_band = frequency_band
        self.coordinates_count = coordinates_count
        self.all_coords_str = all_coords_str


def process_coords(raw):
    raw = raw.strip()
    raw = raw.split(" ")
    coordinates_count = 0
    all_coords_str = ""
    for coord in raw:
        i, j, k = coord.split(",")
        coord = "{0:.5f},{1:.5f},".format(float(i), float(j))
        all_coords_str += coord
        all_coords_str += "\n"
        coordinates_count += 1
    #raw  = [i+j for i,j,k in raw.split(",")]
    return all_coords_str, coordinates_count


with open("geofence.kml", encoding='utf8') as f:
    soup = BeautifulSoup(f, 'xml')

# print(soup.prettify())


polygon_region_lookup_table = {"EU863870": "LORAMAC_REGION_EU868",
                               "AS923925": "LORAMAC_REGION_AS923",
                               "AS920923": "LORAMAC_REGION_AS923",
                               "US902928": "LORAMAC_REGION_US915",
                               "AU915928": "LORAMAC_REGION_AU915",
                               "RU864870": "LORAMAC_REGION_RU864",
                               "KR920923": "LORAMAC_REGION_KR920",
                               "IN865867": "LORAMAC_REGION_IN865",
                               "CN779787": "LORAMAC_REGION_CN470"

                               }

results = soup.find_all('Placemark')

fences = []

# print c code for Geofence.h
print("// GEOFENCE ARRAYS (longitude, latitude)")
for i in results:
    fence_name = i.find("name").get_text().replace("-", "")
    frequency_band = fence_name.split("_")[0]
    country = fence_name.split("_")[-1]

    all_coords_str, coordinates_count = process_coords(
        i.find("coordinates").get_text())
    print("static const float " + fence_name + "_" +
          "F[{0}] = {{ ".format(coordinates_count * 2))

    print(all_coords_str)
    print("};")
    print("")
    fences.append(fence(fence_name, frequency_band,
                  coordinates_count, all_coords_str))


# print c code for Geofence.c
for i in fences:
    line = """
else if(pointInPolygonF({0}, {3}_F, latitude, longitude) == 1)
{{
    return {3}_polygon;
}}""".format(i.coordinates_count, i.frequency_band, polygon_region_lookup_table[i.frequency_band], i.fence_name)
    print(line)


print()
print()
print()


# code for Geofence.h data structure enum

print("typedef enum polygon_t {")
for i in fences:
    line = "{0}_polygon,".format(i.fence_name)
    print(line)

print("}Polygon_t;")


print("")
# For the switch statement

start_switch = \
    """switch (current_poly)
{"""

print(start_switch)

for i in fences:
    line = 	\
        """case {0}_polygon:
    current_geofence_status.tx_permission = TX_OK;
    current_geofence_status.current_loramac_region = {1};
    break;""".format(i.fence_name, polygon_region_lookup_table[i.frequency_band])
    print(line)

print("}")
