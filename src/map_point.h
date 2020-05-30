#ifndef MAP_POINT_H
#define MAP_POINT_H

#include <gtk/gtk.h>

typedef struct map_point_t Map_point;

Map_point *map_point_create(GtkContainer *container, GtkButton *button,
                            GtkRevealer *revealer
);
void map_point_destroy(Map_point *map_point);
GtkContainer *map_point_get_container(Map_point *map_point);
void map_point_set_container(Map_point *map_point, GtkContainer *container);
GtkButton *map_point_get_button(Map_point *map_point);
void map_point_set_button(Map_point *map_point, GtkButton *button);
GtkRevealer *map_point_get_revealer(Map_point *map_point);
void map_point_set_revealer(Map_point *map_point, GtkRevealer *revealer);
void map_point_toggle_class_names(Map_point *map_point, gboolean toggle,
                                  gint arg_count, ...
);
void map_point_toggle_coat_of_arms(Map_point *map_point, gboolean toggle);
void map_point_toggle_name(Map_point *map_point, gboolean toggle);
void map_point_toggle_state(Map_point *map_point, gboolean toggle);

#endif
