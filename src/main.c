#include <stdlib.h>
#include <gtk/gtk.h>
#include "city.h"
#include "map_point.h"
#include "game_data.h"
#include "game_logic.h"

#define RESOURCE_PATH(name) g_strdup_printf("/ns/dragi/gradovi-srbije/%s", name)
#define TIMER_FORMAT "%02d:%02d:%02d"

typedef struct {
    GtkWidget *main_window;
    GSList *mode_rb;
    GSList *difficulty_rb;
    GtkButton *mw_start_button, *mw_stop_button;
    GtkFixed *mw_map_points_fixed;

    GtkRevealer *mw_game_info_revealer;
    GtkLabel *mw_gi_question_label;
    GtkLabel *mw_gi_city_name_label;
    GtkLabel *mw_gi_timer_label;
    GtkLabel *mw_gi_correct_count_label;
    GtkLabel *mw_gi_incorrect_count_label;
    GtkLabel *mw_gi_remaining_count_label;

    GtkRevealer *mw_instruction_revealer;

    GtkPopover *question_popover;
    GtkEntry *qp_city_entry;
    GtkEntryCompletion *qp_city_entry_completion;

    GtkPopover *description_popover;
    GtkLabel *dp_city_description_label;

    GtkPopover *correct_location_popover;
    GtkLabel *clp_city_name_label;

    GtkDialog *game_end_dialog;
} App_widgets;

typedef struct app_context_t {
    App_widgets *widgets;
    Game_data *data;
    Game *game;
    GList *cities;
    GtkListStore *city_list_store;
    guint popover_timeout_id;
    GTimer *timer;
    guint timer_timeout_id;
} App_context;

// Auxiliary functions
static void load_style(void);
static void load_widgets(App_context *context, App_widgets *widgets);
static GtkListStore *create_city_list_store(App_context *context);
gboolean entry_completion_match(G_GNUC_UNUSED GtkEntryCompletion *completion,
                                const gchar *key, GtkTreeIter *iter,
                                gpointer user_data
);
static void assign_map_point_to_city(GtkWidget *widget, gpointer user_data);
static void toggle_map_points_state(App_context *context, gboolean toggle);
static guint toggle_mode_radio_buttons_state(App_widgets *widgets, gboolean toggle);
static guint toggle_difficulty_radio_buttons_state(App_widgets *widgets, gboolean toggle);
static void user_start_game(App_context *context);
static void user_stop_game(App_context *context);
static void user_restart_game(App_context *context);
static void user_check_answer(GtkButton *button, App_context *context);
static void user_next_question(App_context *context);
static void timer_start(App_context *context);
static void timer_stop(App_context *context);
static gboolean update_timer_label(gpointer user_data);
static gchar *generate_timer_str(gint seconds);
static void update_game_information(App_context *context);
static void show_map_point_description(GtkButton *button, App_context *context);
static void show_question_popover(App_context *context);
static gchar *hide_question_popover(App_context *context);
static void notify_about_correct_map_point(
    const gchar *name,
    GtkButton *button,
    App_context *context
);
static gboolean hide_correct_location_popover(gpointer user_data);
static gint show_end_game_dialog(App_context *context);

// Callback functions
void on_start_game_button_clicked(G_GNUC_UNUSED GtkButton *button, App_context *context);
void on_stop_game_button_clicked(G_GNUC_UNUSED GtkButton *button, App_context *context);
void on_map_point_button_clicked(GtkButton *button, App_context *context);
void on_qp_city_entry_activate(G_GNUC_UNUSED GtkEntry *entry, App_context *context);
gboolean on_correct_location_popover_button_press_event(G_GNUC_UNUSED GtkWidget *widget,
                                                        G_GNUC_UNUSED GdkEvent *event,
                                                        G_GNUC_UNUSED App_context *context
);
gboolean on_correct_location_popover_button_release_event(G_GNUC_UNUSED GtkWidget *widget,
                                                          G_GNUC_UNUSED GdkEvent *event,
                                                          G_GNUC_UNUSED App_context *context
);
gboolean on_correct_location_popover_key_press_event(G_GNUC_UNUSED GtkWidget *widget,
                                                     GdkEventKey *key,
                                                     G_GNUC_UNUSED App_context *context
);
gboolean on_question_popover_button_press_event(G_GNUC_UNUSED GtkWidget *widget,
                                                G_GNUC_UNUSED GdkEvent *event,
                                                G_GNUC_UNUSED App_context *context
);
gboolean on_question_popover_button_release_event(G_GNUC_UNUSED GtkWidget *widget,
                                                  G_GNUC_UNUSED GdkEvent *event,
                                                  G_GNUC_UNUSED App_context *context
);
gboolean on_question_popover_key_press_event(G_GNUC_UNUSED GtkWidget *widget,
                                             GdkEventKey *key,
                                             G_GNUC_UNUSED App_context *context);
void on_main_window_destroy(void);

int main(int argc, char *argv[]) {
    App_context *context;

    gtk_init(&argc, &argv);

    load_style();

    context = g_slice_new(App_context);
    context->widgets = g_slice_new(App_widgets);
    context->data = game_data_create();
    context->cities = game_data_get_cities(context->data);
    context->game = game_create(context->cities);
    context->city_list_store = create_city_list_store(context);
    context->popover_timeout_id = 0;
    context->timer = g_timer_new();
    g_timer_stop(context->timer);
    context->timer_timeout_id = 0;

    load_widgets(context, context->widgets);

    gtk_entry_completion_set_model(
        context->widgets->qp_city_entry_completion,
        GTK_TREE_MODEL(context->city_list_store)
    );
    gtk_entry_completion_set_match_func(
        context->widgets->qp_city_entry_completion,
        entry_completion_match,
        context,
        NULL
    );

    toggle_map_points_state(context, TRUE);

    gtk_widget_show(context->widgets->main_window);
    gtk_main();

    g_timer_destroy(context->timer);
    g_object_unref(G_OBJECT(context->city_list_store));
    game_destroy(context->game);
    g_list_free(context->cities);
    game_data_destroy(context->data);
    g_slice_free(App_widgets, context->widgets);
    g_slice_free(App_context, context);

    exit(EXIT_SUCCESS);
}

static void load_style() {
    gchar *path;
    GtkCssProvider *provider;

    provider = gtk_css_provider_new();
    path = RESOURCE_PATH("styles.css");
    gtk_css_provider_load_from_resource(provider, path);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    g_object_unref(G_OBJECT(provider));
    g_free(path);
}

static void load_widgets(App_context *context, App_widgets *widgets) {
    gchar *path;
    GtkBuilder *builder;

    path = RESOURCE_PATH("main.glade");
    builder = gtk_builder_new_from_resource(path);

    widgets->main_window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    widgets->mode_rb = gtk_radio_button_get_group(
        GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "mw_selection_rb"))
    );
    widgets->difficulty_rb = gtk_radio_button_get_group(
        GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "mw_medium_rb"))
    );
    widgets->mw_start_button = GTK_BUTTON(
        gtk_builder_get_object(builder, "mw_start_button")
    );
    widgets->mw_stop_button = GTK_BUTTON(
        gtk_builder_get_object(builder, "mw_stop_button")
    );
    widgets->mw_map_points_fixed = GTK_FIXED(
        gtk_builder_get_object(builder, "mw_map_points_fixed")
    );

    widgets->mw_game_info_revealer = GTK_REVEALER(
        gtk_builder_get_object(builder, "mw_game_info_revealer")
    );
    widgets->mw_gi_question_label = GTK_LABEL(
        gtk_builder_get_object(builder, "mw_gi_question_label")
    );
    widgets->mw_gi_timer_label = GTK_LABEL(
        gtk_builder_get_object(builder, "mw_gi_timer_label")
    );
    widgets->mw_gi_city_name_label = GTK_LABEL(
        gtk_builder_get_object(builder, "mw_gi_city_name_label")
    );
    widgets->mw_gi_correct_count_label = GTK_LABEL(
        gtk_builder_get_object(builder, "mw_gi_correct_count_label")
    );
    widgets->mw_gi_incorrect_count_label = GTK_LABEL(
        gtk_builder_get_object(builder, "mw_gi_incorrect_count_label")
    );
    widgets->mw_gi_remaining_count_label = GTK_LABEL(
        gtk_builder_get_object(builder, "mw_gi_remaining_count_label")
    );

    widgets->mw_instruction_revealer = GTK_REVEALER(
        gtk_builder_get_object(builder, "mw_instruction_revealer")
    );

    widgets->question_popover = GTK_POPOVER(
        gtk_builder_get_object(builder, "question_popover")
    );
    widgets->qp_city_entry = GTK_ENTRY(
        gtk_builder_get_object(builder, "qp_city_entry")
    );
    widgets->qp_city_entry_completion = GTK_ENTRY_COMPLETION(
        gtk_builder_get_object(builder, "qp_city_entry_completion")
    );

    widgets->description_popover = GTK_POPOVER(
        gtk_builder_get_object(builder, "description_popover")
    );
    widgets->dp_city_description_label = GTK_LABEL(
        gtk_builder_get_object(builder, "dp_city_description_label")
    );

    widgets->correct_location_popover = GTK_POPOVER(
        gtk_builder_get_object(builder, "correct_location_popover")
    );
    widgets->clp_city_name_label = GTK_LABEL(
        gtk_builder_get_object(builder, "clp_city_name_label")
    );

    widgets->game_end_dialog = GTK_DIALOG(
        gtk_builder_get_object(builder, "game_end_dialog")
    );

    gtk_container_foreach(
        GTK_CONTAINER(context->widgets->mw_map_points_fixed),
        assign_map_point_to_city,
        context
    );

    gtk_builder_connect_signals(builder, context);

    g_object_unref(G_OBJECT(builder));
    g_free(path);
}

static GtkListStore *create_city_list_store(App_context *context) {
    City *city;
    GList *i;
    GtkTreeIter tree_iter;
    GtkListStore *list_store;

    list_store = gtk_list_store_new(1, G_TYPE_STRING);
    for (i = context->cities; i != NULL; i = i->next) {
        city = (City *) i->data;

        gtk_list_store_append(list_store, &tree_iter);
        gtk_list_store_set(list_store, &tree_iter, 0, city_get_name(city), -1);
    }

    return list_store;
}

gboolean entry_completion_match(G_GNUC_UNUSED GtkEntryCompletion *completion,
                                const gchar *key, GtkTreeIter *iter,
                                gpointer user_data
) {
    gboolean possible_match;
    GValue city_name = G_VALUE_INIT;

    gtk_tree_model_get_value(
        gtk_entry_completion_get_model(((App_context *) user_data)->widgets->qp_city_entry_completion),
        iter,
        0,
        &city_name
    );

    possible_match = g_str_match_string(key, g_value_get_string(&city_name), FALSE);

    g_value_unset(&city_name);

    return possible_match;
}

static void assign_map_point_to_city(GtkWidget *widget, gpointer user_data) {
    City *city;
    Map_point *map_point;
    GList *children, *i;

    city = game_data_get_city(
        ((App_context *) user_data)->data,
        gtk_widget_get_name(widget)
    );

    if (city == NULL) {
        return;
    }

    map_point = map_point_create(
        GTK_CONTAINER(widget),
        NULL,
        NULL
    );

    children = gtk_container_get_children(
        map_point_get_container(map_point)
    );
    for (i = children; i != NULL; i = i->next) {
        if (GTK_IS_BUTTON(i->data)) {
            map_point_set_button(
                map_point,
                GTK_BUTTON(i->data)
            );
        } else if (GTK_IS_REVEALER(i->data)) {
            map_point_set_revealer(
                map_point,
                GTK_REVEALER(i->data)
            );
        }
    }
    g_list_free(children);

    city_set_map_point(
        city,
        map_point
    );
}

static void toggle_map_points_state(App_context *context, gboolean toggle) {
    GList *i;
    Map_point *map_point;

    for (i = context->cities; i != NULL; i = i->next) {
        map_point = city_get_map_point((City *) i->data);

        if (toggle) {
            map_point_toggle_class_names(map_point, FALSE, 3, "mistery", "correct", "incorrect");
            map_point_toggle_coat_of_arms(map_point, TRUE);
            map_point_toggle_name(map_point, TRUE);
            map_point_toggle_state(map_point, TRUE);
        } else {
            map_point_toggle_coat_of_arms(map_point, FALSE);
            map_point_toggle_name(map_point, FALSE);
            map_point_toggle_class_names(map_point, TRUE, 1, "mistery");

            if (game_get_mode(context->game) != SELECTION) {
                map_point_toggle_state(map_point, FALSE);
            }
        }
    }
}

static guint toggle_mode_radio_buttons_state(App_widgets *widgets, gboolean toggle) {
    GSList *i;
    guint mode = 0;

    for (i = widgets->mode_rb; i != NULL; i = i->next) {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(i->data))) {
            mode = (guint) atoi(gtk_widget_get_name(GTK_WIDGET(i->data)));
        }

        gtk_widget_set_sensitive(
            GTK_WIDGET(i->data),
            toggle
        );
    }

    return mode;
}

static guint toggle_difficulty_radio_buttons_state(App_widgets *widgets, gboolean toggle) {
    GSList *i;
    guint difficulty = 0;

    for (i = widgets->difficulty_rb; i != NULL; i = i->next) {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(i->data))) {
            difficulty = (guint) atoi(gtk_widget_get_name(GTK_WIDGET(i->data)));
        }

        gtk_widget_set_sensitive(
            GTK_WIDGET(i->data),
            toggle
        );
    }

    return difficulty;
}

static void user_start_game(App_context *context) {
    guint mode;
    guint difficulty;

    mode = toggle_mode_radio_buttons_state(context->widgets, FALSE);
    difficulty = toggle_difficulty_radio_buttons_state(context->widgets, FALSE);

    game_set_mode(context->game, mode);
    game_set_difficulty(context->game, difficulty);

    gtk_widget_hide(GTK_WIDGET(context->widgets->mw_start_button));
    gtk_widget_show(GTK_WIDGET(context->widgets->mw_stop_button));

    gtk_revealer_set_reveal_child(
        context->widgets->mw_instruction_revealer,
        FALSE
    );

    if (mode != SELECTION) {
        gtk_widget_hide(GTK_WIDGET(context->widgets->mw_gi_question_label));
        gtk_widget_hide(GTK_WIDGET(context->widgets->mw_gi_city_name_label));
    } else {
        gtk_widget_show(GTK_WIDGET(context->widgets->mw_gi_question_label));
        gtk_widget_show(GTK_WIDGET(context->widgets->mw_gi_city_name_label));
    }

    toggle_map_points_state(context, FALSE);

    game_start(context->game);
    timer_start(context);

    update_game_information(context);

    gtk_revealer_set_reveal_child(
        context->widgets->mw_game_info_revealer,
        TRUE
    );

    if (mode != SELECTION) {
        show_question_popover(context);
    }
}

static void user_stop_game(App_context *context) {
    hide_question_popover(context);

    game_stop(context->game);
    timer_stop(context);

    toggle_map_points_state(context, TRUE);

    gtk_revealer_set_reveal_child(context->widgets->mw_game_info_revealer, FALSE);
    gtk_revealer_set_reveal_child(context->widgets->mw_instruction_revealer, TRUE);

    gtk_widget_hide(GTK_WIDGET(context->widgets->mw_stop_button));
    gtk_widget_show(GTK_WIDGET(context->widgets->mw_start_button));

    toggle_mode_radio_buttons_state(context->widgets, TRUE);
    toggle_difficulty_radio_buttons_state(context->widgets, TRUE);
}

static void user_restart_game(App_context *context) {
    GList *i;
    Map_point *map_point;

    game_stop(context->game);
    timer_stop(context);

    for (i = context->cities; i != NULL; i = i->next) {
        map_point = city_get_map_point((City *) i->data);
        map_point_toggle_class_names(map_point, FALSE, 2, "correct", "incorrect");
        map_point_toggle_class_names(map_point, TRUE, 1, "mistery");
        map_point_toggle_name(map_point, FALSE);
        if (game_get_mode(context->game) == SELECTION) {
            map_point_toggle_state(map_point, TRUE);
        }
    }

    game_start(context->game);
    timer_start(context);

    update_game_information(context);

    if (game_get_mode(context->game) != SELECTION) {
        show_question_popover(context);
    }
}

static void user_check_answer(GtkButton *button, App_context *context) {
    City *city;
    Map_point *map_point;
    const gchar *user_answer;

    if (!game_is_running(context->game)) {
        show_map_point_description(button, context);
        return;
    }

    city = game_get_current_city(context->game);
    map_point = city_get_map_point(city);
    map_point_toggle_class_names(map_point, TRUE, 1, "mistery");
    map_point_toggle_name(map_point, TRUE);

    if (game_get_mode(context->game) != SELECTION) {
        user_answer = hide_question_popover(context);
    } else {
        user_answer = gtk_widget_get_name(GTK_WIDGET(button));
        map_point_toggle_state(map_point, FALSE);
    }

    if (game_check_user_answer(context->game, user_answer)) {
        map_point_toggle_class_names(map_point, TRUE, 1, "correct");
    } else {
        map_point_toggle_class_names(map_point, TRUE, 1, "incorrect");

        notify_about_correct_map_point(
            city_get_name(city),
            map_point_get_button(map_point),
            context
        );
        return;
    }

    if (game_get_mode(context->game) != SELECTION) {
        // In this case the variable user_answer is a dynamically allocated string
        g_free((gchar *) user_answer);
    }

    user_next_question(context);
}

static void user_next_question(App_context *context) {
    gint response_id;
    gboolean has_next;

    has_next = game_next_question(context->game);
    update_game_information(context);

    if (!has_next) {
        timer_stop(context);
        response_id = show_end_game_dialog(context);

        switch (response_id) {
            case GTK_RESPONSE_YES:
                user_restart_game(context);
                break;
            default:
                user_stop_game(context);
                break;
        }

        return;
    }

    if (game_get_mode(context->game) != SELECTION) {
        show_question_popover(context);
    }
}

static void timer_start(App_context *context) {
    g_timer_start(context->timer);

    context->timer_timeout_id = g_timeout_add_seconds(
        1,
        update_timer_label,
        context
    );
}

static void timer_stop(App_context *context) {
    if (context->timer_timeout_id > 0) {
        g_timer_stop(context->timer);
        g_source_remove(context->timer_timeout_id);
        context->timer_timeout_id = 0;
    }
}

static gboolean update_timer_label(gpointer user_data) {
    gint seconds;
    gchar *timer_str;

    seconds = g_timer_elapsed(((App_context *) user_data)->timer, NULL);
    timer_str = generate_timer_str(seconds);

    gtk_label_set_text(
        ((App_context *) user_data)->widgets->mw_gi_timer_label,
        timer_str
    );

    g_free(timer_str);

    return G_SOURCE_CONTINUE;
}

static gchar *generate_timer_str(gint seconds) {
    gchar *timer_str;
    gint hours, minutes;

    minutes = seconds / 60;
    hours = (minutes / 60) % 24;
    minutes %= 60;

    timer_str = g_strdup_printf(TIMER_FORMAT, hours, minutes, seconds % 60);

    return timer_str;
}

static void update_game_information(App_context *context) {
    City *city;
    const gchar *city_name_str;
    gchar *correct_count_str;
    gchar *incorrect_count_str;
    gchar *remaining_count_str;

    correct_count_str = g_strdup_printf(
        "%u",
        game_get_correct_answer_count(context->game)
    );

    incorrect_count_str = g_strdup_printf(
        "%u",
        game_get_incorrect_answer_count(context->game)
    );

    remaining_count_str = g_strdup_printf(
        "%u",
        game_get_remaining_questions_count(context->game)
    );

    if (game_get_mode(context->game) == SELECTION) {
        city = game_get_current_city(context->game);
        if (city == NULL) {
            city_name_str = "-";
        } else {
            city_name_str = city_get_name(city);
        }

        gtk_label_set_text(
            context->widgets->mw_gi_city_name_label,
            city_name_str
        );
    }

    update_timer_label(context);

    gtk_label_set_text(
        context->widgets->mw_gi_correct_count_label,
        correct_count_str
    );
    gtk_label_set_text(
        context->widgets->mw_gi_incorrect_count_label,
        incorrect_count_str
    );
    gtk_label_set_text(
        context->widgets->mw_gi_remaining_count_label,
        remaining_count_str
    );

    // gtk_label_set_text uses g_strdup internally
    g_free(correct_count_str);
    g_free(incorrect_count_str);
    g_free(remaining_count_str);
}

static void show_map_point_description(GtkButton *button, App_context *context) {
    City *city;

    city = game_data_get_city(
        context->data,
        gtk_widget_get_name(GTK_WIDGET(button))
    );

    if (city == NULL) {
        return;
    }

    gtk_label_set_markup(
        context->widgets->dp_city_description_label,
        city_get_description(city)
    );

    gtk_popover_set_relative_to(
        context->widgets->description_popover,
        GTK_WIDGET(button)
    );

    gtk_popover_popup(context->widgets->description_popover);
}

static void show_question_popover(App_context *context) {
    City *city;
    Map_point *map_point;

    city = game_get_current_city(context->game);
    if (city == NULL) {
        return;
    }

    map_point = city_get_map_point(city);

    gtk_popover_set_relative_to(
        context->widgets->question_popover,
        GTK_WIDGET(map_point_get_button(map_point))
    );

    gtk_widget_set_sensitive(
        GTK_WIDGET(context->widgets->question_popover),
        TRUE
    );

    gtk_popover_popup(
        context->widgets->question_popover
    );

    gtk_widget_grab_focus(
        GTK_WIDGET(context->widgets->qp_city_entry)
    );
}

static gchar *hide_question_popover(App_context *context) {
    gchar *user_answer;

    if (!game_is_running(context->game)) {
        user_answer = NULL;
    } else {
        user_answer = g_strdup(gtk_entry_get_text(context->widgets->qp_city_entry));
    }

    gtk_entry_set_text(context->widgets->qp_city_entry, "");

    gtk_widget_hide(GTK_WIDGET(context->widgets->question_popover));

    return user_answer;
}

static void notify_about_correct_map_point(const gchar *name, GtkButton *button,
                                           App_context *context
) {
    gtk_label_set_text(context->widgets->clp_city_name_label, name);

    gtk_popover_set_relative_to(
        context->widgets->correct_location_popover,
        GTK_WIDGET(button)
    );

    gtk_widget_set_sensitive(
        GTK_WIDGET(context->widgets->correct_location_popover),
        TRUE
    );

    gtk_popover_popup(context->widgets->correct_location_popover);

    context->popover_timeout_id = g_timeout_add_seconds(
        1,
        hide_correct_location_popover,
        context
    );
}

static gboolean hide_correct_location_popover(gpointer user_data) {
    if (((App_context *) user_data)->popover_timeout_id > 0) {
        gtk_widget_hide(
            GTK_WIDGET(((App_context *) user_data)->widgets->correct_location_popover)
        );

        user_next_question((App_context *) user_data);

        ((App_context *) user_data)->popover_timeout_id = 0;
    }

    return G_SOURCE_REMOVE;
}

static gint show_end_game_dialog(App_context *context) {
    gint response_id;
    gchar *timer_str;

    timer_str = generate_timer_str(g_timer_elapsed(context->timer, NULL));

    gtk_message_dialog_format_secondary_text(
        GTK_MESSAGE_DIALOG(context->widgets->game_end_dialog),
        "Vreme: %s\nTačnih odgovora: %u\nNetačnih odgovora: %u",
        timer_str,
        game_get_correct_answer_count(context->game),
        game_get_incorrect_answer_count(context->game)
    );

    g_free(timer_str);

    response_id = gtk_dialog_run(context->widgets->game_end_dialog);
    gtk_widget_hide(GTK_WIDGET(context->widgets->game_end_dialog));

    return response_id;
}

void on_start_game_button_clicked(G_GNUC_UNUSED GtkButton *button, App_context *context) {
    user_start_game(context);
}

void on_stop_game_button_clicked(G_GNUC_UNUSED GtkButton *button, App_context *context) {
    user_stop_game(context);
}

void on_map_point_button_clicked(GtkButton *button, App_context *context) {
    user_check_answer(button, context);
}

void on_qp_city_entry_activate(G_GNUC_UNUSED GtkEntry *entry, App_context *context) {
    user_check_answer(NULL, context);
}

gboolean on_correct_location_popover_button_press_event(G_GNUC_UNUSED GtkWidget *widget,
                                                        G_GNUC_UNUSED GdkEvent *event,
                                                        G_GNUC_UNUSED App_context *context
) {
    return TRUE;
}

gboolean on_correct_location_popover_button_release_event(G_GNUC_UNUSED GtkWidget *widget,
                                                          G_GNUC_UNUSED GdkEvent *event,
                                                          G_GNUC_UNUSED App_context *context
) {
    return TRUE;
}

gboolean on_correct_location_popover_key_press_event(G_GNUC_UNUSED GtkWidget *widget,
                                                     GdkEventKey *key,
                                                     G_GNUC_UNUSED App_context *context
) {
    if (key->keyval == GDK_KEY_Escape) {
        return TRUE;
    }

    return FALSE;
}

gboolean on_question_popover_button_press_event(G_GNUC_UNUSED GtkWidget *widget,
                                                G_GNUC_UNUSED GdkEvent *event,
                                                G_GNUC_UNUSED App_context *context
) {
    return TRUE;
}

gboolean on_question_popover_button_release_event(G_GNUC_UNUSED GtkWidget *widget,
                                                  G_GNUC_UNUSED GdkEvent *event,
                                                  G_GNUC_UNUSED App_context *context
) {
    return TRUE;
}

gboolean on_question_popover_key_press_event(G_GNUC_UNUSED GtkWidget *widget,
                                             GdkEventKey *key,
                                             G_GNUC_UNUSED App_context *context
) {
    if (key->keyval == GDK_KEY_Escape) {
        return TRUE;
    }

    return FALSE;
}

void on_main_window_destroy() {
    gtk_main_quit();
}
