#ifndef PLUG_H_
#define PLUG_H_

#include <stdbool.h>
#include "raylib.h"
#include "raymath.h"
#include "entity.h"
#include "layout.h"
#include "xml.h"

#define SCREEN_WIDTH 792
#define SCREEN_HEIGHT 468
#define MAP_TILE_SIZE 36

#define TILESX SCREEN_WIDTH/MAP_TILE_SIZE
#define TILESY SCREEN_HEIGHT/MAP_TILE_SIZE

typedef enum {
    BLOCK_EMPTY    = 0,
    BLOCK_MIDDLE   = 1 << 0,
    BLOCK_LEFT     = 1 << 1,
    BLOCK_TOP      = 1 << 2,
    BLOCK_RIGHT    = 1 << 3,
    BLOCK_BOTTOM   = 1 << 4,
    BLOCK_COIN     = 1 << 5,
    BLOCK_SPIKE    = (1 << 5) + 1,
    BLOCK_LEVER    = (1 << 5) + 2,
    BLOCK_S_BRICK  = (1 << 5) + 3,
    BLOCK_B_BRICK  = (1 << 5) + 4,
    BLOCK_DOOR     = (1 << 5) + 5,
    BLOCK_BRICK    = (1 << 5) + 6,
} BlockID;

typedef enum {
    DIALOG_NONE,
    DIALOG_EDITOR,
    DIALOG_SELECT,
    DIALOG_GAME,
    DIALOG_PAUSE,
} DialogState;

typedef enum {
    EDITOR,
    GAME,
    START_MENU,
} GameState;

typedef struct {
    int x;
    int y;
} Tile2D;

typedef union {
    Tile2D entity;
    int block_id;
} Value;

typedef enum {
    ENTITY,
    BLOCK,
} Key;

typedef struct {
    Value value;
    Key key;
} Item;

typedef struct Plug {
    int tilemap[TILESY][TILESX];
    Camera2D camera;
    Vector2 mouse_position;
    Tile2D mouse_tile_pos;
    bool eraser;
    Item item_selected;
    bool show;
    Entity *players;
    GameState state;
    DialogState dialog;
    Layout *layouts;
    char **paths;
    size_t level_selected;
    int attempt;
    int score;
    int max_coins;
    int coins;
    int bricks;
    size_t page;
    bool window_should_close;
} Plug;

// Définition de la liste des fonctions plug avec leurs signatures (X macro: https://en.wikipedia.org/wiki/X_macro).
#define LIST_OF_PLUGS \
    BASE_PLUG(void, plug_init, Plug *plug)				\
    BASE_PLUG(void, plug_temp, Plug *plug)				\
    BASE_PLUG(void, plug_update, Plug *plug)				\
    BASE_PLUG(void, plug_render, Plug *plug, Texture2D background, Texture2D tileset, Texture2D player, Texture2D player_flop) \
    BASE_PLUG(void, plug_save, Plug *plug, char *file_path)	\
    BASE_PLUG(void, plug_free, Plug *plug)

// définition de chaque fonction de plug comme un type de fonction avec le préfixe '_t'
#define BASE_PLUG(return_type, name, ...) typedef return_type (name##_t)(__VA_ARGS__);
// expansion de la liste des fonctions plug pour générer les types de fonction correspondants
LIST_OF_PLUGS
// suppression de la définition de la macro BASE_PLUG pour eviter toute collision de nom
#undef BASE_PLUG

#endif // PLUG_H_
