#include "xml.h"
#include <stddef.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#include "array.h"

XMLNode* xml_node_new(XMLNode *parent) {
    XMLNode *node = malloc(sizeof(XMLNode));
    node->tag = NULL;
    node->inner_text = NULL;
    node->parent = parent;
    node->attributes = array_create_init(2, sizeof(XMLAttribute));
    node->children = array_create_init(2, sizeof(XMLNode*));
    if (parent) array_push(parent->children, node);
    return node;
}

static void xml_node_free(XMLNode *node) {
    if (node) {
	for (size_t i = 0; i < array_size(node->children); i++) {
	    xml_node_free(node->children[i]);
	}

	for (size_t i = 0; i < array_size(node->attributes); i++) {
	    if (node->attributes[i].key) free(node->attributes[i].key);
	    if (node->attributes[i].value) free(node->attributes[i].value);
	}

	if (node->tag) free(node->tag);
	if (node->inner_text) free(node->inner_text);
	array_free(node->children);
	array_free(node->attributes);
	free(node);
    }
}

static TagType parse_attrs(XMLNode *curr_node, char *buf, size_t *i, char *lex, size_t *lexi) {
    XMLAttribute curr_attr = {0};
    while (buf[*i] != '>') {
	lex[(*lexi)++] = buf[(*i)++];

	// Tag name
	if (buf[*i] == ' ' && !curr_node->tag) {
	    lex[*lexi] = '\0';
	    curr_node->tag = strdup(lex);
	    *lexi = 0;
	    (*i)++;
	    continue;
	}

	// ignore space
	if (lex[*lexi-1] == ' ') {
	    (*lexi)--;
	}

	// Attribute key
	if (buf[*i] == '=') {
	    lex[*lexi] = '\0';
	    curr_attr.key = strdup(lex);
	    *lexi = 0;
	    continue;
	}

	// Attribute value
	if (buf[*i] == '"') {
	    if (!curr_attr.key) {
		fprintf(stderr, "Value has no key\n");
		return TAG_START;
	    }

	    *lexi = 0;
	    (*i)++;

	    while (buf[*i] != '"') {
		lex[(*lexi)++] = buf[(*i)++];
	    }
	    lex[*lexi] = '\0';
	    curr_attr.value = strdup(lex);
	    //curr_node->attributes
	    array_push(curr_node->attributes, curr_attr);
	    curr_attr.key = NULL;
	    curr_attr.value = NULL;
	    *lexi = 0;
	    (*i)++;
	    continue;
	}

	// Inline node
	if (buf[*i - 1] == '/' && buf[*i] == '>') {
	    lex[*lexi - 1] = '\0';
	    if (!curr_node->tag) curr_node->tag = strdup(lex);
	    (*i)++;
	    return TAG_INLINE;
	}
    }
    return TAG_START;
}

bool xml_load(XMLDocument* doc, const char *file_path) {
    bool result = true;
    char *buf = NULL;
    FILE *file = fopen(file_path, "r");
    if (!file) {
	fprintf(stderr, "ERROR: Could not fopen the file %s: %s\n", file_path, strerror(errno));
	return_defer(false);
    }

    if (fseek(file, 0, SEEK_END) == -1) {
	fprintf(stderr, "ERROR: Could not fseek the file %s to the end: %s", file_path, strerror(errno));
	return_defer(false);
    }

    int size = ftell(file);
    if (size == -1) {
	fprintf(stderr, "ERROR: Could not ftell the file %s: %s", file_path, strerror(errno));
	return_defer(false);
    }

    if (fseek(file, 0, SEEK_SET) == -1) {
	fprintf(stderr, "ERROR: Could not fseek the file %s to the start: %s", file_path, strerror(errno));
	return_defer(false);
    }

    buf = malloc(sizeof(char) * size + 1);
    if (buf == NULL) {
	fprintf(stderr, "ERROR: Could not allocate sufficient memory for reading the source file: %s\n", strerror(errno));
	return_defer(false);
    }

    if (fread(buf, 1, size, file) == 0) {
	fprintf(stderr, "ERROR: Could not fread the file %s: %s\n", file_path, strerror(errno));
	return_defer(false);
    }
    buf[size] = '\0';

    // implementation de lecture
    //doc->root = xml_node_new(NULL); // old version
    doc->root = NULL;
    char lex[1024];
    size_t lexi = 0;
    size_t i = 0;

    //XMLNode *curr_node = doc->root; // old version
    XMLNode *curr_node = NULL;
    while (buf[i] != '\0') {
	if (buf[i] == '<') {
	    lex[lexi] = '\0';

	    // inner text
	    if (lexi > 0) {
		if (!curr_node) {
		    fprintf(stderr, "Text outside of document\n");
		    return_defer(false);
		}

		curr_node->inner_text = strdup(lex);
		lexi = 0;
	    }

	    // end of node (</root>)
	    if (buf[i + 1] == '/') {
		i += 2;
		while (buf[i] != '>') {
		    lex[lexi++] = buf[i++];
		}
		lex[lexi] = '\0';
		if (!curr_node) {
		    fprintf(stderr, "Already at the root\n");
		    return_defer(false);
		}

		if (strcmp(curr_node->tag, lex)) {
		    fprintf(stderr, "Mismatched tags (%s != %s)\n", curr_node->tag, lex);
		    return_defer(false);
		}

		curr_node = curr_node->parent;
		lexi = 0;
		i++;
		continue;
	    }

	    // set current_node
	    //curr_node = xml_node_new(curr_node); // old version
	    if (doc->root == NULL) {
		curr_node = xml_node_new(NULL);
		doc->root = curr_node;
	    } else {
		curr_node = xml_node_new(curr_node);
	    }

	    // set tag name
	    i++;
	    if (parse_attrs(curr_node, buf, &i, lex, &lexi) == TAG_INLINE) {
		curr_node = curr_node->parent;
		lexi = 0;
		i++;
		continue;
	    }

	    lex[lexi] = '\0';
	    if (!curr_node->tag) curr_node->tag = strdup(lex);

	    // Reset lexer
	    lexi = 0;
	    i++;
	} else {
	    if (isspace(buf[i])) {
		i++;
	    } else {
		lex[lexi++] = buf[i++];
	    }
	}
    }

 defer:
    if (file) fclose(file);
    if (buf != NULL) free(buf);
    if (doc != NULL && !result) xml_doc_free(doc);
    return result;
}

void xml_node_print(XMLNode *node, int height) {
    if (node) {
	printf("%*s", 4 * height, " ");
	printf("tag: %s\n", node->tag);

	printf("%*s", 4 * height, " ");
	printf("attributes: ");

	for (size_t i = 0; i < array_size(node->attributes); i++) {
	    printf("(%s = %s) ", node->attributes[i].key, node->attributes[i].value);
	}
	printf("\n");

	printf("%*s", 4 * height, " ");
	printf("inner_text: %s\n\n", node->inner_text);

	for (size_t i = 0; i < array_size(node->children); i++) {
	    xml_node_print(node->children[i], height + 1);
	}
    }
}

static void xml_node_out(FILE *file, XMLNode *node, int indent, int times) {
    for (size_t i = 0; i < array_size(node->children); i++) {
	XMLNode *child = node->children[i];

	if (times > 0) fprintf(file, "%*s", indent * times, " ");

	fprintf(file, "<%s", child->tag);

	for (size_t j = 0; j < array_size(child->attributes); j++) {
	    XMLAttribute attr = child->attributes[j];
	    if (!attr.value || !strcmp(attr.value, "")) {
		continue;
	    }
	    fprintf(file, " %s=\"%s\"", attr.key, attr.value);
	}

	if (array_size(child->children) == 0 && !child->inner_text) {
	    fprintf(file, "/>\n");
	} else {
	    fprintf(file, ">");
	    if (array_size(child->children) == 0) {
		fprintf(file, "\n%*s", indent * (times + 1), " ");
		fprintf(file, "%s", child->inner_text);
		fprintf(file, "\n%*s", indent * times, " ");
		fprintf(file, "</%s>\n", child->tag);
	    } else {
		fprintf(file, "\n");
		xml_node_out(file, child, indent, times + 1);
		if (times > 0) fprintf(file, "%*s", indent * times, " ");
		fprintf(file, "</%s>\n", child->tag);
	    }
	}
    }
}

bool xml_doc_write(XMLDocument *doc, const char *file_path, int indent) {
    bool result = true;
    FILE *file = fopen(file_path, "w");
    if (!file) {
	fprintf(stderr, "ERROR: Could not fopen the file %s: %s\n", file_path, strerror(errno));
	return_defer(false);
    }

    fprintf(file, "<%s", doc->root->tag);
    for (size_t i = 0; i < array_size(doc->root->attributes); i++) {
	XMLAttribute attr = doc->root->attributes[i];
	if (!attr.value || !strcmp (attr.value, "")) {
	    continue;
	}
	fprintf(file, " %s=\"%s\"", attr.key, attr.value);
    }
    fprintf(file, ">\n");
    xml_node_out(file, doc->root, indent, 1);
    fprintf(file, "<%s/>\n", doc->root->tag);

 defer:
    if (file) fclose(file);
    return result;
}

//void xml_doc_print(XMLDocument *doc) {
//    //xml_node_print(doc->root->children[0], 0);
//    xml_node_print(doc->root, 0);
//}
XMLNode* xml_node_find_tag(XMLNode *root, const char *tagname) {
    if (strcmp(root->tag, tagname)) {
	for (size_t i = 0; i < array_size(root->children); i++) {
	    return xml_node_find_tag(root->children[i], tagname);
	}
    }
    return root;
}

Array_XMLNode xml_node_find_tags(XMLNode *root, const char *tagname) {
    Array_XMLNode stack = array_create_init(2, sizeof(XMLNode*));
    Array_XMLNode nodes = array_create_init(2, sizeof(XMLNode*));

    array_push(stack, root);

    while (array_size(stack)) {
	XMLNode *node = array_last(stack);
	array_pop_last(stack);
	if (strcmp(node->tag, tagname) == 0) {
	    array_push(nodes, node);
	}
	for (size_t i = 0; i < array_size(node->children); i++) {
	    array_push(stack, node->children[i]);
	}
    }

    array_free(stack);
    return nodes;
}

void xml_doc_free(XMLDocument *doc) {
    xml_node_free(doc->root);
}

XMLDocument xml_doc_init(const char *tagname) {
    XMLDocument result = {0};
    result.root = xml_node_new(NULL);
    result.root->tag = strdup(tagname);
    return result;
}

void xml_insert_node(XMLNode *parent, const char *tag, const char *inner_text) {
    XMLNode *node = xml_node_new(parent);
    node->tag = strdup(tag);
    if (inner_text != NULL) node->inner_text = strdup(inner_text);
}

void xml_attrib_add(XMLNode *node, const char *key, const char *value) {
    XMLAttribute attr = {0};
    attr.key = strdup(key);
    attr.value = strdup(value);
    array_push(node->attributes, attr);
}

char* xml_attrib_get_value(XMLNode *root, const char *key) {
    for (size_t i = 0; i < array_size(root->attributes); i++) {
	if (strcmp(root->attributes[i].key, key) == 0) {
	    return root->attributes[i].value;
	}
    }
    return NULL;
}

static bool check_xml_ext(const char *filename) {
    char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return false;
    if (strcmp(dot + 1, "xml") == 0) return true;
    else return false;
}

char** xml_get_filepaths(const char *path) {
    char **paths = array_create_init(2, sizeof(char*));
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path)) != NULL) {
	while ((entry = readdir(dir)) != NULL) {
	    if (entry->d_type != DT_DIR && check_xml_ext(entry->d_name)) {
		int size_str = snprintf(NULL, 0, "%s/%s", path, entry->d_name);
		char *full_path = malloc(size_str + 1);
		sprintf(full_path, "%s/%s", path, entry->d_name);
		array_push(paths, full_path);
	    }
	}
    } else {
	if (mkdir(path, 0777) == -1) {
	    fprintf(stderr, "Error: Could not create a directory: %s\n", strerror(errno));
	    exit(1);
	}
	return xml_get_filepaths(path);
    }

    if (dir != NULL) closedir(dir);
    return paths;
}
