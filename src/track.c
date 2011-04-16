/*
   track.c - Implements the Track API
   See README for Copyright and License
*/

#define _GNU_SOURCE
#include <stdio.h>
#include "lfm_helper.h"
#include "lastfm.h"
#include <string.h>

extern int progress_callback(void *data, 
	double dltotal, double dlnow, 
	double ultotal, double ulnow);


int LASTFM_free_track_info(LASTFM_TRACK_INFO *a){
	if(!a)return 1;
	if(a->name)free(a->name);
	if(a->artist)free(a->artist);
	free(a);
	return 0;
}

int LASTFM_free_track_info_array(LASTFM_TRACK_INFO **a){
	int i;
	if(!a)return 1;
	for(i=0;a[i];i++){
		LASTFM_free_track_info(a[i]);
	}
	free (a);
	return 0;
}

void LASTFM_print_track_info(FILE *out, LASTFM_TRACK_INFO *a){
	if(a == NULL) return;
	fprintf(out,"name        = %s\n",a->name);
	fprintf(out,"artist      = %s\n",a->artist);
	fprintf(out,"playcount   = %i\n",a->playcount);
}

int LASTFM_track_update_now_playing(LASTFM_SESSION *s,
		char *title,char *album, char *artist,
		unsigned int length, unsigned short trackno, 
		unsigned int mbtrack_id){
	WebData *wpage=NULL;
	char api_sig[MD5_BUFFER];
	char *q_artist,*q_album,*q_title;
	int rv;


	if(s == NULL) return LASTFM_STATUS_INVALID;

	if( !title || !artist || strisspace(title) ||  strisspace(artist) ){
		sprintf(s->status,"Failed: Title and Artist fields are mandatory");
		return LASTFM_STATUS_INVALID;
	}

	q_artist =  curl_easy_escape(s->curl,artist,0);
	q_album =  curl_easy_escape(s->curl,album,0);
	q_title =  curl_easy_escape(s->curl,title,0);

	/* Do not escape params yet */
	char *temp = NULL;
	int temp_size;
	temp_size = asprintf(&temp,
"album%sapi_key%sartist%sduration%u\
method%ssk%strack%strackNumber%u%s",
		album,
		s->api_key,
		artist,
		length,		
		"track.updatenowplaying",
		s->session_key,
		title,
		trackno,
		s->secret);
	
	rv = string2MD5(temp,api_sig);
	free(temp);

	temp_size = asprintf(&temp,
"album=%s&api_key=%s&api_sig=%s&artist=%s\
&duration=%u\
&method=track.updatenowplaying&track=%s\
&trackNumber=%u\
&sk=%s",
	q_album,s->api_key,api_sig,q_artist,
	length,
	q_title,
	trackno,
	s->session_key);

	curl_free(q_artist);
	curl_free(q_album);
	curl_free(q_title);

	/* POST */
	wpage = lfm_helper_post_page(s->curl,progress_callback,s,
		API_ROOT,temp);
	free(temp);
	s->fraction = -1;
		
	if((wpage == NULL) || (wpage->page == NULL))return LASTFM_STATUS_ERROR;

	char *status = NULL;
	rv = lfm_helper_get_xml_element_content(wpage,
		"lfm","status",NULL,&status);
	strcpy(s->status,status);
	free(status);

	s->fraction = -1;
	lfm_helper_free_page(wpage);

	if(strcmp(s->status,"ok")){
		return LASTFM_STATUS_ERROR;
	}

	return LASTFM_STATUS_OK;
}

int LASTFM_track_scrobble(LASTFM_SESSION *s,
		char *title, char *album, char *artist,
		time_t start_time,unsigned int length, unsigned int trackno,
		unsigned int mbtrack_id){
	WebData *wpage=NULL;
	char api_sig[MD5_BUFFER];
	char *q_artist,*q_album,*q_title;
	int rv;


	if(s == NULL) return LASTFM_STATUS_INVALID;

	if( !start_time || strisspace(title) ||  strisspace(artist) ){
		sprintf(s->status,"Failed: Title and Artist fields are mandatory");
		return LASTFM_STATUS_INVALID;
	}

	q_artist =  curl_easy_escape(s->curl,artist,0);
	q_album =  curl_easy_escape(s->curl,album,0);
	q_title =  curl_easy_escape(s->curl,title,0);

	char *temp = NULL;
	int temp_size;
	temp_size = asprintf(&temp,
"album%sapi_key%sartist%sduration%u\
method%ssk%stimestamp%lutrack%strackNumber%u%s",
		album,
		s->api_key,
		artist,
		length,		
		"track.scrobble",
		s->session_key,
		start_time,
		title,
		trackno,
		s->secret);
	
	rv = string2MD5(temp,api_sig);
	free(temp);

	temp_size = asprintf(&temp,
"album=%s&api_key=%s&api_sig=%s&artist=%s\
&duration=%u\
&method=track.scrobble&timestamp=%lu&track=%s\
&trackNumber=%u\
&sk=%s",
	q_album,s->api_key,api_sig,q_artist,
	length,
	start_time,q_title,
	trackno,
	s->session_key);

	curl_free(q_artist);
	curl_free(q_album);
	curl_free(q_title);

	/* POST */
	wpage = lfm_helper_post_page(s->curl,progress_callback,s,
		API_ROOT,temp);
	free(temp);
	s->fraction = -1;
		
	if((wpage == NULL) || (wpage->page == NULL))return LASTFM_STATUS_ERROR;

	char *status = NULL;
	rv = lfm_helper_get_xml_element_content(wpage,
		"lfm","status",NULL,&status);
	strcpy(s->status,status);
	free(status);

	s->fraction = -1;
	lfm_helper_free_page(wpage);

	if(strcmp(s->status,"ok")){
		return LASTFM_STATUS_ERROR;
	}

	return LASTFM_STATUS_OK;
}

