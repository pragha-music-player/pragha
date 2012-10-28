/************************************************************************
 * Copyright (C) 2012 matias <mati86dl@gmail.com>                       *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    * 
 * along with this program.  If not, see <http:www.gnu.org/licenses/>.  * 
 ************************************************************************/

#include "pragha.h"

#include <curl/curl.h>
#include <curl/easy.h>

typedef struct {
	char *page;
	unsigned int size;
} WebData;

size_t write_cb(void *ptr, size_t size, size_t nmemb, void *data);
WebData *tunein_helper_get_page(CURL *curl, const char *url);
int tunein_helper_free_page(WebData *wpage);

#define LARGE_BUFFER	1024
#define TUNEIN_API_ROOT	"http://opml.radiotime.com/"

/* TuneIn responce example:
<opml version="1">
	<head>
		<title>Buenos Aires</title>
		<status>200</status>
	</head>
	<body>
		<outline
		 	type="audio"
		 	text="104.5 FM | (Variedades)"
			URL="http://opml.radiotime.com/Tune.ashx?id=s123141"
			bitrate="32" reliability="22"
			guide_id="s123141"
			subtext="Español"
			genre_id="g88"
			formats="wma"
			item="station"
			image="http://d1i6vahw24eb07.cloudfront.net/s123141q.png"
			now_playing_id="s123141"
			preset_id="s123141"
		/>
	</body>
</opml>
*/


void
tunein_helper_print_atribute(XMLNode *xml, const gchar *atribute)
{
	XMLNode *xi;

	xi = xmlnode_get(xml,CCA {"outline", NULL}, atribute, NULL);

	if(xi)
		g_print("%s: %s\n", atribute, xi->content);
}

void
tunein_print_local_radios()
{
	char *buffer;
	WebData *data = NULL;
	XMLNode *xml = NULL, *xi;
	CURL *curl;

	curl_global_init(CURL_GLOBAL_ALL);

	/* Setup a CURL handle */
	curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, " ");

	// Allow redirection
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); 

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);

	buffer = malloc(LARGE_BUFFER);

	snprintf(buffer, LARGE_BUFFER,
		"%s%s",
		TUNEIN_API_ROOT, "Browse.ashx?c=local");

	data = tunein_helper_get_page(curl, buffer);
	free(buffer);

	/* Done with CURL */
	curl_global_cleanup();

	if(data == NULL || data->size == 0)
		return;

	xml = tinycxml_parse(data->page);

	xi = xmlnode_get(xml, CCA{"opml", "body", "outline", NULL }, NULL, NULL);
	for(;xi;xi=xi->next){
		tunein_helper_print_atribute(xi, "text");
	}

	xmlnode_free(xml);
	tunein_helper_free_page(data);
}

static void *myrealloc(void *ptr, unsigned int size)
{
  /* There might be a realloc() out there that doesn't like reallocing
     NULL pointers, so we take care of it here */
  if(ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}

size_t write_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	WebData *mem = data;
	char *page = NULL;

	/* Realloc the existing size + the new data size + 1 for the null terminator */
	page = myrealloc(mem->page, mem->size + realsize + 1);
	if (page) {
		mem->page = page;
		memcpy(mem->page+mem->size, ptr, realsize);
		mem->size += realsize;
		mem->page[mem->size] = 0;
		return realsize;
	}
	else {
		perror("write_cb: Could not realloc");
		return 0;
	}
}

int tunein_helper_free_page(WebData *wpage)
{
	if(wpage == NULL)
		return 1;
	if(wpage->page)
		free(wpage->page);

	free(wpage);

	return 0;
}
	
WebData *tunein_helper_get_page(CURL *curl, const char *url)
{
	WebData *chunk = NULL;
	
	if(url == NULL) return NULL;

	chunk = malloc(sizeof(WebData));

	chunk->page	= NULL;
	chunk->size	= 0;

	/* specify URL to get */
	curl_easy_setopt(curl, CURLOPT_URL, url);

	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)chunk);

	/* get it! */
	curl_easy_perform(curl);

	/* cleanup curl stuff */
	curl_easy_cleanup(curl);

	if(chunk->size == 0) {
		if(chunk->page) {
			free(chunk->page);
			chunk->page = NULL;
		}
	}

	return chunk;
}