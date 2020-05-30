#ifndef CITY_H
#define CITY_H

#include <glib.h>
#include "map_point.h"

typedef struct city_t City;

City *city_create(const gchar *name, const gchar *description,
                  Map_point *map_point
);
void city_destroy(City *city);
gchar *city_get_name(City *city);
void city_set_name(City *city, const gchar *name);
gchar *city_get_description(City *city);
void city_set_description(City *city, const gchar *description);
Map_point *city_get_map_point(City *city);
void city_set_map_point(City *city, Map_point *map_point);

#endif
