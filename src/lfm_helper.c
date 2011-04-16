/*
   lfm_helper.c - Misc helper functions
   See README for Copyright and License
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include <openssl/md5.h>

#include "lfm_helper.h"

char *HTML_ESCAPE[] = {
	"&amp;", "&",
	"&quot;","\"",
	"&ndash;","-",
	NULL
};

char *unescape_HTML(char *original){
	int i;
	char *cptr;
	if(original == NULL) return NULL;

	for(i=0;HTML_ESCAPE[i];i+=2){
		cptr = strstr(original,HTML_ESCAPE[i]);
		while(cptr!= NULL){
			// This is pretty naughty
			sprintf(cptr,"%s%s",
				HTML_ESCAPE[i+1],
				cptr+strlen(HTML_ESCAPE[i]));
			cptr = strstr(original,HTML_ESCAPE[i]);
		}
	}
	return original;
}

int string2MD5(const char *string, char *buffer){
	char hash[MD5_DIGEST_LENGTH];
	int i;
	char *cptr;
	MD5((const unsigned char *)string,strlen(string),(unsigned char *)hash);
	cptr = buffer;
	for(i=0;i<MD5_DIGEST_LENGTH;i++){
		sprintf(cptr,"%02x",(unsigned char)hash[i]);
		cptr+=2;
	}
	buffer[(MD5_DIGEST_LENGTH*2)]=0;
	return 0;
}
void *myrealloc(void *ptr, unsigned int size)
{
  /* There might be a realloc() out there that doesn't like reallocing
     NULL pointers, so we take care of it here */
  if(ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}

size_t WriteWebDataCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  unsigned int realsize = size * nmemb;
  WebData *mem = data;

  mem->page = myrealloc(mem->page, mem->size + realsize + 1);
  if (mem->page) {
    memcpy(&(mem->page[mem->size]), ptr, realsize);

    mem->size += realsize;
    mem->page[mem->size] = 0;
  }
  return realsize;
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

int lfm_helper_get_xml_element_content(const WebData *xml,
	const char *element,const char *property, const char *value, char **buffer){

	char *end_element = NULL;
	char *start_element = NULL;
	char *start_property = NULL;
	char *cptr = NULL;
	char *cptr2 = NULL;
	char *cptr3 = NULL;
	char *cptr4 = NULL;
	int length;
	int rv;
	int i;

	if(xml == NULL || xml->page == NULL) return 1;

	rv = asprintf(&end_element,"</%s>",element);
	if(property){
		rv = asprintf(&start_element,"<%s ",element);
		rv = asprintf(&start_property,"%s=\"",property);
	}else{
		rv = asprintf(&start_element,"<%s>",element);
	}

	cptr2 = xml->page;

	next_element:
	cptr = strstr(cptr2,start_element);
	if(cptr == NULL) { rv = 1; goto done; }
	
	cptr2 = strstr(cptr,end_element);	
	if(cptr2 == NULL) { rv = 1; goto done; }

	/* At this point we have succesfully found the element and its delimiter */

	/* Now find the property */
	if(property){
		cptr3 = strstr(cptr,start_property);
		if(cptr3 == NULL) { rv = 1; goto done; }
		if(cptr3 > cptr2 ) goto next_element;
		if(value){
			cptr4 = strstr(cptr3,value);
			if(cptr4 == NULL) { rv = 1; goto done; }
			if(cptr4 > cptr2) goto next_element;
			cptr4 += strlen(value) + 2;	
			length = cptr2-cptr4;
			*buffer = malloc(length + 1);
			strncpy(*buffer,cptr4,length);
			(*buffer)[length]=0;
		}else{
			cptr3+=strlen(start_property);
			for(i=0;cptr3[i]!= '\"';i++); 
			*buffer = malloc(i+1);
			strncpy(*buffer,cptr3,i);
			(*buffer)[i] = 0;
		}
	}else{
		cptr+=strlen(start_element);
		length =  (cptr2-cptr); 
		*buffer = malloc( length+1 );
		strncpy(*buffer,cptr,length);
		(*buffer)[length]=0;
	}

	rv = 0;
	done:
	free(start_element);
	free(end_element);
	if(start_property)free(start_property);
	return rv;	
}

int lfm_helper_get_all_xml_element_content(const WebData *xml,
	const char **element_path,const char *element, 
	const char *property, const char *value, char ***buffer){
	char *cptr0=NULL,* cptr1=NULL,*cptr2=NULL;
	int depth;
	int i;
	int allocated;
	char *end_element = NULL;
	char **tarray=NULL;
	char *tbuf = NULL;	
	char *copy = NULL;
	int rv;
	
	for(depth=0;element_path[depth];depth++);
	
	cptr1 = xml->page;
	for(i=0;i< (depth-1);i++){
		cptr1 = strstr(cptr1,element_path[i]);
		if(cptr1 == NULL) return 1;
		cptr1+= strlen(element_path[i]);
	}
	rv = asprintf(&end_element,"/%s\n",element_path[depth-1]);
	
	cptr1 -= strlen(element_path[depth-1]);
	copy = malloc(xml->size - (cptr1-xml->page));
	memcpy(copy,cptr1,xml->size - (cptr1-xml->page) )  ;

	cptr0 = strtok(copy,"<>");
	
	allocated = 1;
	tarray = malloc(sizeof(char *));
	while(cptr0 != NULL){
	//printf("cptr0=|%s|\n",cptr0);
	   if(strncmp(cptr0,element_path[depth-1],strlen(element_path[depth-1])) == 0){
	      cptr0 = strtok(NULL,"<>");
	      while(cptr0 != NULL){
	         if(strcmp(cptr0,end_element) == 0) goto next_item;
	         if(memcmp(cptr0,element,strlen(element)) == 0){
	            if(property){
	               cptr2 = strstr(cptr0,property);
	               if(cptr2 == NULL) goto next_item;
	               if(value){
	                  cptr2 = strstr(cptr2,value);
	                  if(cptr2 == NULL ) goto next_property;
	               }else { // The user wants the value of the property
	                       // property="value"
	                  cptr2 += strlen(property);
	                  if(*cptr2 != '=') goto next_item;
	                  cptr2+=2;
	                  /* Please compiler, dont optimise this out */
	                  for(i=0;cptr2[i]!= '\"';i++); 
		          tarray = realloc(tarray,sizeof(char*) * (allocated+1));
	                  tbuf = malloc(i+1);
	                  memcpy(tbuf,cptr2,i-1);
	                  tbuf[i] = 0;
	                  tarray[allocated-1] = tbuf;
	                  allocated++;
	                  goto next_item;
	               }
	            }
	            // Content should be the next token
	            cptr2 = strtok(NULL,"<>");
	            if(cptr2 == NULL) goto next_item;
	            tarray = realloc(tarray,sizeof(char*) * (allocated+1));
	            tarray[allocated-1] = strdup(cptr2);
	            allocated++;
	            goto next_item;
	         }
	         next_property:
	         cptr0 = strtok(NULL,"<>");
	      }
	   }
	   next_item:
	   cptr0 = strtok(NULL,"<>");
	}

	free(end_element);
	free(copy);

	if(allocated == 1){
		free(tarray);
		*buffer = NULL;
	}else {
		tarray[allocated-1] = NULL;
		*buffer = tarray;
	}
	return 0;
}
	
/* Size must be a multiple of 2, ie there are (size/2) params in 'args';
 * Each param is at N
 * Each value is at N+1
 */

WebData *lfm_helper_post_page(CURL *curl, curl_progress_callback cb, void *cb_data, const char *url,const char *args){
	WebData *wpage;
	int own_curl=0;
	CURLcode result;
	
	printf("Posting to URL|%s|\n",url);

	wpage = malloc(sizeof(WebData));
	wpage->size=0;
	wpage->page=NULL;
	/* Are we using our own handle ? */
	if(!curl){
		own_curl=1;
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_POST , 1 ) ;
		/* send all data to this function  */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteWebDataCallback);
		/* Allow redirection */
		curl_easy_setopt(curl ,CURLOPT_FOLLOWLOCATION, 1); 

	}
	/* we pass our 'WebData' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)wpage);
	curl_easy_setopt(curl, CURLOPT_URL,url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, args);

	result = curl_easy_perform(curl);
	printf("Curl result = %i\n",result);
	if(!wpage)return NULL;
	/* Did we use our curl, and was the result not bad */
	if(own_curl){
		curl_easy_cleanup(curl);
	}
	if(result){
		if(wpage)lfm_helper_free_page(wpage);
		wpage = NULL;
	}
	return wpage;
}
	
int lfm_helper_free_page(WebData *wpage){
	if(!wpage)return 1;
	if(wpage->page)	free(wpage->page);
	free(wpage);
	return 0;
}
	
//WebData *lfm_helper_get_page(const char *url)
WebData *lfm_helper_get_page(const char *url, curl_progress_callback cb, void *cb_data)
{
	/* libcurl handle */
	CURL *curl_handle;

	WebData *chunk;
	
	printf("Getting URL|%s|\n",url);
	chunk = malloc(sizeof(WebData));

	chunk->page	= NULL; /* we expect realloc(NULL, size) to work */
	chunk->size	= 0;    /* no data at this point */

	/* init the curl session */
	curl_handle = curl_easy_init();

	/* specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);

	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteWebDataCallback);

	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)chunk);

	/* some servers don't like requests that are made without a user-agent
	 * field, so we provide one */
	/* Just in case somone out there doesnt like us */
	//curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, 
"Mozilla/5.0 (X11; U; Linux 2.4.2-2 i586; en-US; m18) Gecko/20010131 Netscape6/6.01");

	/* progress bar */
	curl_easy_setopt(curl_handle,CURLOPT_NOPROGRESS,0);
	curl_easy_setopt(curl_handle,CURLOPT_PROGRESSFUNCTION,cb);
	curl_easy_setopt(curl_handle,CURLOPT_PROGRESSDATA,cb_data);

	/* get it! */
	curl_easy_perform(curl_handle);

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	/* we're done with libcurl, so clean it up */
	curl_global_cleanup();

//	printf("%s\n",chunk->page);
	return chunk;
}
