/*
   album.c - Implements the Album API
   See README for Copyright and License
*/

#include "lfm_helper.h"
#include "lastfm.h"
#include <string.h>

extern char addr_buffer[LARGE_BUFFER];

extern int progress_callback(void *data, 
	double dltotal, double dlnow, 
	double ultotal, double ulnow);

int LASTFM_free_album_info_array(LASTFM_ALBUM_INFO **a){
	int i;
	if(!a)return 1;
	for(i=0;a[i];i++){
		LASTFM_free_album_info(a[i]);
	}
	free (a);
	return 0;
}

int LASTFM_free_album_info(LASTFM_ALBUM_INFO *a){
	if(!a)return 1;
	if(a->name)free(a->name);
	if(a->artist)free(a->artist);
	if(a->releasedate)free(a->releasedate);
	if(a->image)free(a->image);
	if(a->summary)free(a->summary);
	free(a);
	return 0;
}

void LASTFM_print_album_info(FILE *out, LASTFM_ALBUM_INFO *a){
//	if(a == NULL) return;
	fprintf(out,"name        = %s\n",a->name);
	fprintf(out,"artist      = %s\n",a->artist);
	fprintf(out,"playcount   = %i\n",a->playcount);
	fprintf(out,"releasedate = %s\n",a->releasedate);
	// Obviously you cant just print out the binary file
	// so just print out its size 
	fprintf(out,"image size  = %zu\n",a->image_size);
	fprintf(out,"summary     = %s\n",a->summary);

	FILE *tmp = fopen("image.out","w+");
	fwrite(a->image,a->image_size,1,tmp);
	fclose(tmp);
}

LASTFM_ALBUM_INFO **LASTFM_album_search(LASTFM_SESSION *s,
	unsigned short images, unsigned short limit, const char *album){
	char *alb_tmp;
	WebData *data,*image;
	LASTFM_ALBUM_INFO **a;
	char *s_results=NULL;
	int results=0;
	int rv;
	int i;

	if(s == NULL || album == NULL) return NULL;

	alb_tmp = curl_easy_escape(s->curl,album,0);

	/* 30 is the maximum according to lastfm docs */
	if( (limit > 30) || (limit == 0) ) limit = 30;

	snprintf(addr_buffer,LARGE_BUFFER,
		"%s?method=album.search&"
		"api_key=%s&"
		"album=%s&"
		"limit=%hu",
		API_ROOT,s->api_key,alb_tmp,limit);

	curl_free(alb_tmp);

	data = lfm_helper_get_page(addr_buffer,progress_callback,s);
	
	rv = lfm_helper_get_xml_element_content(data,"opensearch:totalResults",NULL,
		NULL,&s_results);
	
	if(rv) return NULL;

	results = atoi(s_results);
	free(s_results);
		
	/* sanity test */
	if(results == 0) return NULL;
	if(results > limit)results = limit;

	a = malloc(sizeof(LASTFM_ALBUM_INFO*) *(results +1) );
	a[results]=NULL;

	 /* Get all the album names */
        char **temp_array=NULL;
        lfm_helper_get_all_xml_element_content(data,
                (const char *[]){"results","albummatches","album",NULL},
                "name",NULL,NULL,&temp_array);

	for(i=0;temp_array[i];i++){
		a[i] = malloc(sizeof(LASTFM_ALBUM_INFO));

		a[i]->name	= unescape_HTML(temp_array[i]);
		a[i]->artist	= NULL;
		a[i]->releasedate = NULL;
		a[i]->playcount = 0;
		a[i]->image	= NULL;
		a[i]->summary	= NULL;
		a[i]->image_size = 0;
	}
	free(temp_array);

	if(i != results){
		printf("ERROR:LASTFM_album_search: itemsPerPage = %i, names = %i\n",
			results,i);
		results = i;
	}
	/* Get all the artist names */
        lfm_helper_get_all_xml_element_content(data,
                (const char *[]){"results","albummatches","album",NULL},
                "artist",NULL,NULL,&temp_array);

	for(i=0;temp_array[i] && i < results ;i++){
		a[i]->artist = unescape_HTML(temp_array[i]);
	}
	free(temp_array);
	
	if(images == 0)goto done;
	/* If the user asks for 30 results, each image a 60kb, 
	 * this could download 1.8mb! */

	temp_array = NULL;
	lfm_helper_get_all_xml_element_content(data,
                (const char *[]){"results","albummatches","album",NULL},
		"image","size","extralarge",&temp_array);

	for(i=0;temp_array[i];i++);

	if(results != i){
		printf("ERROR:LASTFM_album_search: itemsPerPage = %i, extra large images = %i\n",
			results,i);
	}

	for(i=0;temp_array[i];i++){
		image = lfm_helper_get_page(temp_array[i],progress_callback,s);
		/* Sanity test */
		if(image->size > 1024){
			a[i]->image  = malloc(image->size);
			memcpy(a[i]->image,image->page,image->size);
			a[i]->image_size = image->size;
		}
		free(temp_array[i]);
		lfm_helper_free_page(image);
	}
	free(temp_array);

	done:

	s->fraction = -1;
	lfm_helper_free_page(data);
	return a;
}

LASTFM_ALBUM_INFO *LASTFM_album_get_info(LASTFM_SESSION *s,
	const char *artist, const char *album){
	LASTFM_ALBUM_INFO *a;
	WebData *data;
	WebData *image;
	char *image_url=NULL;
	char *status = NULL;
	int i;
  
	if(s == NULL) return NULL;

	if( !(artist && album) ){
		sprintf(s->status,"Artist and Album fields are mandatory");
		return NULL;
	}

	/* Need to convert any spaces into %20 */
	char *art_tmp,*alb_tmp;
	art_tmp = curl_easy_escape(s->curl,artist,0);
	alb_tmp = curl_easy_escape(s->curl,album,0);
	snprintf(addr_buffer,LARGE_BUFFER,
		"%s?method=album.getinfo&api_key=%s&artist=%s&album=%s",
			API_ROOT,s->api_key,art_tmp,alb_tmp);
	
	curl_free(art_tmp);
	curl_free(alb_tmp);

	data = lfm_helper_get_page(addr_buffer,progress_callback,s);

	lfm_helper_get_xml_element_content(data,"lfm","status",
		NULL,&status);
	if(status){
		strncpy(s->status,status,SMALL_BUFFER-1);
		s->status[SMALL_BUFFER-1]=0;
		free(status);
	}else{
		strcpy(s->status,"No Status");
		return NULL;
	}

	if(strcmp(s->status,"ok"))return NULL;

	a = malloc(sizeof(LASTFM_ALBUM_INFO));

	a->name = malloc(strlen(album)+1);
	strcpy(a->name,album);
	
	a->artist = malloc(strlen(artist)+1);
	strcpy(a->artist,artist);
	
	a->releasedate= NULL;
	a->image = NULL;
	a->image_size = 0;
	a->summary = NULL;

	/* Try get the biggest image possible */	
	for(i=0;i<LASTFM_IMAGE_SIZES_COUNT;i++){
		lfm_helper_get_xml_element_content(data,"image","size",
			LASTFM_IMAGE_SIZES[i],&image_url);
		if(image_url){
			/* Get Image */
			image = lfm_helper_get_page(image_url,progress_callback,s);
			if(image->size > 1024){
				a->image  = malloc(image->size);
				memcpy(a->image,image->page,image->size);
				a->image_size = image->size;
			}
			free(image_url);
			lfm_helper_free_page(image);
			break;
		}
	}
		
	/* Get release date */
	lfm_helper_get_xml_element_content(data,"releasedate",NULL,
		NULL,&a->releasedate);

	/* Get Summary */
	lfm_helper_get_xml_element_content(data,"summary",NULL,
		NULL,&a->summary);
	if(a->summary)
		unescape_HTML(a->summary);

	s->fraction = -1;
	lfm_helper_free_page(data);
	return a;
}
