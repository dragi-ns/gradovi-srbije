#include <glib.h>
#include "city.h"
#include "map_point.h"

struct city_t {
    gchar *name;
    gchar *description;
    Map_point *map_point;
};

City *city_create(const gchar *name, const gchar *description,
                  Map_point *map_point
) {
    City *city = g_slice_new(City);
    city->name = g_strdup(name);
    city->description = g_strdup(description);
    city->map_point = map_point;
    return city;
}

void city_destroy(City *city) {
    g_return_if_fail(city != NULL);

    g_free(city->name);
    g_free(city->description);
    map_point_destroy(city->map_point);
    g_slice_free(City, city);
}

gchar *city_get_name(City *city) {
    g_return_val_if_fail(city != NULL, NULL);

    return city->name;
}

void city_set_name(City *city, const gchar *name) {
    g_return_if_fail(city != NULL);

    if (city->name != NULL) {
        g_free(city->name);
    }

    city->name = g_strdup(name);
}

gchar *city_get_description(City *city) {
    g_return_val_if_fail(city != NULL, NULL);

    return city->description;
}

void city_set_description(City *city, const gchar *description) {
    g_return_if_fail(city != NULL);

    if (city->description != NULL) {
        g_free(city->description);
    }

    city->description = g_strdup(description);
}

Map_point *city_get_map_point(City *city) {
    g_return_val_if_fail(city != NULL, NULL);

    return city->map_point;
}

void city_set_map_point(City *city, Map_point *map_point) {
    g_return_if_fail(city != NULL);

    if (city->map_point != NULL) {
        map_point_destroy(city->map_point);
    }

    city->map_point = map_point;
}
