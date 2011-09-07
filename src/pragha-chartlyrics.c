/************************************************************************
 * Copyright (C) 2011 matias <mati86dl@gmail.com>                       *
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
#include <pthread.h>

#define LARGE_BUFFER	1024
#define CHARTLYRICS_API_ROOT	"http://api.chartlyrics.com/apiv1.asmx/"

#ifdef HAVE_LIBGLYR
void *do_chartlyric_dialog (gpointer data)
{
	GtkWidget *dialog;
	GtkWidget *header, *view, *frame, *scrolled;
	GtkTextBuffer *buffer;
	gchar *title_header = NULL;
	gint result;
	GdkCursor *cursor;
	GlyrQuery q;
	GLYR_ERROR err;

	struct con_win *cwin = data;

	glyr_init();

	gdk_threads_enter ();
	cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), cursor);
	gdk_cursor_unref(cursor);
	gdk_threads_leave ();

	glyr_init_query(&q);
	glyr_opt_type(&q,GLYR_GET_LYRICS);

	glyr_opt_artist(&q,(char*)cwin->cstate->curr_mobj->tags->artist);
	glyr_opt_album (&q,(char*)cwin->cstate->curr_mobj->tags->album);
	glyr_opt_title (&q,(char*)cwin->cstate->curr_mobj->tags->title);
	
	GlyrMemCache *head = glyr_get(&q,&err,NULL);

	if(head == NULL) {
		gdk_threads_enter ();
		set_status_message(_("Error searching Lyric on Chartlyrics."), cwin);
		gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);
		gdk_threads_leave ();
		return NULL;
	}

	gdk_threads_enter ();
	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
	gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (view), FALSE);

	frame = gtk_frame_new (NULL);
	scrolled = gtk_scrolled_window_new (NULL, NULL);

	gtk_container_add (GTK_CONTAINER (scrolled), view);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 8);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), scrolled);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	gtk_text_buffer_set_text (buffer, head->data, -1);

	dialog = gtk_dialog_new_with_buttons(head->prov,
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	gtk_dialog_add_button(GTK_DIALOG(dialog), _("_Edit"), GTK_RESPONSE_HELP);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	title_header = g_markup_printf_escaped (_("%s <small><span weight=\"light\">by</span></small> %s"),
				cwin->cstate->curr_mobj->tags->title, cwin->cstate->curr_mobj->tags->artist);
	header = sokoke_xfce_header_new (title_header, NULL, cwin);
	g_free(title_header);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), header, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);

	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result) {
		case GTK_RESPONSE_HELP:
			//open_url (cwin, lyric_info->url);
			break;
		case GTK_RESPONSE_OK:
			break;
		default:
			break;
	}

	gtk_widget_destroy(dialog);
	gdk_threads_leave ();

	glyr_free_list(head);
	glyr_destroy_query(&q);
	glyr_cleanup ();

	return NULL;
}
#else
typedef struct {
	unsigned int id;
	char checksum[33];
	char *song;
	char *artist;
	char *lyric;
	char *url;
	char *correct_url;
} Lyric;

const char *HTML_ESCAPE[] = {
	"&amp;", "&",
	"&quot;","\"",
	"&ndash;","-",
	NULL
};

Lyric *malloc_lyric(){
	Lyric *l;

	l = malloc(sizeof(Lyric));
	l->id     = 0;
	l->checksum[0]=0;
	l->song        = NULL;
	l->artist      = NULL;
	l->lyric       = NULL;
	l->url         = NULL;
	l->correct_url = NULL;
	return l;
}

void free_lyric(Lyric *l){
	if(l == NULL) return;
	if(l->song)        free(l->song);
	if(l->artist)      free(l->artist);
	if(l->lyric)       free(l->lyric);
	if(l->url)         free(l->url);
	if(l->correct_url) free(l->correct_url);
	free(l);
}

void print_lyric(FILE *f, Lyric *l){
	if(l == NULL) return;
	fprintf(f,"checksum    = %s\n",l->checksum);
	fprintf(f,"id          = %u\n",l->id);
	fprintf(f,"artist      = %s\n",l->artist);
	fprintf(f,"song        = %s\n",l->song);
	fprintf(f,"lyric       = %s\n",l->lyric);
	fprintf(f,"url         = %s\n",l->url);
	fprintf(f,"correct_url = %s\n",l->correct_url);
}

Lyric *chartlyrics_get_info(const char *song, const char *artist)
{
	char *buffer;
	WebData *data = NULL;
	XMLNode *xml = NULL, *xi, *xj;
	char *q_artist,*q_song;
	Lyric *l;
	CURL *curl;

	curl_global_init(CURL_GLOBAL_ALL);

	/* Setup a CURL handle */
	curl = curl_easy_init();
        curl_easy_setopt(curl,CURLOPT_COOKIEFILE," ");

	// Allow redirection
        curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION, 1); 

	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, write_cb);

	buffer = malloc(LARGE_BUFFER);

	/* Escape into HTML friendly strings */
	q_artist = curl_easy_escape(curl,artist,0);
	q_song = curl_easy_escape(curl,song,0);

	snprintf(buffer,LARGE_BUFFER,
		"%s%s?artist=%s&song=%s",
		CHARTLYRICS_API_ROOT,"SearchLyricDirect",q_artist,q_song);
	curl_free(q_artist);
	curl_free(q_song);

	data = chartlyrics_helper_get_page(curl,buffer);
	free(buffer);

	/* Done with CURL */
	curl_global_cleanup();

	if(data == NULL || data->size == 0)
		return NULL;

	xml = tinycxml_parse(data->page);

	chartlyrics_helper_free_page(data);

	l = malloc_lyric();

	xi = xmlnode_get(xml,CCA {"GetLyricResult",NULL},NULL,NULL);
	if(xi == NULL){
		xmlnode_free(xml);
		return NULL;
	}

	xj = xmlnode_get(xml, CCA {"GetLyricResult","LyricChecksum",NULL},NULL,NULL);
	if(xj && xj->content)
		strcpy(l->checksum,xj->content);

	xj = xmlnode_get(xi,CCA { "GetLyricResult","LyricId",NULL},NULL,NULL);
	if(xj && xj->content)
		l->id = atoi(xj->content);

	xj = xmlnode_get(xi,CCA { "GetLyricResult","LyricSong",NULL},NULL,NULL);
	if(xj && xj->content)
		l->song = unescape_HTML(strdup(xj->content));

	xj = xmlnode_get(xi,CCA { "GetLyricResult","LyricArtist",NULL},NULL,NULL);
	if(xj && xj->content)
		l->artist = unescape_HTML(strdup(xj->content));

	xj = xmlnode_get(xi,CCA { "GetLyricResult","Lyric",NULL},NULL,NULL);
	if(xj && xj->content)
		l->lyric = unescape_HTML(strdup(xj->content));

	xj = xmlnode_get(xi,CCA { "GetLyricResult","LyricUrl",NULL},NULL,NULL);
	if(xj && xj->content)
		l->url = unescape_HTML(strdup(xj->content));

	xj = xmlnode_get(xi,CCA { "GetLyricResult","LyricCorrectUrl",NULL},NULL,NULL);
	if(xj && xj->content)
		l->correct_url = unescape_HTML(strdup(xj->content));

	xmlnode_free(xml);

	return l;
}

void *do_chartlyric_dialog (gpointer data)
{
	GtkWidget *dialog;
	GtkWidget *header, *view, *frame, *scrolled;
	GtkTextBuffer *buffer;
	gchar *title_header = NULL;
	gint result;
	GdkCursor *cursor;

	Lyric *lyric_info;

	struct con_win *cwin = data;

	gdk_threads_enter ();

	cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), cursor);
	gdk_cursor_unref(cursor);

	gdk_threads_leave ();

	lyric_info = chartlyrics_get_info (cwin->cstate->curr_mobj->tags->title, cwin->cstate->curr_mobj->tags->artist);

	if(!lyric_info) {
		gdk_threads_enter ();
		set_status_message(_("Error searching Lyric on Chartlyrics."), cwin);
		gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);
		gdk_threads_leave ();
		return NULL;
	}

	gdk_threads_enter ();
	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
	gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (view), FALSE);

	frame = gtk_frame_new (NULL);
	scrolled = gtk_scrolled_window_new (NULL, NULL);

	gtk_container_add (GTK_CONTAINER (scrolled), view);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 8);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), scrolled);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	if(lyric_info->lyric)
		gtk_text_buffer_set_text (buffer, lyric_info->lyric, -1);

	dialog = gtk_dialog_new_with_buttons("Chartlyrics",
					     GTK_WINDOW(cwin->mainwindow),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_OK,
					     GTK_RESPONSE_OK,
					     NULL);

	gtk_dialog_add_button(GTK_DIALOG(dialog), _("_Edit"), GTK_RESPONSE_HELP);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	title_header = g_markup_printf_escaped (_("%s <small><span weight=\"light\">by</span></small> %s"),
				lyric_info->song, lyric_info->artist);
	header = sokoke_xfce_header_new (title_header, NULL, cwin);
	g_free(title_header);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), header, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);

	gdk_window_set_cursor(GDK_WINDOW(cwin->mainwindow->window), NULL);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result) {
		case GTK_RESPONSE_HELP:
			open_url (cwin, lyric_info->url);
			break;
		case GTK_RESPONSE_OK:
			break;
		default:
			break;
	}

	gtk_widget_destroy(dialog);
	gdk_threads_leave ();

	free_lyric(lyric_info);

	return NULL;
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

char *unescape_HTML(char *original){
	int i;
	char *cptr;
	if(original == NULL) return NULL;

	for(i=0;HTML_ESCAPE[i];i+=2){
		cptr = strstr(original,HTML_ESCAPE[i]);
		while(cptr){
			// This may not work on older systems
			sprintf(cptr,"%s%s",
				HTML_ESCAPE[i+1],
				cptr+strlen(HTML_ESCAPE[i]));
			cptr = strstr(original,HTML_ESCAPE[i]);
		}
	}
	return original;
}

size_t write_cb(void *ptr, size_t size, size_t nmemb, void *data){
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
	}else {
		perror("write_cb: Could not realloc");
		return 0;
	}
}

int chartlyrics_helper_free_page(WebData *wpage){
	if(wpage == NULL)return 1;
	if(wpage->page)	free(wpage->page);
	free(wpage);
	return 0;
}
	
WebData *chartlyrics_helper_get_page(CURL *curl, const char *url){
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

	if(chunk->size == 0){
		if(chunk->page){
			free(chunk->page);
			chunk->page = NULL;
		}
	}
	return chunk;
}
#endif

void chartlyric_dialog (struct con_win *cwin)
{
	pthread_t tid;

	if(cwin->cstate->state == ST_STOPPED)
		return;

	CDEBUG(DBG_INFO, "Get lyrics Action");

	pthread_create(&tid, NULL, do_chartlyric_dialog, cwin);
}