#ifndef ENTITY_H_
#define ENTITY_H_

#include <stdbool.h>
#include "raylib.h"

#define PLAYER_SPEED 90

/**
 * @struct Plug
 * @brief Représente la structure principale contenant des informations sur l'état du jeu.
 */
typedef struct Plug Plug;

/**
 * @enum EntityType
 * @brief Énumération représentant le type d'entités de jeu.
 */
typedef enum {
    PLAYER, /**< Entité joueur. */
    ENEMY,  /**< Entité ennemie. */
} EntityType;

/**
 * @enum State
 * @brief Énumération représentant l'état d'une entité.
 */
typedef enum State {
    STATIC,
    MOVE_LEFT,
    MOVE_RIGHT,
} State;

/**
 * @struct Entity
 * @brief Représente une entité dans le jeu.
 */
typedef struct {
    Rectangle rect;   /**< Rectangle représentant la position et la taille de l'entité. */
    Vector2 velocity; /**< Vecteur de vélocité de l'entité. */
    EntityType type;  /**< Type de l'entité (par exemple, Joueur ou Ennemi). */
    State state;      /**< État de l'entité (par exemple, Statique, Déplacement à gauche, Déplacement à droite). */
    bool on_ground;   /**< Booléen indiquant si l'entité est actuellement au sol. */
} Entity;

/**
 * @brief Met à jour les entités de jeu en fonction de l'état actuel du jeu.
 *
 * Cette fonction est responsable de la mise à jour de la position et de l'état des entités dans le jeu.
 *
 * @param plug Pointeur vers la structure principale du jeu.
 */
void entity_update(Plug *plug);

/**
 * @brief Initialise une nouvelle entité avec les coordonnées spécifiées.
 *
 * Cette fonction crée une nouvelle entité avec les coordonnées spécifiées et des valeurs par défaut.
 *
 * @param x Coordonnée X de l'entité.
 * @param y Coordonnée Y de l'entité.
 * @return Structure Entity initialisée.
 */
Entity entity_init(int x, int y);

#endif // ENTITY_H_
