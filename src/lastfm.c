/*
   lastfm.c - Main library initialisation
   See README for Copyright and License
*/

#include "lfm_helper.h"
#include <string.h>
#include <stdlib.h>
#include "lastfm.h"
#include <stdio.h>

/* Global address buffer variable */
char addr_buffer[LARGE_BUFFER];

int progress_callback(void *data, double dltotal, double dlnow, double ultotal, double ulnow){
	LASTFM_SESSION *s = data;
	double fraction=0;
//	printf("progress: dl=%.0f/%.0f ul=%.0f/%.0f\n",dlnow,dltotal,ulnow,ultotal);
	if( dltotal && !ultotal){
		fraction = dlnow/dltotal;		
	}else if( ultotal && !dltotal){
		fraction = ulnow/ultotal;
	}
	s->fraction = fraction; 
	return 0;
}
LASTFM_SESSION *LASTFM_init( const char *api_key,const char *secret)
{
	LASTFM_SESSION *session;

	session = malloc(sizeof(LASTFM_SESSION));

	strncpy(session->api_key,api_key,MD5_BUFFER-1);
	session->api_key[MD5_BUFFER-1]=0;

	strncpy(session->secret,secret,MD5_BUFFER-1);
	session->secret[MD5_BUFFER-1]=0;

	session->curl = NULL;
	session->fraction = -1;
	session->status[0]=0;
	session->auth_token[0]=0;
	session->session_key[0]=0;
	session->session_id[0]=0;
	session->username=NULL;
	session->playing_url=NULL;
	session->submission_url=NULL;

	curl_global_init(CURL_GLOBAL_ALL);

	/* Setup a CURL handle */
	session->curl = curl_easy_init();
        curl_easy_setopt(session->curl,CURLOPT_COOKIEFILE," ");
        curl_easy_setopt(session->curl,CURLOPT_POST , 1 ) ;
	// Allow redirection
        curl_easy_setopt(session->curl,CURLOPT_FOLLOWLOCATION, 1); 
	curl_easy_setopt(session->curl,CURLOPT_WRITEFUNCTION, 
		WriteWebDataCallback);
	
	/* progress bar */
	curl_easy_setopt(session->curl,CURLOPT_NOPROGRESS,0);
	curl_easy_setopt(session->curl,CURLOPT_PROGRESSFUNCTION,progress_callback);
	curl_easy_setopt(session->curl,CURLOPT_PROGRESSDATA,session);

	return session;
}

int LASTFM_login(LASTFM_SESSION *s, const char *user, const char *pass){
	char pass_hash[MD5_BUFFER];
	int rv=0;
	rv = string2MD5(pass,pass_hash);

	return LASTFM_login_MD5(s,user,pass_hash);
}
int LASTFM_login_MD5(LASTFM_SESSION *s,const char *user,const char *pass_hash){
	WebData *wpage = NULL;
	int rv =0;
	char api_sig[MD5_BUFFER];
	char *sk=NULL;
	char *status=NULL;

	/* Store the password  */
	strcpy(s->password,pass_hash);

	/* Build an authToken  */
	rv = snprintf(addr_buffer,LARGE_BUFFER,"%s%s",user,pass_hash);
	rv = string2MD5(addr_buffer,s->auth_token);

	/* Store the username */
	s->username=malloc(strlen(user)+1);
	strcpy(s->username,user);
	
	/* Build an api_sig */
	rv = snprintf(addr_buffer,LARGE_BUFFER,
		"api_key%sauthToken%smethod%susername%s%s",
		s->api_key,
		s->auth_token,
		"auth.getmobilesession",
		s->username,
		s->secret);

	rv = string2MD5(addr_buffer,api_sig);
		
	/* Build the final url */
	snprintf(addr_buffer,LARGE_BUFFER,
		"%s?method=%s&username=%s&authToken=%s&api_key=%s&api_sig=%s",
			API_ROOT,
			"auth.getmobilesession",
			s->username,
			s->auth_token,
			s->api_key,
			api_sig);

	/* Fetch the webpage */
	wpage = lfm_helper_get_page(addr_buffer,progress_callback,s);	
	s->fraction =-1;

	if(!wpage)return LASTFM_STATUS_ERROR;
	if(!wpage->page)return LASTFM_STATUS_ERROR;

	rv = lfm_helper_get_xml_element_content(wpage,"lfm","status",
		NULL,&status);

	if(rv)goto out;
	strncpy(s->status,status,SMALL_BUFFER-1);
	s->status[SMALL_BUFFER-1]=0;
	free(status);

	/* Get the key */
	rv = lfm_helper_get_xml_element_content(wpage,"key",NULL,
                NULL,&sk);

	if(rv)goto out;
	strncpy(s->session_key,sk,32);
	s->session_key[MD5_BUFFER-1]=0;
	free(sk);

	out:
	
	lfm_helper_free_page(wpage);
	return rv;
}

int LASTFM_dinit(LASTFM_SESSION *s){
	if(s == NULL)return 1;

	if(s->username)free(s->username);
	if(s->submission_url)free(s->submission_url);
	if(s->playing_url)free(s->playing_url);
	if(s->curl)curl_easy_cleanup(s->curl);
	free(s);
	return 0;
}

void LASTFM_print_session(FILE *out, LASTFM_SESSION *s){
	fprintf(out,"status = %s\n",s->status);
	fprintf(out,"username = %s\n",s->username);
	fprintf(out,"api_key = %s\n",s->api_key);
	fprintf(out,"secret = %s\n",s->secret);
	fprintf(out,"auth_token = %s\n",s->auth_token);
	fprintf(out,"sk = %s\n",s->session_key);
	fprintf(out,"session_id = %s\n",s->session_id);
	fprintf(out,"Now-Playing URL = %s\n",s->playing_url);
	fprintf(out,"Submission URL = %s\n",s->submission_url);
}

