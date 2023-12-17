#ifndef XML_H_
#define XML_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

/**
 * @def return_defer(value)
 * @brief Macro utilisée pour retourner une valeur et effectuer des actions de nettoyage différé.
 *
 * Cette macro assigne une valeur à la variable result, puis utilise l'instruction goto pour
 * sauter à l'étiquette defer, où des actions de nettoyage peuvent être effectuées avant de quitter
 * la fonction.
 */
#define return_defer(value) do { result = (value); goto defer; } while(0)

/**
 * @enum TagType
 * @brief Énumération représentant le type de balise XML.
 */
typedef enum {
    TAG_START,  /**< Balise de début. */
    TAG_INLINE, /**< Balise en ligne. */
} TagType;

/**
 * @struct XMLAttribute
 * @brief Représente un attribut XML avec une clé et une valeur.
 */
typedef struct {
    char *key;   /**< Clé de l'attribut. */
    char *value; /**< Valeur de l'attribut. */
} XMLAttribute;

/**
 * @struct XMLNode
 * @brief Représente un nœud XML avec un tag, du texte interne, des attributs, et des enfants.
 */
typedef struct XMLNode XMLNode;
struct XMLNode {
    char *tag;                /**< Tag du nœud. */
    char *inner_text;         /**< Texte interne du nœud. */
    XMLNode *parent;          /**< Parent du nœud. */
    XMLAttribute *attributes; /**< Tableau d'attributs du nœud. */
    XMLNode **children;       /**< Tableau des enfants du nœud. */
};

/**
 * @struct XMLDocument
 * @brief Représente un document XML avec un nœud racine.
 */
typedef struct {
    XMLNode *root;  /**< Racine du document XML. */
} XMLDocument;

/**
 * @typedef Array_XMLNode
 * @brief Alias pour un tableau de pointeurs vers des nœuds XML.
 */
typedef XMLNode** Array_XMLNode;

/**
 * @typedef Array_XMLAttribute
 * @brief Alias pour un tableau d'attributs XML.
 */
typedef XMLAttribute* Array_XMLAttribute;

/**
 * @brief Crée un nouveau nœud XML avec un parent donné.
 *
 * @param parent Parent du nouveau nœud.
 * @return Nouveau nœud XML créé.
 */
XMLNode* xml_node_new(XMLNode *parent);

/**
 * @brief Charge un document XML à partir d'un fichier.
 *
 * @param doc Pointeur vers le document XML à charger.
 * @param file_path Chemin du fichier XML à charger.
 * @return `true` si le chargement est réussi, sinon `false`.
 */
bool xml_load(XMLDocument *doc, const char *file_path);

/**
 * @brief Affiche récursivement la structure du nœud XML.
 *
 * @param node Nœud XML à afficher.
 * @param height Niveau de profondeur pour l'indentation.
 */
void xml_node_print(XMLNode *node, int height);

/**
 * @brief Écrit le document XML dans un fichier avec une indentation spécifiée.
 *
 * @param doc Document XML à écrire.
 * @param file_path Chemin du fichier XML de sortie.
 * @param indent Niveau d'indentation.
 * @return `true` si l'écriture est réussie, sinon `false`.
 */
bool xml_doc_write(XMLDocument *doc, const char *file_path, int indent);

/**
 * @brief Libère la mémoire associée à un document XML.
 *
 * @param doc Document XML à libérer.
 */
void xml_doc_free(XMLDocument *doc);

/**
 * @brief Ajoute un nouveau nœud à la racine du document XML.
 *
 * @param doc Document XML auquel ajouter le nœud.
 */
void xml_add_node(XMLDocument *doc);

/**
 * @brief Initialise un document XML avec une balise racine spécifiée.
 *
 * @param tagname Nom de la balise racine.
 * @return Document XML initialisé.
 */
XMLDocument xml_doc_init(const char *tagname);

/**
 * @brief Insère un nouveau nœud sous un parent donné avec un tag et un texte interne spécifiés.
 *
 * @param parent Parent du nouveau nœud.
 * @param tag Balise du nouveau nœud.
 * @param inner_text Texte interne du nouveau nœud.
 */
void xml_insert_node(XMLNode *parent, const char *tag, const char *inner_text);

/**
 * @brief Ajoute un attribut à un nœud XML avec une clé et une valeur spécifiées.
 *
 * @param node Nœud XML auquel ajouter l'attribut.
 * @param key Clé de l'attribut.
 * @param value Valeur de l'attribut.
 */
void xml_attrib_add(XMLNode *node, const char *key, const char *value);

/**
 * @brief Recherche un nœud XML avec un tag spécifié dans l'arborescence du nœud donné.
 *
 * @param root Racine de l'arborescence à rechercher.
 * @param tagname Tag à rechercher.
 * @return Pointeur vers le nœud trouvé, ou NULL s'il n'est pas trouvé.
 */
XMLNode* xml_node_find_tag(XMLNode *root, const char *tagname);

/**
 * @brief Recherche tous les nœuds XML avec un tag spécifié dans l'arborescence du nœud donné.
 *
 * @param root Racine de l'arborescence à rechercher.
 * @param tagname Tag à rechercher.
 * @return Tableau de pointeurs vers les nœuds trouvés.
 */
Array_XMLNode xml_node_find_tags(XMLNode *root, const char *tagname);

/**
 * @brief Obtient la valeur d'un attribut XML spécifié dans l'arborescence du nœud donné.
 *
 * @param root Racine de l'arborescence à rechercher.
 * @param key Clé de l'attribut à obtenir.
 * @return Valeur de l'attribut, ou NULL si l'attribut n'est pas trouvé.
 */
char* xml_attrib_get_value(XMLNode *root, const char *key);

/**
 * @brief Obtient les chemins de fichiers XML dans un répertoire spécifié.
 *
 * @param path Chemin du répertoire contenant les fichiers XML.
 * @return Tableau de chaînes de caractères représentant les chemins des fichiers XML.
 */
char** xml_get_filepaths(const char *path);

#endif // XML_H_
