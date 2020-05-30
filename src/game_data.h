#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <glib.h>
#include "city.h"

typedef struct game_data_t Game_data;

Game_data *game_data_create();
void game_data_destroy(Game_data *data);
City *game_data_get_city(Game_data *data, const gchar *name);
GList *game_data_get_cities(Game_data *data);

#endif
