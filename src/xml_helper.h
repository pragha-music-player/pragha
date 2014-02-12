#include <stdlib.h>

#ifndef HELPERS_H
#define HELPERS_H

#define CCA	(const char *[])

/* Nodes can be an element or an attribute */
typedef struct _XMLNode XMLNode;

struct _XMLNode {
	char *name;
	char *content;
	XMLNode *attributes;	// List of attributes
	XMLNode *children;	// List of child elements
	XMLNode *next;		// List of peer elements
};

char *unescape_HTML(const char *original);
int strisspace(const char *string);

XMLNode *xmlnode_get(XMLNode *root, const char **path, const char *name, const char *value);
void xmlnode_free(XMLNode *node);
XMLNode *tinycxml_parse(char *xml);

#endif
