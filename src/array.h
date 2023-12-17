#ifndef ARRAY_H_
#define ARRAY_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

/**
 * @brief Structure représentant un tableau dynamique.
 */
typedef struct {
    size_t size;     /**< Nombre d'éléments actuellement dans le tableau. */
    size_t capacity; /**< Capacité totale du tableau. */
} Array;

#define CAST(T, expr) ((T)(expr)) /**< Macro pour effectuer une conversion de type. */

/**
 * @brief Macro pour obtenir les métadonnées du tableau.
 * 
 * Cette macro permet d'obtenir un pointeur vers les métadonnées du tableau,
 * qui sont stockées juste avant le début de la zone allouée pour le tableau.
 */
#define array_meta(array) \
    (CAST(Array*, (array)) - 1)

/**
 * @brief Macro pour tenter d'agrandir le tableau.
 * 
 * Cette macro vérifie si le tableau a assez de capacité pour accueillir
 * une taille supplémentaire spécifiée. Si ce n'est pas le cas, elle appelle
 * la fonction array_grow pour agrandir le tableau.
 */
#define array_try_grow(array, size_) \
    (((array) && array_meta(array)->size + (size_) < array_meta(array)->capacity) \
     ? true \
     : array_grow(CAST(void **, &(array)), (size_), sizeof *(array)))

/**
 * @brief Macro pour obtenir la taille actuelle du tableau.
 */
#define array_size(array) \
    ((array) ? array_meta(array)->size : 0)

/**
 * @brief Macro pour obtenir la capacité actuelle du tableau.
 */
#define array_capacity(array) \
    ((array) ? array_meta(array)->capacity : 0)

/**
 * @brief Macro pour étendre le tableau.
 * 
 * Cette macro tente d'agrandir le tableau de la taille spécifiée.
 */
#define array_expand(array, size_) \
    (array_try_grow((array), (size_)) \
     ? (array_meta(array)->size += (size_), true) \
     : false)

/**
 * @brief Macro pour ajouter un élément à la fin du tableau.
 * 
 * Cette macro tente d'agrandir le tableau de 1, puis ajoute la valeur à la fin.
 */
#define array_push(array, value) \
    (array_try_grow((array), 1) \
     ? ((array)[array_meta(array)->size++] = (value), true) \
     : false)

/**
 * @brief Macro pour supprimer le premier élément du tableau.
 */
#define array_pop_front(array) \
    (array_size(array) ? \
     (memmove((array), &(array)[1], sizeof *(array) * (array_meta(array)->size - 1)), \
      array_meta(array)->size--, true) \
     : false)

/**
 * @brief Macro pour supprimer le dernier élément du tableau.
 */
#define array_pop_last(array) \
    (array_size(array) ? \
     (array_resize((array), array_size(array) - 1), true) \
     : false)

/**
 * @brief Macro pour supprimer l'élément à un index donné du tableau.
 */
#define array_pop_at(array, index) \
    (array_size(array) && (index) < array_size(array) ?	\
     (memmove((array) + (index), &(array)[(index) + 1], sizeof *(array) * (array_meta(array)->size - (index) - 1)), \
      array_meta(array)->size--, true) \
     : false)

/**
 * @brief Macro pour libérer la mémoire associée au tableau.
 */
#define array_free(array) \
    array_delete((void*)array)

/**
 * @brief Macro pour redimensionner le tableau.
 * 
 * Cette macro redimensionne le tableau à la taille spécifiée.
 * Si le tableau est déjà assez grand, il ajuste simplement la taille actuelle.
 * Sinon, il tente d'agrandir le tableau.
 */
#define array_resize(array, size_) \
    ((array) \
     ? (array_meta(array)->size >= (size_) \
	? (array_meta(array)->size = (size_), true) \
	: array_expand((array), (size_) - array_meta(array)->size)) \
     : (array_grow(CAST(void **, &(array)), (size_), sizeof *(array)) \
	? (array_meta(array)->size = (size_), true) \
	: false))

/**
 * @brief Macro pour obtenir le dernier élément du tableau.
 */
#define array_last(array) \
    ((array)[array_size(array) - 1])

/**
 * @brief Macro pour vider le tableau (définir la taille à zéro).
 */
#define array_clear(array) \
    (void)((array) ? array_meta(array)->size = 0 : 0)

bool array_grow(void **const array, size_t elements, size_t type_size);
void array_delete(void **const array);
void *array_create_init(size_t capacity, size_t type_size);

#ifdef ARRAY_IMPLEMENTATION

/**
 * @brief Agrandit la capacité d'un tableau dynamique.
 * 
 * Cette fonction augmente la capacité d'un tableau dynamique afin de pouvoir
 * contenir un certain nombre d'éléments supplémentaires. Elle est utilisée
 * par les macros de gestion de tableau pour redimensionner le tableau au besoin.
 *
 * @param array   Un pointeur vers le tableau à agrandir.
 * @param elements Le nombre d'éléments supplémentaires à prendre en compte.
 * @param type_size La taille en octets d'un élément du tableau.
 * @return Retourne true si l'agrandissement réussit, false en cas d'échec.
 */
bool array_grow(void **const array, size_t elements, size_t type_size) {
    assert(*array);
    Array *meta = array_meta(*array);
    const size_t count = 2 * meta->capacity + elements;
    void *data = realloc(meta, type_size * count + sizeof *meta);
    if (!data) {
	free(meta);
	return false;
    }
    meta = CAST(Array *, data);
    meta->capacity = count;
    *array = meta + 1;
    return true;
}

/**
 * @brief Libère la mémoire associée à un tableau dynamique.
 * 
 * Cette fonction libère la mémoire allouée pour un tableau dynamique,
 * y compris les métadonnées stockées avant le début du tableau.
 *
 * @param array Un pointeur vers le tableau à libérer.
 */
void array_delete(void **const array) {
    assert(array);
    Array *const meta = array_meta(array);
    free(meta);
}

/**
 * @brief Crée et initialise un nouveau tableau dynamique.
 * 
 * Cette fonction alloue la mémoire nécessaire pour un nouveau tableau
 * dynamique et initialise ses métadonnées, puis renvoie un pointeur
 * vers la première position du tableau.
 *
 * @param capacity La capacité initiale du tableau (nombre d'éléments pouvant être stockés).
 * @param type_size La taille en octets d'un élément du tableau.
 * @return Un pointeur vers la première position du tableau nouvellement créé.
 *         En cas d'échec, renvoie NULL.
 */
void *array_create_init(size_t capacity, size_t type_size) {
    void *data = malloc(type_size * capacity + sizeof(Array));
    if (!data) {
	return 0;
    }
    Array *array = CAST(Array *, data);
    array->size = 0;
    array->capacity = capacity;
    return array + 1;
}

#endif // ARRAY_IMPLEMENTATION

#endif // ARRAY_H_
