/*
   artist.c - Implements the Artist API
   See README for Copyright and License
*/

#include "lastfm.h"
#include "lfm_helper.h"
#include <string.h>

extern char addr_buffer[LARGE_BUFFER];
extern int progress_callback(void *data, 
	double dltotal, double dlnow, 
	double ultotal, double ulnow);

int LASTFM_free_artist_info(LASTFM_ARTIST_INFO *a){
	if(!a)return 1;
	if(a->name)free(a->name);
	if(a->image)free(a->image);
	if(a->summary)free(a->summary);
	if(a->similar){
		int i;	
		for(i=0;a->similar[i];i++)free(a->similar[i]);
		free(a->similar);
	}
	free(a);
	return 0;
}

int LASTFM_free_artist_info_array(LASTFM_ARTIST_INFO **a){
	int i;
	if(!a)return 1;
	for(i=0;a[i];i++){
		LASTFM_free_artist_info(a[i]);
	}
	free (a);
	return 0;
}

int LASTFM_free_image_info_array(LASTFM_IMAGE_INFO **a){
	int i;
	if(!a)return 1;
	for(i=0;a[i];i++){
		if(a[i]->title)free(a[i]->title);
		if(a[i]->image)free(a[i]->image);
		free(a[i]);
	}
	free(a);
	return 0;
}

void LASTFM_print_image_info_array(FILE *out, LASTFM_IMAGE_INFO **a){
	int i;
	if(!a)return;
	for(i=0;a[i];i++){
		printf("Image[%02i] Title=\"%s\", Image Size = %zu, Thumbs Up = %i, Thumbs Down = %i\n",
			i,a[i]->title,a[i]->image_size,a[i]->thumbs_up,a[i]->thumbs_down);
	}
}

void LASTFM_print_artist_info(FILE *out, LASTFM_ARTIST_INFO *a){
	int i;
	if(a== NULL)return;
	fprintf(out,"name       = %s\n",a->name);
	fprintf(out,"playcount  = %i\n",a->playcount);
	// Obviously you cant just print out the binary file
	// so just print out its size 
	fprintf(out,"image size = %zu\n",a->image_size);
	fprintf(out,"summary    = %s\n",a->summary);
	fprintf(out,"similar :\n");
	if(a->similar != NULL){
		for(i=0;a->similar[i];i++){
			printf("           %i:%s\n",i,a->similar[i]);
		}
	}

}


LASTFM_IMAGE_INFO **LASTFM_artist_get_images(LASTFM_SESSION *s, const char *artist, int max){
	LASTFM_IMAGE_INFO **array;
	WebData *data;
	WebData *image;
	char *image_url = NULL;
	int total= 0;
	int i,j;
	char *tmp_buffer;
	int array_size;

	if(s == NULL || artist == NULL) return NULL;

	tmp_buffer = curl_easy_escape(s->curl,artist,0);
	snprintf(addr_buffer,LARGE_BUFFER,
		"%s?method=artist.getimages&api_key=%s&artist=%s&limit=1&page=1",
			API_ROOT,s->api_key,tmp_buffer);
	curl_free(tmp_buffer);

	data = lfm_helper_get_page(addr_buffer,progress_callback,s);
	s->fraction = -1;
	if(data->page == NULL || data->size == 0){
		lfm_helper_free_page(data);
		return NULL;
	}

	lfm_helper_get_xml_element_content(data,"lfm","status",
		NULL,&tmp_buffer);
	strncpy(s->status,tmp_buffer,SMALL_BUFFER-1);
	free(tmp_buffer);

	s->status[SMALL_BUFFER-1]=0;
	if(strcmp(s->status,"ok")){
		lfm_helper_free_page(data);
		return NULL;
	}
	
	lfm_helper_get_xml_element_content(data,"images","totalpages",
		NULL,&tmp_buffer);
	total = atoi(tmp_buffer);
	free(tmp_buffer);

	if(total < max){
		array_size = total;
	}else{
		array_size = max;
	}
	
	array = malloc(sizeof(LASTFM_IMAGE_INFO*)* (array_size+1) );

//	printf("data->page =\n%s\n",data->page);
	for(i=0;i<array_size;i++){
		array[i] = malloc(sizeof(LASTFM_IMAGE_INFO));
		array[i+1] = NULL;
		array[i]->image = NULL;
		lfm_helper_get_xml_element_content(data,"title",NULL,
			NULL,&array[i]->title);
	
		for(j=0;j<LASTFM_IMAGE_SIZES_COUNT;j++){
			lfm_helper_get_xml_element_content(data,"size","name",
				LASTFM_IMAGE_SIZES[j],&image_url);
			if(image_url)goto done_image; // we got the biggest image
		}
		printf("ERROR: LASTFM_artist_get_images. Could not find image on page=%i\n",i+1);
		break;
		done_image:
		/* Get Image */
		image = lfm_helper_get_page(image_url,progress_callback,s);
		if(image->size > 1024){
			array[i]->image  = malloc(image->size);
			memcpy(array[i]->image,image->page,image->size);
			array[i]->image_size = image->size;
		}
		free(image_url);
		image_url = NULL;
		lfm_helper_free_page(image);

		lfm_helper_get_xml_element_content(data,"thumbsup",NULL,
			NULL,&tmp_buffer);
		array[i]->thumbs_up = atoi(tmp_buffer);
		free(tmp_buffer);

		lfm_helper_get_xml_element_content(data,"thumbsdown",NULL,
			NULL,&tmp_buffer);
		array[i]->thumbs_down = atoi(tmp_buffer);
		free(tmp_buffer);

		lfm_helper_free_page(data);

		if( (i+1) == array_size) break;
		sprintf(strstr(addr_buffer,"page=")+strlen("page="),"%i",i+2);
		data = lfm_helper_get_page(addr_buffer,progress_callback,s);
		if(data->page == NULL || data->size == 0){
			lfm_helper_free_page(data);
			break; // this shouldnt happen
		}

	}
	return array;
}

LASTFM_ARTIST_INFO *LASTFM_artist_get_info(LASTFM_SESSION *s, const char *artist){
	LASTFM_ARTIST_INFO *a;
	WebData *data;
	WebData *image;
	char *image_url=NULL;
	int i;


	if(s == NULL || artist == NULL) return NULL;

	a = malloc(sizeof(LASTFM_ARTIST_INFO));
	a->name = malloc(strlen(artist)+1);
	strcpy(a->name,artist);

	a->image = NULL;
	a->image_size = 0;
	a->summary = NULL;
	a->similar = NULL;

	/* Need to convert any spaces into %20 */
	char *art_tmp;
	art_tmp = curl_easy_escape(s->curl,artist,0);
	snprintf(addr_buffer,LARGE_BUFFER,
		"%s?method=artist.getinfo&api_key=%s&artist=%s",
			API_ROOT,s->api_key,art_tmp);
	curl_free(art_tmp);

	data = lfm_helper_get_page(addr_buffer,progress_callback,s);
	s->fraction = -1;
	if(data->page == NULL || data->size == 0){
		lfm_helper_free_page(data);
		LASTFM_free_artist_info(a);
		return NULL;
	}


	/* Try get the biggest image possible */	
	for(i=0;i<LASTFM_IMAGE_SIZES_COUNT;i++){
		lfm_helper_get_xml_element_content(data,"image","size",
			LASTFM_IMAGE_SIZES[i],&image_url);
		if(image_url)break; // we got the biggest image
	}
	if(image_url && strlen(image_url)>10){
		/* Get Image */
		image = lfm_helper_get_page(image_url,progress_callback,s);
		if(image->size > 1024){
			a->image  = malloc(image->size);
			memcpy(a->image,image->page,image->size);
			a->image_size = image->size;
		}
		free(image_url);
		lfm_helper_free_page(image);
	}

	/* Get Summary */
	lfm_helper_get_xml_element_content(data,"summary",NULL,
		NULL,&a->summary);
	unescape_HTML(a->summary);

	/* Get the similar artist names */
	char **temp_array=NULL;
	lfm_helper_get_all_xml_element_content(data,
		(const char *[]){"artist","similar","artist",NULL},
		"name",NULL,NULL,&temp_array);
	a->similar = temp_array;
	if(a->similar)
		for(i=0;a->similar[i];i++)unescape_HTML(a->similar[i]);

	s->fraction = -1;
	lfm_helper_free_page(data);
	return a;
}

