/*
   scrobbler.c - Implements the Scrobble API
   See README for Copyright and License
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "lastfm.h"
#include "lfm_helper.h"

extern char addr_buffer[LARGE_BUFFER];
extern int progress_callback(void *data, 
	double dltotal, double dlnow, 
	double ultotal, double ulnow);

int LASTFM_scrobbler_handshake(	LASTFM_SESSION *s,char *client_id, char *client_ver){

	WebData *wpage = NULL;
	int rv =0;
	char auth_token[MD5_BUFFER];
	time_t timestamp;
	char *lineptr = NULL;

	if(s == NULL) return LASTFM_STATUS_INVALID;
 
	/* Get the current epoch time */
	time(&timestamp);

	/* Build an authToken = md5(md5(password) + timestamp) */
	rv = snprintf(addr_buffer,LARGE_BUFFER,"%s%lu",s->password,timestamp);
	rv = string2MD5(addr_buffer,auth_token);

	snprintf(addr_buffer,LARGE_BUFFER,
		"%s?hs=true&p=%s&c=%s&v=%s&u=%s&t=%lu&a=%s",
			SCROBBLER_ROOT,
			SCROBBLER_VERSION,
			client_id,
			client_ver,
			s->username,
			timestamp,
			auth_token
			);
			
	/* Fetch the webpage */
	wpage = lfm_helper_get_page(addr_buffer,progress_callback,s);	
	s->fraction =-1;
	if((wpage == NULL) || (wpage->page == NULL))return LASTFM_STATUS_ERROR;
	

	/* First line is the status */
	lineptr = strtok(wpage->page,"\n");
	strcpy(s->status,lineptr);
	if(strcmp(s->status,"OK")){
		lfm_helper_free_page(wpage);
		return LASTFM_STATUS_ERROR;
	}

	/* Second line is the Session ID */
	lineptr = strtok(NULL,"\n");
	strcpy(s->session_id,lineptr);

	/* 3rd is the Now-Playing URL */
	lineptr = strtok(NULL,"\n");
	s->playing_url = strdup(lineptr);

	/* 4th is the Submission URL */
	lineptr = strtok(NULL,"\n");
	s->submission_url = strdup(lineptr);
	
	lfm_helper_free_page(wpage);
	return LASTFM_STATUS_OK;
} 

int LASTFM_scrobbler_now_playing(LASTFM_SESSION *s,
		char *title,char *album, char *artist,
		unsigned int length, unsigned short trackno, 
		unsigned int mbtrack_id){

	WebData *wpage=NULL;
	char length_buffer[8];
	char trackno_buffer[8];
	char *lineptr=NULL;


	if(s == NULL) return LASTFM_STATUS_INVALID;

	if(strisspace(title) ||  strisspace(artist)){
		sprintf(s->status,"Failed: Title and Artist fields are mandatory");
		return LASTFM_STATUS_INVALID;
	}

	length_buffer[0]=0;
	trackno_buffer[0]=0;
	if(length)snprintf(length_buffer,7,"%i",length);
	if(trackno)snprintf(trackno_buffer,7,"%i",trackno);

	char *temp=NULL;
	int temp_size;
	temp_size = asprintf(&temp,"&s=%s&a=%s&t=%s&b=%s&l=%s&n=%s&m=",
		s->session_id,artist,title,album,length_buffer,trackno_buffer);
	/* POST */
	wpage = lfm_helper_post_page(s->curl,progress_callback,s,
		s->playing_url,temp);
	free(temp);

	s->fraction = -1;
		
	if((wpage == NULL) || (wpage->page == NULL))return LASTFM_STATUS_ERROR;

	/* First line is the status */
	lineptr = strtok(wpage->page,"\n");
	strcpy(s->status,lineptr);
	lfm_helper_free_page(wpage);

	if(strcmp(s->status,"OK")){
		return LASTFM_STATUS_ERROR;
	}
	return LASTFM_STATUS_OK;
}

int LASTFM_scrobbler_submit(LASTFM_SESSION *s,
		char *title, char *album, char *artist,
		time_t start_time,unsigned int length, unsigned int trackno,
		unsigned int mbtrack_id){
	WebData *wpage=NULL;
	char length_buffer[8];
	char trackno_buffer[8];
	char *lineptr=NULL;
	char *q_artist,*q_album,*q_title;


	if(s == NULL) return LASTFM_STATUS_INVALID;

	if( !start_time || strisspace(title) ||  strisspace(artist) ){
		sprintf(s->status,"Failed: Title and Artist fields are mandatory");
		return LASTFM_STATUS_INVALID;
	}

	length_buffer[0]=0;
	trackno_buffer[0]=0;
	if(length)snprintf(length_buffer,7,"%u",length);
	if(trackno)snprintf(trackno_buffer,7,"%u",trackno);

	q_artist =  curl_easy_escape(s->curl,artist,0);
	q_album =  curl_easy_escape(s->curl,album,0);
	q_title =  curl_easy_escape(s->curl,title,0);

	char *temp;
	int temp_size;
	temp_size = asprintf(&temp,
		"&s=%s&a[0]=%s&t[0]=%s&i[0]=%lu&o[0]=P&r[0]=&l[0]=%s\
		&b[0]=%s&n[0]=%s&m[0]=",
		s->session_id,q_artist,q_title,start_time,length_buffer,
		q_album,trackno_buffer);

	curl_free(q_artist);
	curl_free(q_album);
	curl_free(q_title);

	/* POST */
	wpage = lfm_helper_post_page(s->curl,progress_callback,s,
		s->submission_url,temp);
	free(temp);
	s->fraction = -1;
		
	if((wpage == NULL) || (wpage->page == NULL))return LASTFM_STATUS_ERROR;

	/* First line is the status */
	lineptr = strtok(wpage->page,"\n");
	strcpy(s->status,lineptr);
	lfm_helper_free_page(wpage);

	if(strcmp(s->status,"OK")){
		return LASTFM_STATUS_ERROR;
	}
	return LASTFM_STATUS_OK;

}
