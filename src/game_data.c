#include <glib.h>
#include <json-glib/json-glib.h>
#include "city.h"
#include "resources.h"
#include "game_data.h"

struct game_data_t {
    GHashTable *cities;
};

static void game_data_add_city(G_GNUC_UNUSED JsonArray *array,
                               G_GNUC_UNUSED guint index_,
                               JsonNode *element_node,
                               gpointer user_data
);
static void game_data_city_free(gpointer user_data);

Game_data *game_data_create() {
    GResource *resource;
    GInputStream *stream;
    Game_data *data;
    GError *error = NULL;
    JsonParser *parser;

    resource = gradovi_srbije_get_resource();
    stream = g_resource_open_stream(
        resource, "/ns/dragi/gradovi-srbije/cities.json",
        G_RESOURCE_LOOKUP_FLAGS_NONE, &error
    );

    if (error != NULL) {
        g_printerr("%s\n", error->message);

        g_error_free(error);
        return NULL;
    }

    parser = json_parser_new_immutable();
    json_parser_load_from_stream(parser, stream, NULL, &error);

    if (error != NULL) {
        g_printerr("%s\n", error->message);

        g_error_free(error);
        g_object_unref(G_OBJECT(parser));
        g_object_unref(G_OBJECT(stream));
        return NULL;
    }

    data = g_slice_new(Game_data);
    data->cities = g_hash_table_new_full(
        g_str_hash, g_str_equal,
        NULL, game_data_city_free
    );

    json_array_foreach_element(
        json_node_get_array(json_parser_get_root(parser)),
        game_data_add_city,
        data
    );

    g_object_unref(G_OBJECT(parser));
    g_object_unref(G_OBJECT(stream));
    return data;
}

void game_data_destroy(Game_data *data) {
    g_return_if_fail(data != NULL);

    g_hash_table_destroy(data->cities);
    g_slice_free(Game_data, data);
}

City *game_data_get_city(Game_data *data, const gchar *name) {
    g_return_val_if_fail(data != NULL, NULL);

    return (City *) g_hash_table_lookup(data->cities, name);
}

GList *game_data_get_cities(Game_data *data) {
    g_return_val_if_fail(data != NULL, NULL);

    return g_hash_table_get_values(data->cities);
}

static void game_data_add_city(G_GNUC_UNUSED JsonArray *array,
                               G_GNUC_UNUSED guint index_,
                               JsonNode *element_node,
                               gpointer user_data
) {
    City *city;
    JsonObject *element_object;

    element_object = json_node_get_object(element_node);

    city = city_create(
        json_node_get_string(
            json_object_get_member(element_object, "name")
        ),
        json_node_get_string(
            json_object_get_member(element_object, "description")
        ),
        NULL
    );

    g_hash_table_insert(
        ((Game_data *) user_data)->cities,
        city_get_name(city),
        city
    );
}

static void game_data_city_free(gpointer user_data) {
    city_destroy((City *) user_data);
}
