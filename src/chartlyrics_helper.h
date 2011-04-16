#include <stdlib.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#ifndef HELPERS_H
#define HELPERS_H

#define CCA	(const char *[])

/* Inspired by libcurl example code */
typedef struct {
  char *page;
  unsigned int size;
}WebData;

/* Nodes can be an element or an attribute */
typedef struct _XMLNode XMLNode;

struct _XMLNode {
	char *name;
	char *content;
	XMLNode *attributes;	// List of attributes
	XMLNode *children;	// List of child elements
	XMLNode *next;		// List of peer elements
};


int strisspace(const char *string);
char *unescape_HTML(char *original);

size_t write_cb(void *ptr, size_t size, size_t nmemb, void *data);

WebData *chartlyrics_helper_post_page(CURL *curl, void *cb_data, const char *url,const char *args);
WebData *chartlyrics_helper_get_page(CURL *curl,const char *url);
int	chartlyrics_helper_free_page(WebData *wpage);
int	chartlyrics_helper_get_xml_element_content(const WebData *xml,
        	const char *element, const char *property, const char *value,
		char **buffer);
int chartlyrics_helper_get_all_xml_element_content(const WebData *xml,
	const char **element_path,const char *element, 
	const char *property, const char *value, char ***buffer);

XMLNode *xmlnode_get(XMLNode *root, const char **path, const char *name, const char *value);
void xmlnode_free(XMLNode *node);
XMLNode *tinycxml_parse(char *xml);


#endif
