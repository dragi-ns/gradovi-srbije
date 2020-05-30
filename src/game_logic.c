#include <glib.h>
#include "game_logic.h"
#include "city.h"

#define CITIES_COUNT 29

#define GAME_RETURN_IF_RUNNING(game)                    \
if (game_is_running(game)) {                            \
    g_warning("In %s: the game is running!", __func__); \
    return;                                             \
}                                                       \

#define GAME_RETURN_VAL_IF_RUNNING(game, val)                   \
if (game_is_running(game)) {                                    \
    g_warning("In %s: the game is running!", __func__);         \
    return val;                                                 \
}                                                               \

#define GAME_RETURN_IF_NOT_RUNNING(game)                    \
if (!game_is_running(game)) {                               \
    g_warning("In %s: the game is not running!", __func__); \
    return;                                                 \
}                                                           \

#define GAME_RETURN_VAL_IF_NOT_RUNNING(game, val)               \
if (!game_is_running(game)) {                                   \
    g_warning("In %s: the game is not running!", __func__);     \
    return val;                                                 \
}                                                               \

typedef enum game_state_t {NOT_RUNNING, RUNNING} Game_state;

struct game_t {
    GList *cities;
    Game_state state;
    Game_mode mode;
    Game_difficulty difficulty;
    GList *random_cities;
    GList *current_node;
    guint correct_answer_count;
    guint incorrect_answer_count;
    guint remaining_questions_count;
};

static void game_pick_random_cities(Game *game);

Game *game_create(GList *cities) {
    g_return_val_if_fail(cities != NULL, NULL);

    Game *game;

    game = g_slice_new0(Game);
    game->cities = cities;

    return game;
}

void game_destroy(Game *game) {
    g_return_if_fail(game != NULL);

    g_list_free(game->random_cities);
    g_slice_free(Game, game);
}

Game_mode game_get_mode(Game *game) {
    g_return_val_if_fail(game != NULL, 0);

    return game->mode;
}

void game_set_mode(Game *game, Game_mode mode) {
    g_return_if_fail(game != NULL);
    g_return_if_fail(
        mode == SELECTION ||
        mode == TYPING
    );

    GAME_RETURN_IF_RUNNING(game);

    game->mode = mode;
}

Game_difficulty game_get_difficulty(Game *game) {
    g_return_val_if_fail(game != NULL, 0);

    return game->difficulty;
}

void game_set_difficulty(Game *game, Game_difficulty difficulty) {
    g_return_if_fail(game != NULL);
    g_return_if_fail(
        difficulty == EASY ||
        difficulty == MEDIUM ||
        difficulty == HARD
    );

    GAME_RETURN_IF_RUNNING(game);

    game->difficulty = difficulty;
    game->remaining_questions_count = (guint) game->difficulty;
}

City *game_get_current_city(Game *game) {
    g_return_val_if_fail(game != NULL, NULL);

    GAME_RETURN_VAL_IF_NOT_RUNNING(game, NULL);

    if (game->current_node == NULL) {
        return NULL;
    }

    return (City *) game->current_node->data;
}

guint game_get_correct_answer_count(Game *game) {
    g_return_val_if_fail(game != NULL, 0);

    GAME_RETURN_VAL_IF_NOT_RUNNING(game, 0);

    return game->correct_answer_count;
}

guint game_get_incorrect_answer_count(Game *game) {
    g_return_val_if_fail(game != NULL, 0);

    GAME_RETURN_VAL_IF_NOT_RUNNING(game, 0);

    return game->incorrect_answer_count;
}

guint game_get_remaining_questions_count(Game *game) {
    g_return_val_if_fail(game != NULL, 0);

    GAME_RETURN_VAL_IF_NOT_RUNNING(game, 0);

    return game->remaining_questions_count;
}

gboolean game_is_running(Game *game) {
    g_return_val_if_fail(game != NULL, FALSE);

    return game->state == RUNNING;
}

void game_start(Game *game) {
    g_return_if_fail(game != NULL);

    GAME_RETURN_IF_RUNNING(game);

    game_pick_random_cities(game);
    game->current_node = game->random_cities;
    game->state = RUNNING;
}

void game_stop(Game *game) {
    g_return_if_fail(game != NULL);

    GAME_RETURN_IF_NOT_RUNNING(game);

    g_list_free(game->random_cities);
    game->state = NOT_RUNNING;
    game->random_cities = NULL;
    game->current_node = NULL;
    game->correct_answer_count = 0;
    game->incorrect_answer_count = 0;
    game->remaining_questions_count = (guint) game->difficulty;
}

gboolean game_check_user_answer(Game *game, const gchar *name) {
    g_return_val_if_fail(game != NULL, FALSE);

    GAME_RETURN_VAL_IF_NOT_RUNNING(game, FALSE);

    City *city;
    gboolean correct;
    gchar *city_name_casefold;
    gchar *user_answer_casefold;

    city = game_get_current_city(game);
    if (city == NULL) {
        return FALSE;
    }

    city_name_casefold = g_utf8_casefold(
        city_get_name(city),
        -1
    );
    user_answer_casefold = g_utf8_casefold(name, -1);

    if (g_strcmp0(city_name_casefold, user_answer_casefold) == 0) {
        correct = TRUE;
        game->correct_answer_count++;
    } else {
        correct = FALSE;
        game->incorrect_answer_count++;
    }

    g_free(city_name_casefold);
    g_free(user_answer_casefold);

    return correct;
}

gboolean game_next_question(Game *game) {
    g_return_val_if_fail(game != NULL, FALSE);

    GAME_RETURN_VAL_IF_NOT_RUNNING(game, FALSE);

    if (game->current_node == NULL) {
        return FALSE;
    }

    game->current_node = game->current_node->next;
    game->remaining_questions_count--;

    return game->current_node != NULL;
}

static void game_pick_random_cities(Game *game) {
    gint32 random_index;
    guint picked_cities_count;

    GRand *random_generator;

    random_generator = g_rand_new();

    picked_cities_count = 0;
    do {
        random_index = g_rand_int_range(
            random_generator,
            0,
            CITIES_COUNT
        );

        City *random_city = (City *) g_list_nth_data(
            game->cities,
            (guint) random_index
        );

        if (g_list_index(game->random_cities, random_city) != -1) {
            continue;
        }

        game->random_cities = g_list_append(
            game->random_cities,
            random_city
        );

        picked_cities_count++;
    } while (picked_cities_count < game->remaining_questions_count);

    g_rand_free(random_generator);
}
