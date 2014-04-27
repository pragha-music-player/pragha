/*
 * This File is part of libclastfm 0.5 (http://liblastfm.sourceforge.net/)
 * Licenced as GNU General Public License version 3
 * http://sourceforge.net/p/liblastfm/code/ci/master/tree/src/lfm_helper.c
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <glib.h>

#include "xml_helper.h"

const char *HTML_ESCAPE[] = {
	"&amp;", "&",
	"&quot;","\"",
	"&ndash;","-",
	NULL
};

/* XML Parser's Element name and content buffer */
#define BUFFER_SIZE	1024*1024

char *
unescape_HTML(const char *original) {
	char *cptr, *dup;
	int i;

	if (original == NULL)
		return NULL;

	dup = strdup (original);
	for( i = 0 ; HTML_ESCAPE[i]; i += 2) {
		cptr = strstr(dup, HTML_ESCAPE[i]);
		while (cptr) {
			// This may not work on older systems
			sprintf (cptr, "%s%s", HTML_ESCAPE[i+1], cptr+strlen(HTML_ESCAPE[i]));
			cptr = strstr (dup, HTML_ESCAPE[i]);
		}
	}
	return dup;
}

/* Returns 0 if not entirely white space */
int strisspace(const char *string){
	char *i;
	if(string == NULL)return 1;
	for(i=(char *)string;*i;i++){
		if(!isspace(*i))return 0;
	}
	return 1;
}

static void xmlnode_add(XMLNode **head,XMLNode *node){
	XMLNode *walk = NULL;
	XMLNode *walk_p = NULL;

	if(*head == NULL){
		*head = node;
	}else{
		for(walk = *head ; walk ; walk = walk->next){
			walk_p = walk;
		}
		walk_p->next  = node;
	}
}

static XMLNode *xmlnode_new(char *name){
	XMLNode *node;
	node = malloc(sizeof(XMLNode));
	node->name       = strdup(name);
	node->content    = NULL;
	node->attributes = NULL;
	node->children   = NULL;
	node->next       = NULL;
	return node;
}

static char * _tinycxml_parse(char *xml,XMLNode *parent, char *buffer){
	char *c = NULL;
	char *n = NULL;
	char *p = NULL;
	XMLNode *node = NULL;
	
	if(xml == NULL) return NULL;
	if(*xml == 0)   return xml;
	c = strstr(xml,"<");
	if(c == NULL)   return NULL;
	c++;
	if(*c == '/' )  return c;

	p = buffer;
	for(;*c && *c != '>' && *c != ' '; c++){
		*(p++) = *c;
	}
	*p = 0;

	node = xmlnode_new(buffer);
	
	xmlnode_add(&parent->children,node);
	
	XMLNode *attrib;
	if(*c == ' '){ // Attributes
		p = buffer;
		n = c;
		c = strstr(c,">");
		for(n++;n<c;n++){
			if(*n == '='){
				*p = 0;
				attrib = xmlnode_new(buffer);
				xmlnode_add(&node->attributes,attrib);
				p = buffer;
				// n+=2 // Get past the opening " char
				for(n+=2;n<c && *n!='"';n++){
					*(p++) = *n;
				}
				*p = 0;
				attrib->content = strdup(buffer);
				p = buffer;
			}else if (*n != ' '){
				*(p++) = *n;	
			}
		}
		// Self closing tag?
		if( *(c-1) == '/' ) return c;
	}
	c++;

	/* There is a lot of XML out there that invalidates
	 * XML spec by using '<' and friends inside a CDATA tag.
	 * Although according to the spec we should just error
	 * out, I prefer to mitigate errors in well known conditions. */

	/* Raw copy in the CDATA tag */
	/* Assume CDATA is the only content */
	if(strncmp(c,"<![CDATA[",9) == 0){
		n = strstr(c,"]]>");
		n += 3;
		node->content = g_strndup(c, n-c);
		n = strstr(n,">");
		return (n) ?  n+1 : NULL;
	}

	while(1){
		n = _tinycxml_parse(c,node,buffer);
		if (n == NULL ) break;
		if(*n == '/' ) {
			memcpy(buffer,c,n-c-1);
			buffer[n-c-1] = 0;
			if(!strisspace(buffer)){
				node->content = strdup(buffer);
			}
			n = strstr(n,">");
			return (n) ?  n+1 : NULL;
		}
		if(*n == 0) break;
		c = n;
	}
	return NULL;
}

XMLNode *tinycxml_parse(char *xml){
	char *c = NULL;
	char *buffer = NULL;
	XMLNode node;

	if(xml == NULL) return NULL;

	node.children = NULL;

	c = strstr(xml,"<?xml");
	if(c){
		c = strstr(xml+5,"?>") + 2;
	}
	if (c == NULL) c = xml;

	buffer = malloc(sizeof(char) * BUFFER_SIZE);
	_tinycxml_parse(c,&node,buffer);
	free(buffer);
	return node.children;
}

void xmlnode_free(XMLNode *node){
	if(node == NULL) return;
	if(node->name) free(node->name);
	if(node->content) free(node->content);
	xmlnode_free(node->attributes);
	xmlnode_free(node->children);
	xmlnode_free(node->next);
	free(node);
}

void tinycxml_dump(XMLNode *root){
	if(root == NULL) return;
	printf("[%p] name = %s, content = %s\n",
		root,root->name,root->content);
	tinycxml_dump(root->attributes);
	tinycxml_dump(root->children);
	tinycxml_dump(root->next);
}

static XMLNode *_xmlnode_get_wcontent(XMLNode *root, const char *name, const char *value){
	XMLNode *i,*j;
	for(i = root; i ; i= i->next){
		for(j = i->attributes ; j ; j=j->next){
			if( strcmp(j->name,name)     == 0) {
				if(value == NULL) return j;
				if(strcmp(j->content,value) == 0)return i;
			}
		}
	}
	return NULL;
}

static XMLNode *_xmlnode_get(XMLNode *root, const char **path, int pos){
	XMLNode *walk;
	if ( root == NULL) return NULL;
	for(walk = root ; walk ; walk=walk->next){
		if(strcmp(walk->name,path[pos])== 0) {
			if(path[pos + 1] == NULL) return walk;
			return _xmlnode_get(walk->children,path,pos+1);	
		}
	}
	return NULL;
}

XMLNode *xmlnode_get(XMLNode *root, const char **path, const char *name, const char *value){
	XMLNode *node;

	node = _xmlnode_get(root,path,0);

	if(name){
		return _xmlnode_get_wcontent(node,name,value);
	}else{
		return node;
	}
	
}
