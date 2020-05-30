#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <glib.h>
#include "city.h"

typedef struct game_t Game;
typedef enum game_difficulty_t {EASY = 9, MEDIUM = 19, HARD = 29} Game_difficulty;
typedef enum game_mode_t {SELECTION, TYPING} Game_mode;

Game *game_create(GList *cities);
void game_destroy(Game *game);
Game_difficulty game_get_difficulty(Game *game);
void game_set_difficulty(Game *game, Game_difficulty difficulty);
Game_mode game_get_mode(Game *game);
void game_set_mode(Game *game, Game_mode mode);
City *game_get_current_city(Game *game);
gchar *game_get_current_city_name(Game *game);
guint game_get_correct_answer_count(Game *game);
guint game_get_incorrect_answer_count(Game *game);
guint game_get_remaining_questions_count(Game *game);
gboolean game_is_running(Game *game);
void game_start(Game *game);
void game_stop(Game *game);
gboolean game_check_user_answer(Game *game, const gchar *name);
gboolean game_next_question(Game *game);

#endif
