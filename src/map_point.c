#include <stdarg.h>
#include <gtk/gtk.h>
#include "map_point.h"

#define RESOURCE_PATH(name) g_strdup_printf("/ns/dragi/gradovi-srbije/%s", name)

struct map_point_t {
    GtkContainer *container;
    GtkButton *button;
    GtkRevealer *revealer;
};

Map_point *map_point_create(GtkContainer *container, GtkButton *button,
                            GtkRevealer *revealer
) {
    Map_point *map_point = g_slice_new(Map_point);
    map_point->container = container;
    map_point->button = button;
    map_point->revealer = revealer;
    return map_point;
}

void map_point_destroy(Map_point *map_point) {
    g_return_if_fail(map_point != NULL);

    g_slice_free(Map_point, map_point);
}

GtkContainer *map_point_get_container(Map_point *map_point) {
    g_return_val_if_fail(map_point != NULL, NULL);

    return map_point->container;
}

void map_point_set_container(Map_point *map_point, GtkContainer *container) {
    g_return_if_fail(map_point != NULL);

    map_point->container = container;
}

GtkButton *map_point_get_button(Map_point *map_point) {
    g_return_val_if_fail(map_point != NULL, NULL);

    return map_point->button;
}

void map_point_set_button(Map_point *map_point, GtkButton *button) {
    g_return_if_fail(map_point != NULL);

    map_point->button = button;
}

GtkRevealer *map_point_get_revealer(Map_point *map_point) {
    g_return_val_if_fail(map_point != NULL, NULL);

    return map_point->revealer;
}

void map_point_set_revealer(Map_point *map_point, GtkRevealer *revealer) {
    g_return_if_fail(map_point != NULL);

    map_point->revealer = revealer;
}

void map_point_toggle_class_names(Map_point *map_point, gboolean toggle,
                                  gint arg_count, ...
) {
    g_return_if_fail(map_point != NULL);

    gint i;
    va_list class_names;
    const gchar *class_name;
    GtkStyleContext *style_context;

    va_start(class_names, arg_count);

    style_context = gtk_widget_get_style_context(
        GTK_WIDGET(map_point->container)
    );

    for (i = 0; i < arg_count; i++) {
        class_name = va_arg(class_names, const gchar *);

        if (gtk_style_context_has_class(style_context, class_name)) {
            if (!toggle) {
                gtk_style_context_remove_class(style_context, class_name);
            }
        } else {
            if (toggle) {
                gtk_style_context_add_class(style_context, class_name);
            }
        }
    }

    va_end(class_names);
}

void map_point_toggle_coat_of_arms(Map_point *map_point, gboolean toggle) {
    g_return_if_fail(map_point != NULL);

    gchar *path;
    GtkWidget *button_image;

    button_image = gtk_button_get_image(map_point->button);

    if (button_image == NULL) {
        button_image = gtk_image_new();
        // The button now "owns" the image.
        // So there is no need to call g_object_unref
        // on the image.
        gtk_button_set_image(map_point->button, button_image);
    }

    if (toggle) {
        path = RESOURCE_PATH(gtk_widget_get_name(
            GTK_WIDGET(map_point->container)
        ));

        gtk_image_set_from_resource(GTK_IMAGE(button_image), path);

        g_free(path);
    } else {
        gtk_image_clear(GTK_IMAGE(button_image));
    }
}

void map_point_toggle_name(Map_point *map_point, gboolean toggle) {
    g_return_if_fail(map_point != NULL);

    gtk_revealer_set_reveal_child(map_point->revealer, toggle);
}

void map_point_toggle_state(Map_point *map_point, gboolean toggle) {
    g_return_if_fail(map_point != NULL);

    gtk_widget_set_sensitive(GTK_WIDGET(map_point->button), toggle);
}
