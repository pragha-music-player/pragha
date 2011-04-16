/*
   user.c - Implements the User API
   See README for Copyright and License
*/

#define _GNU_SOURCE
#include <stdio.h>
#include "lfm_helper.h"
#include "lastfm.h"
#include <string.h>

extern char addr_buffer[LARGE_BUFFER];
extern int progress_callback(void *data, 
	double dltotal, double dlnow, 
	double ultotal, double ulnow);


int LASTFM_user_shout(LASTFM_SESSION *s,const char *user,const char *msg){
	int rv=0;
	WebData *wpage=NULL;
	char api_sig[MD5_BUFFER];
	char *status=NULL;
	
	if(s==NULL) return LASTFM_STATUS_INVALID;

	/* Build up the api_sig */
	rv = snprintf(addr_buffer,LARGE_BUFFER,
		"api_key%smessage%smethod%ssk%suser%s%s",
		s->api_key,
		msg,
		"user.shout",
		s->session_key,
		user,
		s->secret);
	
	rv = string2MD5(addr_buffer,api_sig);
	
	
	char *temp;
	int temp_size;

	temp_size = asprintf(&temp,
		"&user=%s&message=%s&api_key=%s&api_sig=%s&sk=%s&method=user.shout",
		user,msg,s->api_key,api_sig,s->session_key);
	wpage = lfm_helper_post_page(s->curl,progress_callback,s,API_ROOT,temp);
	free(temp);
	/* POST */
	/*
	wpage = lfm_helper_post_page(s->curl,progress_callback,s,
		API_ROOT,
		(const char *[]){
		"user",user,
		"message",msg,
		"api_key",s->api_key,
		"api_sig",api_sig,
		"sk",s->session_key,
		"method","user.shout"},12);
	*/
	s->fraction = -1;
		
	if(!wpage)return LASTFM_STATUS_ERROR;
	if(!wpage->page) return LASTFM_STATUS_ERROR;

	rv = lfm_helper_get_xml_element_content(wpage,
		"lfm","status",NULL,&status);

	lfm_helper_free_page(wpage);
	
	if(rv)return LASTFM_STATUS_INVALID;

	strncpy(s->status,status,SMALL_BUFFER-1);
	s->status[SMALL_BUFFER-1]=0;
	free(status);


	return LASTFM_STATUS_OK;
}


LASTFM_ALBUM_INFO **LASTFM_user_get_top_albums(LASTFM_SESSION *s,const char *user,
	int period){
	int rv=0;
	WebData *data=NULL;
	LASTFM_ALBUM_INFO **array = NULL;
	int array_size;
	char *tmp_buffer = NULL;
        char **temp_array=NULL;
	int i;

	if(s == NULL){
		snprintf(s->status,SMALL_BUFFER,"Invalid LASTFM_SESSION");	
		return NULL;
	}

	if(period >= LASTFM_PERIOD_COUNT ){
		snprintf(s->status,SMALL_BUFFER,"Invalid period");	
		return NULL;
	}
	if(user == NULL && s->username == NULL){
		snprintf(s->status,SMALL_BUFFER,"No user specified");
		return NULL;
	}

	rv = snprintf(addr_buffer,LARGE_BUFFER,
		"%s?method=user.gettopalbums&api_key=%s&user=%s&period=%s",
			API_ROOT,
			s->api_key,
			(user) ? user : s->username,
			LASTFM_PERIOD_STRINGS[period]);

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

	if(strcmp(s->status,"ok") != 0 ) goto failed;
	/* Get all the album names */
        lfm_helper_get_all_xml_element_content(data,
                (const char *[]){"topalbums","album rank",NULL},
                "name",NULL,NULL,&temp_array);

	if(temp_array == NULL) goto failed;

	for(i=0;temp_array[i];i++);
	array_size = i;

	array = malloc(sizeof(LASTFM_ALBUM_INFO*) * (array_size+1));
	for(i=0;i<array_size;i++){
		array[i] = malloc(sizeof(LASTFM_ALBUM_INFO));
		array[i]->name        = unescape_HTML(temp_array[i]);
		array[i]->artist      = NULL;
		array[i]->summary     = NULL;
		array[i]->releasedate = NULL;
		array[i]->image       = NULL;
		array[i]->image_size  = 0;
	}
	array[i] = NULL;
	free(temp_array);

	/* Get all the artist names */
        lfm_helper_get_all_xml_element_content(data,
                (const char *[]){"topalbums","album rank","artist",NULL},
                "name",NULL,NULL,&temp_array);

	if(temp_array == NULL) goto failed;

	for(i=0;i<array_size;i++){
		array[i]->artist = unescape_HTML(temp_array[i]);
	}
	free(temp_array);

	/* Get all the playcounts */
        lfm_helper_get_all_xml_element_content(data,
                (const char *[]){"topalbums","album rank",NULL},
                "playcount",NULL,NULL,&temp_array);

	if(temp_array == NULL) goto failed;

	for(i=0;i<array_size;i++){
		array[i]->playcount = atoi(temp_array[i]);
		free(temp_array[i]);
	}
	free(temp_array);

	lfm_helper_free_page(data);
	return array;

	failed:
	lfm_helper_free_page(data);
	return NULL;
}
LASTFM_ARTIST_INFO **LASTFM_user_get_top_artists(LASTFM_SESSION *s,const char *user,
	int period){
	int rv=0;
	WebData *data=NULL;
	LASTFM_ARTIST_INFO **array = NULL;
	int array_size;
	char *tmp_buffer = NULL;
        char **temp_array=NULL;
	int i;


	if(s == NULL){
		snprintf(s->status,SMALL_BUFFER,"Invalid LASTFM_SESSION");	
		return NULL;
	}

	if(period >= LASTFM_PERIOD_COUNT ){
		snprintf(s->status,SMALL_BUFFER,"Invalid period");	
		return NULL;
	}
	
	if(user == NULL && s->username == NULL){
		snprintf(s->status,SMALL_BUFFER,"No user specified");
		return NULL;
	}
	
	rv = snprintf(addr_buffer,LARGE_BUFFER,
		"%s?method=user.gettopartists&api_key=%s&user=%s&period=%s",
			API_ROOT,
			s->api_key,
			(user) ? user : s->username,
			LASTFM_PERIOD_STRINGS[period]);

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

	if(strcmp(s->status,"ok") != 0 ) goto failed;
	/* Get all the album names */
        lfm_helper_get_all_xml_element_content(data,
                (const char *[]){"topartists","artist rank",NULL},
                "name",NULL,NULL,&temp_array);

	if(temp_array == NULL) goto failed;

	for(i=0;temp_array[i];i++);
	array_size = i;

	array = malloc(sizeof(LASTFM_ARTIST_INFO*) * (array_size+1));
	for(i=0;i<array_size;i++){
		array[i] = malloc(sizeof(LASTFM_ARTIST_INFO));
		array[i]->name        = unescape_HTML(temp_array[i]);
		array[i]->summary     = NULL;
		array[i]->image       = NULL;
		array[i]->similar     = NULL;
		array[i]->image_size  = 0;
	}
	array[i] = NULL;
	free(temp_array);

	/* Get all the playcounts */
        lfm_helper_get_all_xml_element_content(data,
                (const char *[]){"topartists","artist rank",NULL},
                "playcount",NULL,NULL,&temp_array);

	if(temp_array == NULL) goto failed;

	for(i=0;i<array_size;i++){
		array[i]->playcount = atoi(temp_array[i]);
		free(temp_array[i]);
	}
	free(temp_array);

	lfm_helper_free_page(data);
	return array;

	failed:
	lfm_helper_free_page(data);
	return NULL;
}

LASTFM_TRACK_INFO **LASTFM_user_get_top_tracks(LASTFM_SESSION *s,const char *user,
	int period){
	int rv=0;
	WebData *data=NULL;
	LASTFM_TRACK_INFO **array = NULL;
	int array_size;
	char *tmp_buffer = NULL;
        char **temp_array=NULL;
	int i;

	if(s == NULL){
		snprintf(s->status,SMALL_BUFFER,"Invalid LASTFM_SESSION");	
		return NULL;
	}

	if(period >= LASTFM_PERIOD_COUNT ){
		snprintf(s->status,SMALL_BUFFER,"Invalid period");	
		return NULL;
	}
	
	if(user == NULL && s->username == NULL){
		snprintf(s->status,SMALL_BUFFER,"No user specified");
		return NULL;
	}
	
	rv = snprintf(addr_buffer,LARGE_BUFFER,
		"%s?method=user.gettoptracks&api_key=%s&user=%s&period=%s",
			API_ROOT,
			s->api_key,
			(user) ? user : s->username,
			LASTFM_PERIOD_STRINGS[period]);

	data = lfm_helper_get_page(addr_buffer,progress_callback,s);
	s->fraction = -1;
	if(data->page == NULL || data->size == 0) goto failed;

	lfm_helper_get_xml_element_content(data,"lfm","status",
		NULL,&tmp_buffer);
	strncpy(s->status,tmp_buffer,SMALL_BUFFER-1);
	free(tmp_buffer);
	s->status[SMALL_BUFFER-1]=0;

	if(strcmp(s->status,"ok") != 0 ) goto failed;

	/* Get all the track names */
        lfm_helper_get_all_xml_element_content(data,
                (const char *[]){"toptracks","track rank",NULL},
                "name",NULL,NULL,&temp_array);

	if(temp_array == NULL) goto failed;

	for(i=0;temp_array[i];i++);
	array_size = i;

	array = malloc(sizeof(LASTFM_TRACK_INFO*) * (array_size+1));
	for(i=0;i<array_size;i++){
		array[i] = malloc(sizeof(LASTFM_TRACK_INFO));
		array[i]->name        = unescape_HTML(temp_array[i]);
		array[i]->artist      = NULL;
		array[i]->playcount   = 0;
	}
	array[i] = NULL;
	free(temp_array);

	/* Get all the artist names */
        lfm_helper_get_all_xml_element_content(data,
                (const char *[]){"toptracks","track rank","artist",NULL},
                "name",NULL,NULL,&temp_array);

	if(temp_array == NULL) goto failed;

	for(i=0;i<array_size;i++){
		array[i]->artist = unescape_HTML(temp_array[i]);
	}
	free(temp_array);

	/* Get all the playcounts */
        lfm_helper_get_all_xml_element_content(data,
                (const char *[]){"toptracks","track rank",NULL},
                "playcount",NULL,NULL,&temp_array);

	if(temp_array == NULL) goto failed;

	for(i=0;i<array_size;i++){
		array[i]->playcount = atoi(temp_array[i]);
		free(temp_array[i]);
	}
	free(temp_array);

	lfm_helper_free_page(data);
	return array;

	failed:
	lfm_helper_free_page(data);
	return NULL;
}
