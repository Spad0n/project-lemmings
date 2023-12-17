#ifndef UI_H_
#define UI_H_

#include <stddef.h>
#include "raylib.h"

/**
 * @brief Macro pour faciliter le dessin d'une mise en page (layout) à l'aide d'une boucle for.
 *
 * Cette macro simplifie l'utilisation de la mise en page en créant une boucle for avec une instruction
 * break pour effectuer le dessin avec le contexte de mise en page spécifié.
 *
 * @param layout Pointeur vers le tableau de mises en page.
 * @param orient Orientation de la mise en page (LO_HORI ou LO_VERT).
 * @param layout_rect Rectangle délimitant la mise en page.
 * @param count Nombre d'éléments dans la mise en page.
 * @param gap Espacement entre les éléments de la mise en page.
 */
#define LayoutDrawing(layout, orient, layout_rect, count, gap)	\
    for (int _break = (layout_stack_push((layout), (orient), (layout_rect), (count), (gap)), 1); _break; \
	 _break = 0, layout_stack_pop(layout))

#define layout_stack_slot(ls) layout_stack_slot_loc(ls, __FILE__, __LINE__)

/**
 * @brief Enumération représentant l'orientation d'une mise en page.
 */
typedef enum {
    LO_HORI, /**< Mise en page horizontale. */
    LO_VERT, /**< Mise en page verticale. */
} Layout_Orient;

/**
 * @brief Structure représentant une mise en page (layout) individuelle.
 */
typedef struct {
    Layout_Orient orient; /**< Orientation de la mise en page (LO_HORI ou LO_VERT). */
    Rectangle rect;       /**< Rectangle délimitant la mise en page. */
    size_t i;             /**< Indice actuel de la mise en page. */
    size_t count;         /**< Nombre d'éléments dans la mise en page. */
    float gap;            /**< Espacement entre les éléments de la mise en page. */
} Layout;

/**
 * @brief Fonction pour dessiner un élément de la mise en page avec une texture et un rectangle source.
 *
 * Cette fonction est utilisée pour dessiner un élément de la mise en page avec une texture spécifiée
 * et un rectangle source optionnel. Elle permet également de marquer l'élément comme sélectionné.
 *
 * @param selected Booléen indiquant si l'élément est sélectionné ou non.
 * @param texture Texture2D à dessiner.
 * @param rect Rectangle de dessin pour l'élément.
 * @param source Rectangle source dans la texture (utilisé pour découper la texture).
 */
void layout_item(bool selected, Texture2D texture, Rectangle rect, Rectangle source);

/**
 * @brief Fonction pour créer un élément de mise en page individuel.
 *
 * @param x Position en X du coin supérieur gauche du rectangle.
 * @param y Position en Y du coin supérieur gauche du rectangle.
 * @param w Largeur du rectangle.
 * @param h Hauteur du rectangle.
 * @return Un rectangle représentant l'élément de mise en page.
 */
Rectangle layout_make_rec(float x, float y, float w, float h);

/**
 * @brief Fonction pour créer une mise en page (layout).
 *
 * @param orient Orientation de la mise en page (LO_HORI ou LO_VERT).
 * @param rect Rectangle délimitant la mise en page.
 * @param count Nombre d'éléments dans la mise en page.
 * @param gap Espacement entre les éléments de la mise en page.
 * @return Une structure Layout représentant la mise en page.
 */
Layout layout_make(Layout_Orient orient, Rectangle rect, size_t count, float gap);

/**
 * @brief Fonction pour empiler une mise en page sur le dessus de la pile.
 *
 * @param ls Pointeur vers le tableau de mises en page.
 * @param orient Orientation de la mise en page (LO_HORI ou LO_VERT).
 * @param rect Rectangle délimitant la mise en page.
 * @param count Nombre d'éléments dans la mise en page.
 * @param gap Espacement entre les éléments de la mise en page.
 */
void layout_stack_push(Layout **ls, Layout_Orient orient, Rectangle rect, size_t count, float gap);

/**
 * @brief Fonction pour dépiler la mise en page du dessus de la pile.
 *
 * @param ls Pointeur vers le tableau de mises en page.
 */
void layout_stack_pop(Layout **ls);

/**
 * @brief Fonction pour obtenir le rectangle de la mise en page actuelle sur le dessus de la pile.
 *
 * @param ls Pointeur vers le tableau de mises en page.
 * @param file_path Chemin du fichier source pour le débogage.
 * @param line Numéro de ligne du fichier source pour le débogage.
 * @return Rectangle de la mise en page actuelle.
 */
Rectangle layout_stack_slot_loc(Layout **ls, const char *file_path, int line);

#endif // UI_H_
