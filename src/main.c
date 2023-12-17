/* -*- compile-command: "make -C .. debug" -*- */
#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>

#include "plug.h"

#ifdef HOTRELOAD
#include <dlfcn.h>
#endif // HOTRELOAD

const char *libplug_file_name = "libplug.so";
void *libplug = NULL;

// macro utilisé pour définir une fonction de plug (avec ou sans hotreload)
#define BASE_PLUG(return_type, name, ...) PLUG(name)
// Si le hotreload est activé, les fonctions de plug sont déclarées comme des pointeurs.
// Sinon, elles sont déclarées directement comme des objets.
#ifdef HOTRELOAD
#  define PLUG(name) name##_t *name = NULL;
#else
#  define PLUG(name) name##_t name;
#endif // HOTRELOAD
LIST_OF_PLUGS
#undef PLUG
#undef BASE_PLUG

// instance de la structure Plug initialisée à zéro
Plug plug = {0};

#ifdef HOTRELOAD
/**
 * @brief Recharge la bibliothèque dynamique libplug.so pour le hot-reloading.
 *
 * Si HOTRELOAD est activé, cette fonction ferme la bibliothèque précédemment chargée,
 * charge la nouvelle bibliothèque libplug.so, et redéfinit les fonctions du plug
 * avec leurs nouvelles adresses. Si HOTRELOAD n'est pas activé, cette fonction renvoie toujours true.
 *
 * @return Retourne true si le rechargement est réussi ou si HOTRELOAD n'est pas activé, false sinon.
 */
bool reload_libplug(void) {
    // Fermeture de la librairie précédemment chargée, si elle existe
    if (libplug != NULL) dlclose(libplug);

    // Chargement de la nouvelle librairie libplug.so
    libplug = dlopen(libplug_file_name, RTLD_NOW);
    if (libplug == NULL) {
	fprintf(stderr, "ERROR: could not load %s: %s", libplug_file_name, dlerror());
	return false;
    }

    // Redéfinition des fonctions de plug avec les nouvelles adresses après le chargement.
#define BASE_PLUG(return_type, name, ...) PLUG(name)
#define PLUG(name) \
    name = dlsym(libplug, #name); \
    if (name == NULL) { \
    	fprintf(stderr, "ERROR: could not find %s in %s: %s\n", #name, libplug_file_name, dlerror()); \
    	return false; \
    }
    LIST_OF_PLUGS
#undef PLUG
#undef BASE_PLUG

    return true;
}
#else
// Si le hotreload n'est pas activé, cette fonction retourne toujours true
#define reload_libplug() true
#endif // HOTRELOAD

int main(void) {

    if (!reload_libplug()) return 1;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lemmings");

    // Chargement de tout les textures pour le jeu
    Texture2D background = LoadTexture("./assets/pixel-platformer/Tilemap/newtest.png");
    Texture2D tileset = LoadTexture("./assets/pixel-platformer/Tilemap/scaled_packed.png");
    Texture2D player = LoadTexture("./assets/pixel-platformer/Tilemap/tilemap-characters_scaled.png");
    Texture2D player_flop = LoadTexture("./assets/pixel-platformer/Tilemap/tilemap-characters_scaled_flop.png");

    SetTargetFPS(60);

    plug_init(&plug);

    while(!plug.window_should_close && !WindowShouldClose()) {
        plug_temp(&plug);
	plug_update(&plug);

	if (IsKeyPressed(KEY_Z)) plug.camera.zoom = 1.0f;

	if (IsKeyPressed(KEY_R) && plug.dialog == DIALOG_NONE) {
	    if (!reload_libplug()) return 1;
	}

	plug_render(&plug, background, tileset, player, player_flop);
    }

    plug_free(&plug);

    UnloadTexture(background);
    UnloadTexture(tileset);
    UnloadTexture(player);
    UnloadTexture(player_flop);
    CloseWindow();

    return 0;
}
