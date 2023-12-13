#ifndef XML_H_
#define XML_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define return_defer(value) do { result = (value); goto defer; } while(0)

typedef enum {
    TAG_START,
    TAG_INLINE,
} TagType;

typedef struct {
    char *key;
    char *value;
} XMLAttribute;

typedef struct XMLNode XMLNode;
struct XMLNode {
    char *tag;
    char *inner_text;
    XMLNode *parent;
    XMLAttribute *attributes;
    XMLNode **children;
};

typedef struct {
    XMLNode *root;
} XMLDocument;

typedef XMLNode** Array_XMLNode;

typedef XMLAttribute* Array_XMLAttribute;

XMLNode* xml_node_new(XMLNode *parent);

bool xml_load(XMLDocument *doc, const char *file_path);

void xml_node_print(XMLNode *node, int height);

bool xml_doc_write(XMLDocument *doc, const char *file_path, int indent);

void xml_doc_free(XMLDocument *doc);

void xml_add_node(XMLDocument *doc);

XMLDocument xml_doc_init(const char *tagname);

void xml_insert_node(XMLNode *parent, const char *tag, const char *inner_text);

void xml_attrib_add(XMLNode *node, const char *key, const char *value);

XMLNode* xml_node_find_tag(XMLNode *root, const char *tagname);

Array_XMLNode xml_node_find_tags(XMLNode *root, const char *tagname);

char* xml_attrib_get_value(XMLNode *root, const char *key);

char** xml_get_filepaths(const char *path);

#endif // XML_H_
