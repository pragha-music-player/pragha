#include <stdlib.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#ifndef HELPERS_H
#define HELPERS_H
/* Inspired by libcurl example code */
typedef struct {
  char *page;
  unsigned int size;
}WebData;

char *unescape_HTML(char *original);
int string2MD5(const char *string, char *buffer);
int strisspace(const char *string);
void *myrealloc(void *ptr, unsigned int size);
size_t WriteWebDataCallback(void *ptr, size_t size, 
	size_t nmemb, void *data);

WebData *lfm_helper_post_page(CURL *curl, curl_progress_callback cb, void *cb_data, const char *url,const char *args);
WebData *lfm_helper_get_page(const char *url, curl_progress_callback cb, void *cb_data);
int	lfm_helper_free_page(WebData *wpage);
int	lfm_helper_get_xml_element_content(const WebData *xml,
        	const char *element, const char *property, const char *value,
		char **buffer);
int lfm_helper_get_all_xml_element_content(const WebData *xml,
	const char **element_path,const char *element, 
	const char *property, const char *value, char ***buffer);


#endif
