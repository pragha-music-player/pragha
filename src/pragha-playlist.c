/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>			 */
/* 									 */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/* 									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 */
/* GNU General Public License for more details.				 */
/* 									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <gdk/gdkkeysyms.h>
#include "pragha-playlist.h"
#include "pragha-file-utils.h"
#include "pragha-menubar.h"
#include "pragha-utils.h"
#include "pragha-playlists-mgmt.h"
#include "gtkcellrendererbubble.h"
#include "pragha-tagger.h"
#include "pragha-tags-mgmt.h"
#include "pragha-tags-dialog.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-dnd.h"
#include "pragha.h"

/**
 * PraghaPlaylist - Pertains to the current state of the playlist
 * @view - The playlist tree view widget
 * @widget - The parent widget containing the view
 * @changing: If current platlist change is in progress
 * @no_tracks: Total no. of tracks in the current playlist
 * @unplayed_tracks: Total no. of tracks that haven't been played
 * @rand: To generate random numbers
 * @rand_track_refs: List of references maintained in Shuffle mode
 * @queue_track_refs: List of references of queued songs
 * @curr_rand_ref: Currently playing track in Shuffle mode
 * @curr_seq_ref: Currently playing track in non-Shuffle mode
 */

struct _PraghaPlaylist {
	GtkScrolledWindow    __parent__;

	/* Global database and preferences instances */

	PraghaDatabase      *cdbase;
	PraghaPreferences   *preferences;

	/* List view. */

	GtkWidget           *view;
	GtkTreeModel        *model;
	GSList              *columns;
	GSList              *column_widths;

	/* Playback control. */

	GRand               *rand;
	GList               *rand_track_refs;
	GSList              *queue_track_refs;
	GtkTreeRowReference *curr_rand_ref;
	GtkTreeRowReference *curr_seq_ref;
	gint                 unplayed_tracks;

	/* Useful flags */

	gboolean             changing;
	gboolean             dragging;
	gint                 no_tracks;
	GError              *track_error;

	/* Pixbuf used on library tree. */

	GdkPixbuf           *paused_pixbuf;
	GdkPixbuf           *playing_pixbuf;

	/* Menu */
	GtkWidget           *header_context_menu;
	GtkUIManager        *playlist_context_menu;
};

enum
{
	PLAYLIST_SET_TRACK,
	PLAYLIST_CHANGE_TAGS,
	PLAYLIST_CHANGED,
	LAST_SIGNAL
};

static int signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(PraghaPlaylist, pragha_playlist, GTK_TYPE_SCROLLED_WINDOW)

/* Columns in current playlist view */

#define P_TRACK_NO_STR      "#"
#define P_TITLE_STR         N_("Title")
#define P_ARTIST_STR        N_("Artist")
#define P_ALBUM_STR         N_("Album")
#define P_GENRE_STR         N_("Genre")
#define P_BITRATE_STR       N_("Bitrate")
#define P_YEAR_STR          N_("Year")
#define P_COMMENT_STR       N_("Comment")
#define P_LENGTH_STR        N_("Length")
#define P_FILENAME_STR      N_("Filename")

/*
 * Prototypes
 */

static void         pragha_playlist_queue_handler      (PraghaPlaylist *playlist);
static void         pragha_playlist_dequeue_handler    (PraghaPlaylist *playlist);

static GtkTreePath* get_first_random_track             (PraghaPlaylist *playlist);
static GtkTreePath* get_prev_random_track              (PraghaPlaylist *playlist);
static GtkTreePath* get_prev_sequential_track          (PraghaPlaylist *playlist);
static GtkTreePath* get_next_queue_track               (PraghaPlaylist *cplaylist);
static GtkTreePath* get_next_unplayed_random_track     (PraghaPlaylist *playlist);
static GtkTreePath* get_next_sequential_track          (PraghaPlaylist *playlist);
static GtkTreePath* get_next_random_ref_track          (PraghaPlaylist *playlist);
static GtkTreePath* get_next_any_random_track          (PraghaPlaylist *playlist);
static GtkTreePath* get_nth_track                      (PraghaPlaylist *playlist, gint n);
static GtkTreePath* get_selected_track                 (PraghaPlaylist *playlist);
static GtkTreePath* get_current_track                  (PraghaPlaylist *playlist);

static void         pragha_playlist_update_playback_sequence (PraghaPlaylist *playlist, PraghaUpdateAction update_action, GtkTreePath *path);

static void         pragha_playlist_set_first_rand_ref (PraghaPlaylist *cplaylist, GtkTreePath *path);

static void         pragha_playlist_select_path        (PraghaPlaylist *playlist, GtkTreePath *path, gboolean center);

static void         pragha_playlist_change_ref_list_tags (PraghaPlaylist *playlist, GList *rlist, gint changed, PraghaMusicobject *mobj);
static void         pragha_playlist_update_track_state    (PraghaPlaylist *playlist, GtkTreePath *path, PraghaBackendState state);

static void playlist_track_column_change_cb    (GtkCheckMenuItem *item, PraghaPlaylist* cplaylist);
static void playlist_title_column_change_cb    (GtkCheckMenuItem *item, PraghaPlaylist* cplaylist);
static void playlist_artist_column_change_cb   (GtkCheckMenuItem *item, PraghaPlaylist* cplaylist);
static void playlist_album_column_change_cb    (GtkCheckMenuItem *item, PraghaPlaylist* cplaylist);
static void playlist_genre_column_change_cb    (GtkCheckMenuItem *item, PraghaPlaylist* cplaylist);
static void playlist_bitrate_column_change_cb  (GtkCheckMenuItem *item, PraghaPlaylist* cplaylist);
static void playlist_year_column_change_cb     (GtkCheckMenuItem *item, PraghaPlaylist* cplaylist);
static void playlist_length_column_change_cb   (GtkCheckMenuItem *item, PraghaPlaylist* cplaylist);
static void playlist_comment_column_change_cb  (GtkCheckMenuItem *item, PraghaPlaylist* cplaylist);
static void playlist_filename_column_change_cb (GtkCheckMenuItem *item, PraghaPlaylist* cplaylist);

static void clear_sort_current_playlist_cb (GtkMenuItem *item, PraghaPlaylist *cplaylist);

static gint compare_track_no (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data);
static gint compare_bitrate  (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data);
static gint compare_year     (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data);
static gint compare_length   (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data);

/*
 * playlist_context_menu calbacks
 */
static void queue_current_playlist        (GtkAction *action, PraghaPlaylist *playlist);
static void dequeue_current_playlist      (GtkAction *action, PraghaPlaylist *playlist);
static void remove_from_playlist          (GtkAction *action, PraghaPlaylist *playlist);
static void crop_current_playlist         (GtkAction *action, PraghaPlaylist *playlist);
static void current_playlist_clear_action (GtkAction *action, PraghaPlaylist *playlist);
static void pragha_playlist_copy_tags     (GtkAction *action, PraghaPlaylist *playlist);
static void pragha_playlist_edit_tags     (GtkAction *action, PraghaPlaylist *playlist);

static const gchar *playlist_context_menu_xml = "<ui>				\
	<popup name=\"SelectionPopup\">		   				\
	<menuitem action=\"Queue track\"/>					\
	<menuitem action=\"Dequeue track\"/>					\
	<separator/>				    				\
	<menuitem action=\"Remove from playlist\"/>				\
	<menuitem action=\"Crop playlist\"/>					\
	<menuitem action=\"Clear playlist\"/>					\
	<separator/>				    				\
	<menuitem action=\"Save playlist\"/>					\
	<menuitem action=\"Save selection\"/>					\
	<menu action=\"SendToMenu\">						\
		<placeholder name=\"pragha-sendto-placeholder\"/>		\
	</menu>									\
	<separator/>				    				\
	<menu action=\"ToolsMenu\">						\
		<placeholder name=\"pragha-plugins-placeholder\"/>		\
	</menu>									\
	<separator/>				    				\
	<menuitem action=\"Copy tag to selection\"/>				\
	<separator/>				    				\
	<menuitem action=\"Edit tags\"/>					\
	</popup>				    				\
	</ui>";

static GtkActionEntry playlist_context_aentries[] = {
	{"Queue track", "list-add", N_("Add to playback queue"),
	 "", "Add to playback queue", G_CALLBACK(queue_current_playlist)},
	{"Dequeue track", "list-remove", N_("Remove from playback queue"),
	 "", "Remove from playback queue", G_CALLBACK(dequeue_current_playlist)},
	{"Remove from playlist", "list-remove", N_("Remove from playlist"),
	 "", "Remove selection from playlist", G_CALLBACK(remove_from_playlist)},
	{"Crop playlist", "list-remove", N_("Crop playlist"),
	 "", "Remove no telected tracks of playlist", G_CALLBACK(crop_current_playlist)},
	{"Clear playlist", "edit-clear", N_("Clear playlist"),
	 "", "Clear the current playlist", G_CALLBACK(current_playlist_clear_action)},
	{"Save playlist", "document-save", N_("Save playlist")},
	{"Save selection", "document-save-as", N_("Save selection")},
	{"SendToMenu", NULL, N_("_Send to")},
	{"ToolsMenu", NULL, N_("_Tools")},
	{"Copy tag to selection", "edit-copy", NULL,
	 "", "Copy tag to selection", G_CALLBACK(pragha_playlist_copy_tags)},
	{"Edit tags", NULL, N_("Edit track information"),
	 "", "Edit information for this track", G_CALLBACK(pragha_playlist_edit_tags)}
};

/*
 * Menu action callbacks.
 */

static void
queue_current_playlist (GtkAction *action, PraghaPlaylist *playlist)
{
	pragha_playlist_queue_handler (playlist);
}

static void
dequeue_current_playlist (GtkAction *action, PraghaPlaylist *playlist)
{
	pragha_playlist_dequeue_handler (playlist);
}

static void
remove_from_playlist (GtkAction *action, PraghaPlaylist *playlist)
{
	pragha_playlist_remove_selection (playlist);
}

static void
crop_current_playlist (GtkAction *action, PraghaPlaylist *playlist)
{
	pragha_playlist_crop_selection (playlist);
}

static void
current_playlist_clear_action (GtkAction *action, PraghaPlaylist *playlist)
{
	pragha_playlist_remove_all (playlist);
}

static void
pragha_playlist_copy_tags (GtkAction *action, PraghaPlaylist *playlist)
{
	PraghaMusicobject *mobj = NULL;
	gint changed = 0;
	GList *rlist;

	mobj    = g_object_get_data (G_OBJECT(action), "mobj");
	changed = GPOINTER_TO_INT(g_object_get_data (G_OBJECT(action), "change"));
	rlist   = pragha_playlist_get_selection_ref_list (playlist);

	pragha_playlist_change_ref_list_tags (playlist, rlist, changed, mobj);

	g_list_free_full (rlist, (GDestroyNotify) gtk_tree_row_reference_free);
}

static void
pragha_edit_tags_playlist_dialog_response (GtkWidget      *dialog,
                                           gint            response_id,
                                           PraghaPlaylist *playlist)
{
	GtkWidget *toplevel;
	PraghaMusicobject *nmobj = NULL;
	gint changed = 0;
	GList *rlist = NULL;

	toplevel = gtk_widget_get_toplevel (GTK_WIDGET(playlist));

	if (response_id == GTK_RESPONSE_HELP) {
		nmobj = pragha_tags_dialog_get_musicobject(PRAGHA_TAGS_DIALOG(dialog));
		pragha_track_properties_dialog (nmobj, toplevel);
		return;
	}

	rlist = g_object_get_data (G_OBJECT (dialog), "reference-list");

	if (response_id == GTK_RESPONSE_OK) {
		changed = pragha_tags_dialog_get_changed (PRAGHA_TAGS_DIALOG(dialog));
		if(!changed)
			goto no_change;

		nmobj = pragha_tags_dialog_get_musicobject (PRAGHA_TAGS_DIALOG(dialog));

		if (rlist)
			pragha_playlist_change_ref_list_tags (playlist, rlist, changed, nmobj);
	}

no_change:
	g_list_free_full (rlist, (GDestroyNotify) gtk_tree_row_reference_free);

	gtk_widget_destroy (dialog);
}

static void
pragha_playlist_edit_tags (GtkAction *action, PraghaPlaylist *playlist)
{
	GtkWidget *dialog;
	GList *rlist = NULL;
	PraghaMusicobject *mobj;

	dialog = pragha_tags_dialog_new();

	/* Get a list of references and music objects selected. */

	rlist = pragha_playlist_get_selection_ref_list (playlist);
	if(g_list_length(rlist) == 1) {
		mobj = pragha_playlist_get_selected_musicobject (playlist);
		pragha_tags_dialog_set_musicobject(PRAGHA_TAGS_DIALOG(dialog), mobj);
	}
	g_object_set_data (G_OBJECT (dialog), "reference-list", rlist);
 
	g_signal_connect (G_OBJECT (dialog), "response",
	                  G_CALLBACK (pragha_edit_tags_playlist_dialog_response), playlist);

	gtk_widget_show (dialog);
}

/*
 * Public Api.
 */

PraghaMusicobject *
pragha_playlist_get_prev_track (PraghaPlaylist *playlist)
{
	PraghaMusicobject *mobj = NULL;
	GtkTreePath *path = NULL;
	gboolean repeat, shuffle, seq_first = FALSE;

	if (playlist->changing ||
		playlist->no_tracks == 0)
		return NULL;

	repeat = pragha_preferences_get_repeat (playlist->preferences);
	shuffle = pragha_preferences_get_shuffle (playlist->preferences);

	if (shuffle) {
		path = get_prev_random_track (playlist);
	}
	else {
		path = get_prev_sequential_track (playlist);
		if (!path)
			seq_first = TRUE;
	}

	if (seq_first && repeat)
		path = get_nth_track (playlist, (playlist->no_tracks - 1));
	
	if (!path)
		return NULL;

	pragha_playlist_update_playback_sequence (playlist, PLAYLIST_PREV, path);

	mobj = current_playlist_mobj_at_path (path, playlist);

	gtk_tree_path_free (path);

	return mobj;
}

PraghaMusicobject *
pragha_playlist_get_any_track (PraghaPlaylist *playlist)
{
	PraghaMusicobject *mobj = NULL;
	GtkTreePath *path = NULL;
	gboolean shuffle;

	if (playlist->changing ||
		playlist->no_tracks == 0)
		return NULL;

	shuffle = pragha_preferences_get_shuffle (playlist->preferences);

	if (playlist->queue_track_refs)
		path = get_next_queue_track (playlist);
	if (!path)
		path = get_selected_track (playlist);
	if (!path) {
		if (shuffle)
			path = get_first_random_track (playlist);
		else
			path = gtk_tree_path_new_first ();
	}

	if (shuffle)
		pragha_playlist_set_first_rand_ref (playlist, path);

	pragha_playlist_update_playback_sequence (playlist, PLAYLIST_CURR, path);

	mobj = current_playlist_mobj_at_path (path, playlist);

	gtk_tree_path_free (path);

	return mobj;
}

PraghaMusicobject *
pragha_playlist_get_next_track (PraghaPlaylist *playlist)
{
	PraghaMusicobject *mobj = NULL;
	GtkTreePath *path = NULL;
	gboolean repeat, shuffle, rand_unplayed = FALSE, seq_last = FALSE;
	GList *last;

	if (playlist->changing ||
		playlist->no_tracks == 0)
		return NULL;

	repeat = pragha_preferences_get_repeat (playlist->preferences);
	shuffle = pragha_preferences_get_shuffle (playlist->preferences);

	if (playlist->queue_track_refs) {
		path = get_next_queue_track (playlist);
	}
	else {
		if (shuffle) {
			last = g_list_last (playlist->rand_track_refs);
			if ((!playlist->curr_rand_ref) || (last && (playlist->curr_rand_ref == last->data))) {
				path = get_next_unplayed_random_track (playlist);
				if (!path)
					rand_unplayed = TRUE;
			}
			else
				path = get_next_random_ref_track (playlist);
		}
		else {
			path = get_next_sequential_track (playlist);
			if (!path)
				seq_last = TRUE;
		}
	}

	if (rand_unplayed && repeat)
		path = get_next_any_random_track (playlist);

	if (seq_last && repeat)
		path = get_nth_track (playlist, 0);

	if (!path)
		return NULL;

	pragha_playlist_update_playback_sequence (playlist, PLAYLIST_NEXT, path);

	mobj = current_playlist_mobj_at_path (path, playlist);

	gtk_tree_path_free (path);

	return mobj;
}

void
pragha_playlist_go_prev_track (PraghaPlaylist *playlist)
{
	PraghaMusicobject *mobj = NULL;
	mobj = pragha_playlist_get_prev_track (playlist);

	if (mobj)
		g_signal_emit (playlist, signals[PLAYLIST_SET_TRACK], 0, mobj);
}

void
pragha_playlist_go_any_track (PraghaPlaylist *playlist)
{
	PraghaMusicobject *mobj = NULL;
	mobj = pragha_playlist_get_any_track (playlist);

	g_signal_emit (playlist, signals[PLAYLIST_SET_TRACK], 0, mobj);
}

void
pragha_playlist_go_next_track (PraghaPlaylist *playlist)
{
	PraghaMusicobject *mobj = NULL;
	mobj = pragha_playlist_get_next_track (playlist);

	g_signal_emit (playlist, signals[PLAYLIST_SET_TRACK], 0, mobj);

	if (!mobj)
		pragha_playlist_stopped_playback (playlist);
}

void
pragha_playlist_stopped_playback (PraghaPlaylist *playlist)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean ret;

	/* Clear playback icon. */
	path = get_current_track (playlist);
	if (path)
		pragha_playlist_update_track_state (playlist, path, ST_STOPPED);

	/* Mark all as playable */
	ret = gtk_tree_model_get_iter_first (playlist->model, &iter);
	while(ret) {
		gtk_list_store_set(GTK_LIST_STORE(playlist->model), &iter, P_PLAYED, FALSE, -1);
		ret = gtk_tree_model_iter_next(playlist->model, &iter);
	}
	playlist->unplayed_tracks = playlist->no_tracks;

	/* Remove old references */
	if (playlist->rand_track_refs) {
		g_list_free_full(playlist->rand_track_refs, (GDestroyNotify) gtk_tree_row_reference_free);
		playlist->rand_track_refs = NULL;
	}
	playlist->curr_rand_ref = NULL;

	if (playlist->curr_seq_ref) {
		gtk_tree_row_reference_free (playlist->curr_seq_ref);
		playlist->curr_seq_ref = NULL;
	}

	gtk_tree_path_free (path);
}

/* Update playback state pixbuf */

static void
pragha_playlist_update_track_state (PraghaPlaylist *playlist, GtkTreePath *path, PraghaBackendState state)
{
	GtkIconTheme *icon_theme;
	GdkPixbuf *pixbuf = NULL;
	GtkTreeIter iter;

	if (pragha_playlist_is_changing (playlist))
		return;

	if (playlist->track_error) {
		icon_theme = gtk_icon_theme_get_default ();

		if(playlist->track_error->code == GST_RESOURCE_ERROR_NOT_FOUND)
			pixbuf = gtk_icon_theme_load_icon (icon_theme, "list-remove", 16, 0, NULL);
		else
			pixbuf = gtk_icon_theme_load_icon (icon_theme, "dialog-warning", 16, 0, NULL);
	}
	else {
		switch (state) {
			case ST_PLAYING:
				pixbuf = playlist->playing_pixbuf;
				break;
			case ST_PAUSED:
				pixbuf = playlist->paused_pixbuf;
				break;
			default:
				break;
		}
	}

	if (gtk_tree_model_get_iter (playlist->model, &iter, path))
		gtk_list_store_set (GTK_LIST_STORE(playlist->model), &iter, P_STATUS_PIXBUF, pixbuf, -1);

	if (playlist->track_error)
		g_object_unref (pixbuf);
}

void
pragha_playlist_show_current_track (PraghaPlaylist *playlist)
{
	GtkTreePath *path = NULL;
	gboolean shuffle = FALSE;

	if (pragha_playlist_is_changing (playlist))
		return;

	path = get_current_track (playlist);

	if (path) {
		shuffle = pragha_preferences_get_shuffle (playlist->preferences);

		pragha_playlist_select_path (playlist, path, shuffle);
		gtk_tree_path_free (path);
	}
}

void
pragha_playlist_set_track_error (PraghaPlaylist *playlist, GError *error)
{
	GtkTreePath *path;

	CDEBUG(DBG_VERBOSE, "Set error on current playlist");

	playlist->track_error = g_error_copy (error);

	path = get_current_track (playlist);
	if(path) {
		pragha_playlist_update_track_state (playlist, path, ST_STOPPED);
		gtk_tree_path_free (path);
	}
}

/*
 * Private api..
 */

static void
pragha_playlist_menu_action_set_sensitive (PraghaPlaylist *playlist,
                                           const gchar *action,
                                           gboolean sentitive)
{
	GtkWidget *item_widget;

	item_widget = gtk_ui_manager_get_widget(playlist->playlist_context_menu, action);

	gtk_widget_set_sensitive (GTK_WIDGET(item_widget), sentitive);
}

static void
pragha_playlist_menu_action_set_visible (PraghaPlaylist *playlist,
                                         const gchar *action,
                                         gboolean visible)
{
	GtkWidget *item_widget;

	item_widget = gtk_ui_manager_get_widget(playlist->playlist_context_menu, action);

	gtk_widget_set_visible (GTK_WIDGET(item_widget), visible);
}

/* Clear current seq ref */

static void clear_curr_seq_ref(PraghaPlaylist *cplaylist)
{
	if (!cplaylist->curr_seq_ref)
		return;

	gtk_tree_row_reference_free(cplaylist->curr_seq_ref);
	cplaylist->curr_seq_ref = NULL;
}

/* Clear cstate->curr_seq_ref if it happens to contain the given path */

static void test_clear_curr_seq_ref(GtkTreePath *path, PraghaPlaylist *cplaylist)
{
	GtkTreePath *lpath;

	if (!cplaylist->curr_seq_ref)
		return;

	lpath = gtk_tree_row_reference_get_path(cplaylist->curr_seq_ref);
	if (!gtk_tree_path_compare(path, lpath)) {
		gtk_tree_row_reference_free(cplaylist->curr_seq_ref);
		cplaylist->curr_seq_ref = NULL;
	}
	gtk_tree_path_free(lpath);
}

/* Check if given ref is the current rand reference */

static gboolean is_current_rand_ref(GtkTreeRowReference *ref, PraghaPlaylist *cplaylist)
{
	if (ref == cplaylist->curr_rand_ref)
		return TRUE;
	else
		return FALSE;
}

static void requeue_track_refs (PraghaPlaylist *cplaylist)
{
	GSList *list = NULL;
	GtkTreeRowReference *ref;
	GtkTreeModel *model = cplaylist->model;
	GtkTreePath *lpath;
	gchar *ch_queue_no=NULL;
	GtkTreeIter iter;
	gint i=0;

	list = cplaylist->queue_track_refs;

	while (list) {
		ref = list->data;
		lpath = gtk_tree_row_reference_get_path(ref);
		if (gtk_tree_model_get_iter(model, &iter, lpath)){
			ch_queue_no = g_strdup_printf("%d", ++i);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_QUEUE, ch_queue_no, -1);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_BUBBLE, TRUE, -1);
			g_free(ch_queue_no);
		}
		gtk_tree_path_free(lpath);
		list = list->next;
	}
}

/* Delete the ref corresponding to the given path */

static void delete_queue_track_refs(GtkTreePath *path, PraghaPlaylist *cplaylist)
{
	GSList *list = NULL;
	GtkTreeRowReference *dref = NULL, *ref;
	GtkTreeModel *model = cplaylist->model;
	GtkTreePath *lpath;
	GtkTreeIter iter;

	if (cplaylist->queue_track_refs) {
		list = cplaylist->queue_track_refs;

		while (list) {
			ref = list->data;
			lpath = gtk_tree_row_reference_get_path(ref);

			if (!gtk_tree_path_compare(path, lpath))
				dref = ref;

			if (gtk_tree_model_get_iter(model, &iter, lpath)){
				gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_QUEUE, NULL, -1);
				gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_BUBBLE, FALSE, -1);
			}
			gtk_tree_path_free(lpath);
			list = list->next;
		}
		if (dref) {
			cplaylist->queue_track_refs = g_slist_remove(cplaylist->queue_track_refs, dref);
			gtk_tree_row_reference_free(dref);
		}
	}
}

/* Delete the ref corresponding to the given path */

static void delete_rand_track_refs(GtkTreePath *path, PraghaPlaylist *cplaylist)
{
	GList *list;
	GtkTreeRowReference *ref;
	GtkTreePath *lpath;

	if (cplaylist->rand_track_refs) {
		list = cplaylist->rand_track_refs;
		while (list) {
			ref = list->data;
			lpath = gtk_tree_row_reference_get_path(ref);
			if (!gtk_tree_path_compare(path, lpath)) {
				if (is_current_rand_ref(ref, cplaylist))
					cplaylist->curr_rand_ref = NULL;
				cplaylist->rand_track_refs =
					g_list_remove(cplaylist->rand_track_refs,
						      ref);
				gtk_tree_row_reference_free(ref);
				gtk_tree_path_free(lpath);
				break;
			}
			gtk_tree_path_free(lpath);
			list = list->next;
		}
	}
}

/* Mark a track in current playlist as dirty */

static void
pragha_playlist_set_dirty_track (PraghaPlaylist *playlist,
                                 GtkTreePath    *path)
{
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter (playlist->model, &iter, path))
		gtk_list_store_set (GTK_LIST_STORE(playlist->model), &iter, P_PLAYED, TRUE, -1);

	if (playlist->unplayed_tracks)
		playlist->unplayed_tracks--;
}

/* Mark a track in current playlist as dirty */

static void
pragha_playlist_unset_dirty_track (PraghaPlaylist *cplaylist, GtkTreePath *path)
{
	GtkTreeModel *model = cplaylist->model;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter(model, &iter, path))
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, P_PLAYED, FALSE, -1);

	cplaylist->unplayed_tracks++;
}

/* Mark all tracks in current playlist as clean */

static void
pragha_playlist_clear_dirty_all (PraghaPlaylist *playlist)
{
	GtkTreeIter iter;
	gboolean ret;

	ret = gtk_tree_model_get_iter_first (playlist->model, &iter);
	while(ret) {
		gtk_list_store_set (GTK_LIST_STORE(playlist->model), &iter, P_PLAYED, FALSE, -1);
		ret = gtk_tree_model_iter_next (playlist->model, &iter);
	}

	playlist->unplayed_tracks = playlist->no_tracks;
}

/* Remove all ref of song next current song */

static void
trim_down_rand_track_refs (PraghaPlaylist *playlist)
{
	GList *list;
	GtkTreeRowReference *ref;
	GtkTreePath *lpath;

	if (playlist->rand_track_refs) {
		list = g_list_find (playlist->rand_track_refs, playlist->curr_rand_ref);

		if (list) {
			list = g_list_next(list);
			while (list) {
				ref = list->data;
				lpath = gtk_tree_row_reference_get_path(ref);

				pragha_playlist_unset_dirty_track (playlist, lpath);

				playlist->rand_track_refs =
					g_list_remove (playlist->rand_track_refs, ref);
				gtk_tree_row_reference_free(ref);
				gtk_tree_path_free(lpath);
				list = list->next;
			}
		}
	}
}

/* Return the next node after the given ref */

static GtkTreeRowReference *
get_rand_ref_next (PraghaPlaylist      *playlist,
                   GtkTreeRowReference *ref)
{
	GtkTreeRowReference *ret_ref = NULL;
	GList *list;

	if (!playlist->rand_track_refs)
		return NULL;

	list = playlist->rand_track_refs;
	while (list) {
		if (ref == list->data) {
			ret_ref = list->next->data;
			break;
		}
		list = list->next;
	}
	return ret_ref;
}

/* Return the prev node of the given ref */

static GtkTreeRowReference *
get_rand_ref_prev (PraghaPlaylist      *playlist,
                   GtkTreeRowReference *ref)
{
	GtkTreeRowReference *ret_ref = NULL;
	GList *list;

	if (!playlist->rand_track_refs)
		return NULL;

	list = playlist->rand_track_refs;
	while (list) {
		if (ref == list->data) {
			ret_ref = list->prev->data;
			break;
		}
		list = list->next;
	}
	return ret_ref;
}

/* Return path of track at nth position in current playlist */

static GtkTreePath *
get_nth_track (PraghaPlaylist *playlist, gint n)
{
	GtkTreeIter iter;
	GtkTreePath *path = NULL;

	if (playlist->changing)
		return NULL;

	if(gtk_tree_model_iter_nth_child (playlist->model, &iter, NULL, n))
		path = gtk_tree_model_get_path (playlist->model, &iter);

	return path;
}

/* Return path of the next queue track */

static GtkTreePath *
get_next_queue_track (PraghaPlaylist *cplaylist)
{
	GtkTreePath *path = NULL;

	path = gtk_tree_row_reference_get_path(cplaylist->queue_track_refs->data);

	/* Remove old next song. */
	trim_down_rand_track_refs(cplaylist);

	/*Remove the queue reference and update gui. */
	delete_queue_track_refs(path, cplaylist);
	requeue_track_refs (cplaylist);

	return path;
}

/* Return path of a first random track */

static GtkTreePath *
get_first_random_track (PraghaPlaylist *playlist)
{
	gint rnd;
	GtkTreePath *path = NULL;

	do {
		rnd = g_rand_int_range (playlist->rand,
		                        0,
		                        playlist->no_tracks);
		path = get_nth_track (playlist, rnd);

	} while (playlist->no_tracks > 1 && (path == NULL));

	if (!path) {
		g_printerr("No track at position : %d\n", rnd);
		return NULL;
	}

	return path;
}

/* Return path of next unique random track */

static GtkTreePath *
get_next_unplayed_random_track (PraghaPlaylist *playlist)
{
	gint rnd;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	gboolean played = TRUE;

	while (played && playlist->unplayed_tracks) {
		rnd = g_rand_int_range (playlist->rand,
		                        0,
		                        playlist->no_tracks);
		path = get_nth_track (playlist, rnd);
		if (!path) {
			g_printerr("No track at position : %d\n", rnd);
			return NULL;
		}

		if (gtk_tree_model_get_iter (playlist->model, &iter, path))
			gtk_tree_model_get (playlist->model, &iter, P_PLAYED, &played, -1);

		if (played) {
			gtk_tree_path_free(path);
			path = NULL;
		}
	}
	return path;
}

/* Return path of next random track,
   this is called after exhausting all unique tracks */

static GtkTreePath *
get_next_any_random_track (PraghaPlaylist *playlist)
{
	gint rnd;
	GtkTreePath *path = NULL, *rpath;

	rpath = gtk_tree_row_reference_get_path (playlist->curr_rand_ref);
	do {
		rnd = g_rand_int_range (playlist->rand,
		                        0,
		                        playlist->no_tracks);
		path = get_nth_track (playlist, rnd);
	} while (!gtk_tree_path_compare(rpath, path) &&
	         (playlist->no_tracks > 1));

	gtk_tree_path_free(rpath);

	if (!path) {
		g_printerr("No track at position : %d\n", rnd);
		return NULL;
	}

	return path;
}

/* Return path of next sequential track */

static GtkTreePath *
get_next_sequential_track (PraghaPlaylist *playlist)
{
	GtkTreeIter iter;
	GtkTreePath *path = NULL;
	gboolean ret = FALSE;

	ret = gtk_tree_model_get_iter_first (playlist->model, &iter);

	/* If no tracks, return NULL.
	   If current track has been removed from the playlist,
	   return the first track. */

	if (!playlist->curr_seq_ref && !ret)
		return NULL;
	else if (!playlist->curr_seq_ref && ret) {
		path = gtk_tree_model_get_path (playlist->model, &iter);
		return path;
	}

	path = gtk_tree_row_reference_get_path (playlist->curr_seq_ref);
	gtk_tree_model_get_iter (playlist->model, &iter, path);

	if (!gtk_tree_model_iter_next (playlist->model, &iter)) {
		gtk_tree_path_free(path);
		path = NULL;
	}
	else {
		gtk_tree_path_free(path);
		path = gtk_tree_model_get_path (playlist->model, &iter);
	}

	return path;
}

/* Return path of next track in the list cstate->rand_track_refs */
/* This is called when the user clicks 'next' after one/more 'prev(s)' */

static GtkTreePath *
get_next_random_ref_track (PraghaPlaylist *playlist)
{
	GtkTreePath *path = NULL;
	GList *i, *j;

	i = g_list_find (playlist->rand_track_refs, playlist->curr_rand_ref);
	if (i) {
		j = g_list_next(i);
		if (j)
			path = gtk_tree_row_reference_get_path(j->data);
	}
	return path;
}

/* Return path of the node previous to the current track from
   cstate->rand_track_refs */

static GtkTreePath *
get_prev_random_track (PraghaPlaylist *playlist)
{
	GtkTreePath *path = NULL;
	GList *i, *j;

	if (!playlist->rand_track_refs)
		return NULL;

	i = g_list_find (playlist->rand_track_refs, playlist->curr_rand_ref);
	if (i) {
		j = g_list_previous(i);
		if (j)
			path = gtk_tree_row_reference_get_path(j->data);
	}

	return path;
}

/* Return path of the previous sequential track */

static GtkTreePath *
get_prev_sequential_track (PraghaPlaylist *playlist)
{
	GtkTreeIter iter;
	GtkTreePath *path = NULL;

	if (!playlist->curr_seq_ref)
		return NULL;

	path = gtk_tree_row_reference_get_path (playlist->curr_seq_ref);
	gtk_tree_model_get_iter (playlist->model, &iter, path);

	if (!gtk_tree_path_prev(path)) {
		gtk_tree_path_free(path);
		path = NULL;
	}

	return path;
}

/* Remove all nodes and free the list */

static void
clear_rand_track_refs (PraghaPlaylist *playlist)
{
	if (playlist->rand_track_refs) {
		g_list_free_full (playlist->rand_track_refs, (GDestroyNotify) gtk_tree_row_reference_free);
		playlist->rand_track_refs = NULL;
	}
	playlist->curr_rand_ref = NULL;
}

/* Remove all nodes and free the list */

static void
clear_queue_track_refs (PraghaPlaylist *playlist)
{
	if (playlist->queue_track_refs) {
		g_slist_free_full (playlist->queue_track_refs, (GDestroyNotify) gtk_tree_row_reference_free);
		playlist->queue_track_refs = NULL;
	}
}

/* Comparison function for column names */

static gint
compare_playlist_column_name(gconstpointer a, gconstpointer b)
{
	const gchar *e1 = a;
	const gchar *e2 = b;

	return g_ascii_strcasecmp(e1, e2);
}

/* Function to add/delete columns from preferences */

static void
modify_current_playlist_columns(PraghaPlaylist* cplaylist,
				const gchar *col_name,
				gboolean state)
{
	gboolean pref_present;
	GSList *element;

	if (!col_name) {
		g_warning("Invalid column name");
		return;
	}

	pref_present = is_present_str_list(col_name, cplaylist->columns);

	/* Already in preferences */

	if (pref_present && state) {
		return;
	}

	/* Remove from preferences */

	else if (pref_present && !state) {
		element = g_slist_find_custom(cplaylist->columns,
				      col_name, compare_playlist_column_name);
		if (element) {
			g_free(element->data);
			cplaylist->columns =
				g_slist_delete_link(cplaylist->columns,
						    element);
		}
		else
			g_warning("Column : %s not found in preferences",
				  col_name);
	}

	/* Add to preferences */
 
	else if (!pref_present && state) {
		 cplaylist->columns =
			g_slist_append(cplaylist->columns,
				       g_strdup(col_name));
	}
}

/* Function to jump to track on current playlist */

static void
pragha_playlist_select_path (PraghaPlaylist *playlist, GtkTreePath *path, gboolean center)
{
	GtkTreeSelection *selection;
	GdkRectangle vrect, crect;
	gint cx, cy, cnt_selected;

	if (!path)
		return;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(playlist->view));
	cnt_selected = gtk_tree_selection_count_selected_rows (selection);

	if (cnt_selected > 1)
		return;

	gtk_tree_selection_unselect_all (selection);
	gtk_tree_selection_select_path (GTK_TREE_SELECTION (selection), path);

	gtk_tree_view_set_cursor (GTK_TREE_VIEW(playlist->view),
	                          path, NULL, FALSE);

	gtk_tree_view_get_visible_rect (GTK_TREE_VIEW(playlist->view), &vrect);
	gtk_tree_view_get_cell_area (GTK_TREE_VIEW(playlist->view), path, NULL, &crect);

	gtk_tree_view_convert_widget_to_tree_coords (GTK_TREE_VIEW(playlist->view), crect.x, crect.y, &cx, &cy);

	if (pragha_preferences_get_shuffle(playlist->preferences) || center) {
		if ((cy < vrect.y) || (cy + crect.height > vrect.y + vrect.height)) {
			gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(playlist->view),
			                              path, NULL, TRUE, 0.5, 0.0);
		}
	}
	else {
		if (cy < vrect.y) {
			gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(playlist->view),
			                              path, NULL, TRUE, 0.0, 0.0);
		}
		else if (cy + crect.height > vrect.y + vrect.height) {
			gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(playlist->view),
			                              path, NULL, TRUE, 1.0, 0.0);
		}
	}
}

/* Select the song numbered according to the position in the playlist */

void select_numered_path_of_current_playlist(PraghaPlaylist *cplaylist, gint path_number, gboolean center)
{
	GtkTreePath *path = NULL;

	path = get_nth_track (cplaylist, path_number);

	pragha_playlist_select_path (cplaylist, path, center);

	gtk_tree_path_free(path);
}

/*************************/
/* General playlist mgmt */
/*************************/

static void
pragha_playlist_update_playback_sequence (PraghaPlaylist *playlist, PraghaUpdateAction update_action, GtkTreePath *path)
{
	GtkTreeRowReference *rand_ref;
	GtkTreePath *opath = NULL;
	gboolean shuffle = FALSE;

	CDEBUG(DBG_VERBOSE, "Update the state from current playlist");

	opath = get_current_track (playlist);
	if (opath) {
		pragha_playlist_update_track_state (playlist, opath, ST_STOPPED);
		gtk_tree_path_free (opath);
	}

	if (playlist->track_error) {
		g_error_free (playlist->track_error);
		playlist->track_error = NULL;
	}

	/* Append the new reference to the list of played track references
	   to retrace the sequence */

	shuffle = pragha_preferences_get_shuffle (playlist->preferences);

	if (!shuffle) {
		gtk_tree_row_reference_free(playlist->curr_seq_ref);
		playlist->curr_seq_ref = gtk_tree_row_reference_new (playlist->model, path);
	}

	if (shuffle) {
		switch (update_action) {
			/* If 'Prev', get the previous node from the track references */
			case PLAYLIST_PREV:
				if (playlist->curr_rand_ref) {
				    playlist->curr_rand_ref =
						get_rand_ref_prev (playlist, playlist->curr_rand_ref);
				}
			break;

			/* If 'Next', get the next node from the track references */
			/* Do this only if the current track and the
			   last node don't match */
			case PLAYLIST_NEXT:
				if (playlist->curr_rand_ref) {
					rand_ref = g_list_last(playlist->rand_track_refs)->data;
					if (playlist->curr_rand_ref != rand_ref) {
						playlist->curr_rand_ref =
							get_rand_ref_next (playlist, playlist->curr_rand_ref);
						break;
					}
				}

			/* Append a new ref of the track to the track references */
			case PLAYLIST_CURR:
				rand_ref = gtk_tree_row_reference_new (playlist->model, path);
				playlist->rand_track_refs =
					g_list_append (playlist->rand_track_refs, rand_ref);
				playlist->curr_rand_ref = rand_ref;
				break;
			default:
				break;
		}
	}

	/* Mark the track as dirty */

	pragha_playlist_set_dirty_track (playlist, path);

	pragha_playlist_update_track_state (playlist, path, ST_PLAYING);
	pragha_playlist_select_path (playlist, path, shuffle);
}

void update_current_playlist_view_track(PraghaPlaylist *cplaylist, PraghaBackend *backend)
{
	GtkTreePath *path;
	PraghaBackendState state;

	path = get_current_track (cplaylist);
	if(path) {
		state = pragha_backend_get_state (backend);
		pragha_playlist_update_track_state (cplaylist, path, state);
		gtk_tree_path_free(path);
	}
}

/* Return musicobject of the given path */

PraghaMusicobject *
current_playlist_mobj_at_path(GtkTreePath *path,
                              PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model = cplaylist->model;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;

	if (gtk_tree_model_get_iter(model, &iter, path))
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

	return mobj;
}

/* Return path of musicobject already in tree */

GtkTreePath*
current_playlist_path_at_mobj (PraghaMusicobject *mobj,
                               PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model = cplaylist->model;
	GtkTreeIter iter;
	PraghaMusicobject *ptr = NULL;
	gboolean ret;

	ret = gtk_tree_model_get_iter_first (model, &iter);
	while(ret) {
		gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &ptr, -1);
		if (ptr == mobj) {
			return gtk_tree_model_get_path(model, &iter);
		}
		ret = gtk_tree_model_iter_next(model, &iter);
	}

	return NULL;
}

/* Reset random rand_refs and appends given ref */

static void
reset_rand_track_refs (PraghaPlaylist      *playlist,
                       GtkTreeRowReference *ref)
{
	GtkTreePath *path;

	/* All songs can be played. */
	pragha_playlist_clear_dirty_all (playlist);
	clear_rand_track_refs (playlist);

	/* Set the current song as played. */
	playlist->rand_track_refs =
		g_list_append (playlist->rand_track_refs, ref);
	playlist->curr_rand_ref = ref;

	path = gtk_tree_row_reference_get_path (ref);
	pragha_playlist_set_dirty_track (playlist, path);

	gtk_tree_path_free (path);
}

static void
pragha_playlist_set_first_rand_ref (PraghaPlaylist *playlist,
                                    GtkTreePath    *path)
{
	GtkTreeRowReference *ref;

	ref = gtk_tree_row_reference_new (playlist->model, path);
	reset_rand_track_refs (playlist, ref);
}

/* Return the path of the selected track */

static GtkTreePath *
get_selected_track (PraghaPlaylist *cplaylist)
{
	GtkTreeSelection *selection;
	GtkTreePath *path = NULL;
	gint cnt_selected = 0;
	GList *list;

	if(cplaylist->changing)
		return NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	cnt_selected = gtk_tree_selection_count_selected_rows (selection);

	if (!cnt_selected)
		return NULL;

	else if (cnt_selected == 1) {
		list = gtk_tree_selection_get_selected_rows(selection, NULL);
		if (list) {
			path = list->data;
			g_list_free(list);
		}
	}
	else if (cnt_selected > 1)
		g_message("Selected multiple");

	return path;
}

/* Return the path of the Actual track playing */

static GtkTreePath *
get_current_track (PraghaPlaylist *cplaylist)
{
	GtkTreePath *path=NULL;
	gboolean shuffle = pragha_preferences_get_shuffle(cplaylist->preferences);

	if (shuffle && cplaylist->curr_rand_ref)
		path = gtk_tree_row_reference_get_path(cplaylist->curr_rand_ref);
	else if (!shuffle && cplaylist->curr_seq_ref)
		path = gtk_tree_row_reference_get_path(cplaylist->curr_seq_ref);

	return path;
}

/* Dequeue selected rows from current playlist */

static void
pragha_playlist_dequeue_handler (PraghaPlaylist *cplaylist)
{
	GtkTreeSelection *selection;
	GList *list;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, NULL);

	g_list_foreach (list, (GFunc) delete_queue_track_refs, cplaylist);
	requeue_track_refs(cplaylist);
	g_list_free_full (list, (GDestroyNotify) gtk_tree_path_free);
}


/* Queue selected rows from current playlist */

static void
pragha_playlist_queue_handler (PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeRowReference *ref;
	GList *list, *l;
	gboolean is_queue = FALSE;
	GtkTreeIter iter;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	l= list;
	while (l) {
		path = l->data;
		if (gtk_tree_model_get_iter(model, &iter, path)) {
			gtk_tree_model_get(model, &iter, P_BUBBLE, &is_queue, -1);
			if(!is_queue) {
				ref = gtk_tree_row_reference_new(model, path);
				cplaylist->queue_track_refs = g_slist_append(cplaylist->queue_track_refs, ref);
			}
		}
		gtk_tree_path_free(path);
		l = l->next;
	}
	requeue_track_refs(cplaylist);
	g_list_free (list);
}

/* Toglle queue state of selection on current playlist. */

void toggle_queue_selected_current_playlist (PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreeRowReference *ref;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean is_queue = FALSE;
	GList *list;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	while (list) {
		path = list->data;
		if (gtk_tree_model_get_iter(model, &iter, path)) {
			gtk_tree_model_get(model, &iter, P_BUBBLE, &is_queue, -1);
			if(is_queue)
				delete_queue_track_refs(path, cplaylist);
			else {
				ref = gtk_tree_row_reference_new(model, path);
				cplaylist->queue_track_refs = g_slist_append(cplaylist->queue_track_refs, ref);
			}
		}
		gtk_tree_path_free(path);
		list = list->next;
	}
	requeue_track_refs(cplaylist);
	g_list_free (list);
}

/* Remove selected rows from current playlist */

void
pragha_playlist_remove_selection (PraghaPlaylist *playlist)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreePath *path, *next;
	GtkTreeIter iter;
	GList *list = NULL, *i = NULL;
	PraghaMusicobject *mobj = NULL;
	gboolean played = FALSE;

	set_watch_cursor (GTK_WIDGET(playlist));

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(playlist->view));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		/* Select the next row to the last selected */

		gtk_tree_view_get_cursor (GTK_TREE_VIEW(playlist->view), &next, NULL);
		do {
			if(gtk_tree_selection_path_is_selected(selection, next) == FALSE)
				break;

			gtk_tree_path_next(next);
		}
		while(next != NULL);
		gtk_tree_view_set_cursor (GTK_TREE_VIEW(playlist->view), next, NULL, FALSE);
		gtk_tree_path_free (next);

		/* Get references from the paths and store them in the 'data'
		   portion of the list elements.
		   This idea was inspired by code from 'claws-mail' */

		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			ref = gtk_tree_row_reference_new(model, path);
			i->data = ref;
			gtk_tree_path_free(path);
		}
		
		/* Now build iterators from the references and delete
		   them from the store */

		for (i=list; i != NULL; i = i->next) {
			ref = i->data;
			path = gtk_tree_row_reference_get_path(ref);
			delete_rand_track_refs (path, playlist);
			delete_queue_track_refs (path, playlist);
			test_clear_curr_seq_ref (path, playlist);

			if (gtk_tree_model_get_iter(model, &iter, path)) {
				gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
				g_object_unref(mobj);
				gtk_tree_model_get(model, &iter, P_PLAYED, &played, -1);
				gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
				playlist->no_tracks--;
				if (!played)
					playlist->unplayed_tracks--;
			}
			gtk_tree_path_free(path);
			gtk_tree_row_reference_free(ref);
		}

		g_list_free(list);
	}

	requeue_track_refs (playlist);

	remove_watch_cursor (GTK_WIDGET(playlist));

	g_signal_emit (playlist, signals[PLAYLIST_CHANGED], 0);
}

/* Crop selected rows from current playlist */

void
pragha_playlist_crop_selection (PraghaPlaylist *playlist)
{
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	gboolean ret, played = FALSE;
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreePath *path;
	GSList *to_delete = NULL, *i = NULL;

	set_watch_cursor (GTK_WIDGET(playlist));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(playlist->view));

	/* At least one row must be selected */

	if (!gtk_tree_selection_count_selected_rows(selection))
		return;

	/* Get a reference to all the nodes that are _not_ selected */

	ret = gtk_tree_model_get_iter_first (playlist->model, &iter);

	while (ret) {
		if (gtk_tree_selection_iter_is_selected(selection, &iter) == FALSE) {
			path = gtk_tree_model_get_path (playlist->model, &iter);
			ref = gtk_tree_row_reference_new (playlist->model, path);
			to_delete = g_slist_prepend(to_delete, ref);
			gtk_tree_path_free(path);
		}
		ret = gtk_tree_model_iter_next (playlist->model, &iter);
	}

	/* Delete the referenced nodes */

	pragha_playlist_set_changing (playlist, TRUE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(playlist->view), NULL);

	for (i=to_delete; i != NULL; i = i->next) {
		ref = i->data;
		path = gtk_tree_row_reference_get_path(ref);
		delete_rand_track_refs (path, playlist);
		delete_queue_track_refs (path, playlist);
		test_clear_curr_seq_ref (path, playlist);

		if (gtk_tree_model_get_iter (playlist->model, &iter, path)) {
			gtk_tree_model_get (playlist->model, &iter, P_MOBJ_PTR, &mobj, -1);
			g_object_unref(mobj);
			gtk_tree_model_get (playlist->model, &iter, P_PLAYED, &played, -1);
			gtk_list_store_remove (GTK_LIST_STORE(playlist->model), &iter);
			playlist->no_tracks--;
			if (!played)
				playlist->unplayed_tracks--;

			/* Have to give control to GTK periodically ... */
			pragha_process_gtk_events ();
		}
		gtk_tree_path_free(path);
		gtk_tree_row_reference_free(ref);
	}

	gtk_tree_view_set_model (GTK_TREE_VIEW(playlist->view), playlist->model);
	pragha_playlist_set_changing (playlist, FALSE);

	requeue_track_refs (playlist);

	remove_watch_cursor (GTK_WIDGET(playlist));
	g_signal_emit (playlist, signals[PLAYLIST_CHANGED], 0);

	g_slist_free(to_delete);
}


void
pragha_playlist_crop_music_type (PraghaPlaylist *playlist, PraghaMusicType music_type)
{
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	gboolean ret, played = FALSE;
	GtkTreeRowReference *ref;
	GtkTreePath *path;
	GSList *to_delete = NULL, *i = NULL;

	set_watch_cursor (GTK_WIDGET(playlist));

	/* Get a reference to all the nodes that are _not_ selected */

	ret = gtk_tree_model_get_iter_first (playlist->model, &iter);
	while (ret) {
		gtk_tree_model_get (playlist->model, &iter, P_MOBJ_PTR, &mobj, -1);
		if (music_type == pragha_musicobject_get_file_type(mobj)) {
			path = gtk_tree_model_get_path (playlist->model, &iter);
			ref = gtk_tree_row_reference_new (playlist->model, path);
			to_delete = g_slist_prepend(to_delete, ref);
			gtk_tree_path_free(path);
		}
		ret = gtk_tree_model_iter_next (playlist->model, &iter);
	}

	/* Delete the referenced nodes */

	pragha_playlist_set_changing (playlist, TRUE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(playlist->view), NULL);

	for (i=to_delete; i != NULL; i = i->next) {
		ref = i->data;
		path = gtk_tree_row_reference_get_path(ref);
		delete_rand_track_refs (path, playlist);
		delete_queue_track_refs (path, playlist);
		test_clear_curr_seq_ref (path, playlist);

		if (gtk_tree_model_get_iter (playlist->model, &iter, path)) {
			gtk_tree_model_get (playlist->model, &iter, P_MOBJ_PTR, &mobj, -1);
			g_object_unref(mobj);
			gtk_tree_model_get (playlist->model, &iter, P_PLAYED, &played, -1);
			gtk_list_store_remove (GTK_LIST_STORE(playlist->model), &iter);
			playlist->no_tracks--;
			if (!played)
				playlist->unplayed_tracks--;

			/* Have to give control to GTK periodically ... */
			pragha_process_gtk_events ();
		}
		gtk_tree_path_free(path);
		gtk_tree_row_reference_free(ref);
	}

	gtk_tree_view_set_model (GTK_TREE_VIEW(playlist->view), playlist->model);
	pragha_playlist_set_changing (playlist, FALSE);

	requeue_track_refs (playlist);

	remove_watch_cursor (GTK_WIDGET(playlist));
	g_signal_emit (playlist, signals[PLAYLIST_CHANGED], 0);

	g_slist_free(to_delete);
}

/* Handle key press on current playlist view.
 * Based on Totem Code*/

static gint
current_playlist_key_press (GtkWidget *win, GdkEventKey *event, PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeIter iter;
	GList *list;
	gint n_select = 0;
	gboolean is_queue = FALSE;

	/* Special case some shortcuts 
	if (event->state != 0) {
		if ((event->state & GDK_CONTROL_MASK)
		    && event->keyval == GDK_KEY_a) {
			gtk_tree_selection_select_all
				(playlist->priv->selection);
			return TRUE;
		}
	}*/
	/* If we have modifiers, and either Ctrl, Mod1 (Alt), or any
	 * of Mod3 to Mod5 (Mod2 is num-lock...) are pressed, we
	 * let Gtk+ handle the key */
	if (event->state != 0
			&& ((event->state & GDK_CONTROL_MASK)
			|| (event->state & GDK_MOD1_MASK)
			|| (event->state & GDK_MOD3_MASK)
			|| (event->state & GDK_MOD4_MASK)
			|| (event->state & GDK_MOD5_MASK)))
		return FALSE;
	if (event->keyval == GDK_KEY_Delete){
		pragha_playlist_remove_selection(cplaylist);
		return TRUE;
	}
	else if(event->keyval == GDK_KEY_q || event->keyval == GDK_KEY_Q){
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
		n_select = gtk_tree_selection_count_selected_rows(selection);

		if(n_select==1){
			list = gtk_tree_selection_get_selected_rows(selection, &model);
			if (gtk_tree_model_get_iter(model, &iter, list->data)){
				gtk_tree_model_get(model, &iter, P_BUBBLE, &is_queue, -1);
				if(is_queue)
					delete_queue_track_refs(list->data, cplaylist);
				else{
					ref = gtk_tree_row_reference_new(model, list->data);
					cplaylist->queue_track_refs = g_slist_append(cplaylist->queue_track_refs, ref);
				}
				requeue_track_refs(cplaylist);
			}
			gtk_tree_path_free(list->data);
			g_list_free (list);
		}
		return TRUE;
	}
	return FALSE;
}

void
pragha_playlist_remove_all (PraghaPlaylist *playlist)
{
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	gboolean ret;

	set_watch_cursor (GTK_WIDGET(playlist));

	clear_rand_track_refs(playlist);
	clear_queue_track_refs(playlist);
	clear_curr_seq_ref(playlist);

	ret = gtk_tree_model_get_iter_first (playlist->model, &iter);

	while (ret) {
		gtk_tree_model_get (playlist->model, &iter, P_MOBJ_PTR, &mobj, -1);
		g_object_unref(mobj);
		ret = gtk_tree_model_iter_next (playlist->model, &iter);
	}

	gtk_list_store_clear (GTK_LIST_STORE(playlist->model));

	remove_watch_cursor (GTK_WIDGET(playlist));

	playlist->no_tracks = 0;
	playlist->unplayed_tracks = 0;

	g_signal_emit (playlist, signals[PLAYLIST_CHANGED], 0);
}

/* Update a list of references in the current playlist */

static gboolean
pragha_playlist_update_ref_list_change_tags(PraghaPlaylist *cplaylist, GList *list, gint changed, PraghaMusicobject *nmobj)
{
	PraghaMusicobject *mobj = NULL;
	PraghaTagger *tagger;
	GtkTreeRowReference *ref;
	GtkTreePath *path = NULL, *apath;
	GtkTreeIter iter;
	GList *i;
	gchar *ch_track_no = NULL, *ch_year = NULL, *ch_title = NULL;
	gboolean update_current_song = FALSE;

	tagger = pragha_tagger_new();

	apath = get_current_track (cplaylist);

	for (i = list; i != NULL; i = i->next) {
		ref = i->data;
		path = gtk_tree_row_reference_get_path(ref);

		if (G_LIKELY(gtk_tree_model_get_iter(cplaylist->model, &iter, path))) {
			gtk_tree_model_get(cplaylist->model, &iter, P_MOBJ_PTR, &mobj, -1);

			if (changed & TAG_TNO_CHANGED) {
				pragha_musicobject_set_track_no(mobj, pragha_musicobject_get_track_no(nmobj));
				ch_track_no = g_strdup_printf("%d", pragha_musicobject_get_track_no(nmobj));
				gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_TRACK_NO, ch_track_no, -1);
				g_free(ch_track_no);
			}
			if (changed & TAG_TITLE_CHANGED) {
				const gchar *title = pragha_musicobject_get_title(nmobj);
				pragha_musicobject_set_title(mobj, title);
				ch_title = string_is_not_empty(title) ? g_strdup(title) : get_display_name(mobj);
				gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_TITLE, ch_title, -1);
				g_free(ch_title);
			}
			if (changed & TAG_ARTIST_CHANGED) {
				pragha_musicobject_set_artist(mobj, pragha_musicobject_get_artist(nmobj));
				gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_ARTIST, pragha_musicobject_get_artist(mobj),-1);
			}
			if (changed & TAG_ALBUM_CHANGED) {
				pragha_musicobject_set_album(mobj, pragha_musicobject_get_album(nmobj));
				gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_ALBUM, pragha_musicobject_get_album(mobj),-1);
			}
			if (changed & TAG_GENRE_CHANGED) {
				pragha_musicobject_set_genre(mobj, pragha_musicobject_get_genre(nmobj));
				gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_GENRE, pragha_musicobject_get_genre(mobj),-1);
			}
			if (changed & TAG_YEAR_CHANGED) {
				pragha_musicobject_set_year(mobj, pragha_musicobject_get_year(nmobj));
				ch_year = g_strdup_printf("%d", pragha_musicobject_get_year(mobj));
				gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_YEAR, ch_year, -1);
				g_free(ch_year);
			}
			if (changed & TAG_COMMENT_CHANGED) {
				pragha_musicobject_set_comment(mobj, pragha_musicobject_get_comment(nmobj));
				gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_COMMENT, pragha_musicobject_get_comment(mobj),-1);
			}

			pragha_tagger_add_file (tagger, pragha_musicobject_get_file(mobj));

			if(apath && gtk_tree_path_compare(path, apath) == 0)
				update_current_song = TRUE;
			gtk_tree_path_free(path);
		}
	}
	pragha_tagger_set_changes(tagger, nmobj, changed);
	pragha_tagger_apply_changes (tagger);
	g_object_unref(tagger);

	return update_current_song;
}

static void
pragha_playlist_change_ref_list_tags (PraghaPlaylist *playlist, GList *rlist, gint changed, PraghaMusicobject *mobj)
{
	GtkWidget *toplevel;
	PraghaMusicobject *tmobj = NULL;
	gboolean need_update;

	toplevel = gtk_widget_get_toplevel (GTK_WIDGET(playlist));

	/* Check if user is trying to set the same title ot track no for multiple tracks */
	if (g_list_length(rlist) > 1) {
		if (changed & TAG_TNO_CHANGED) {
			if (!confirm_tno_multiple_tracks(pragha_musicobject_get_track_no(mobj), toplevel))
				return;
		}
		if (changed & TAG_TITLE_CHANGED) {
			if (!confirm_title_multiple_tracks(pragha_musicobject_get_title(mobj), toplevel))
				return;
		}
	}

	clear_sort_current_playlist_cb (NULL, playlist);
	pragha_playlist_set_changing (playlist, TRUE);

	rlist = pragha_playlist_get_selection_ref_list (playlist);
	tmobj = pragha_musicobject_dup (mobj);

	/* Update the view and save tag change on db and disk.*/
	need_update = pragha_playlist_update_ref_list_change_tags (playlist, rlist, changed, tmobj);
	pragha_playlist_set_changing (playlist, FALSE);

	if (need_update)
		g_signal_emit (playlist, signals[PLAYLIST_CHANGE_TAGS], 0, changed, mobj);

	g_object_unref(tmobj);
}

void
pragha_playlist_update_current_track(PraghaPlaylist *cplaylist, gint changed, PraghaMusicobject *nmobj)
{
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	gchar *ch_track_no = NULL, *ch_year = NULL, *ch_title = NULL;

	path = get_current_track (cplaylist);

	if(!path)
		return;

	if (G_LIKELY(gtk_tree_model_get_iter(cplaylist->model, &iter, path))) {
		gtk_tree_model_get(cplaylist->model, &iter, P_MOBJ_PTR, &mobj, -1);

		if (changed & TAG_TNO_CHANGED) {
			pragha_musicobject_set_track_no(mobj, pragha_musicobject_get_track_no(nmobj));
			ch_track_no = g_strdup_printf("%d", pragha_musicobject_get_track_no(nmobj));
			gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_TRACK_NO, ch_track_no, -1);
			g_free(ch_track_no);
		}
		if (changed & TAG_TITLE_CHANGED) {
			const gchar *title = pragha_musicobject_get_title(mobj);
			pragha_musicobject_set_title(mobj, pragha_musicobject_get_title(nmobj));
			ch_title = string_is_not_empty(title) ? g_strdup(title) : get_display_name(mobj);
			gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_TITLE, ch_title, -1);
			g_free(ch_title);
		}
		if (changed & TAG_ARTIST_CHANGED) {
			pragha_musicobject_set_artist(mobj, pragha_musicobject_get_artist(nmobj));
			gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_ARTIST, pragha_musicobject_get_artist(mobj),-1);
		}
		if (changed & TAG_ALBUM_CHANGED) {
			pragha_musicobject_set_title(mobj, pragha_musicobject_get_title(nmobj));
			gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_ALBUM, pragha_musicobject_get_album(mobj),-1);
		}
		if (changed & TAG_GENRE_CHANGED) {
			pragha_musicobject_set_genre(mobj, pragha_musicobject_get_genre(nmobj));
			gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_GENRE, pragha_musicobject_get_genre(mobj),-1);
		}
		if (changed & TAG_YEAR_CHANGED) {
			pragha_musicobject_set_year(mobj, pragha_musicobject_get_year(nmobj));
			ch_year = g_strdup_printf("%d", pragha_musicobject_get_year(mobj));
			gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_YEAR, ch_year, -1);
			g_free(ch_year);
		}
		if (changed & TAG_COMMENT_CHANGED) {
			pragha_musicobject_set_comment(mobj, pragha_musicobject_get_comment(nmobj));
			gtk_list_store_set(GTK_LIST_STORE(cplaylist->model), &iter, P_COMMENT, pragha_musicobject_get_comment(mobj),-1);
		}
	}
	gtk_tree_path_free(path);
}

/* Insert a track to the current playlist */

static void
insert_current_playlist(PraghaPlaylist *cplaylist,
			PraghaMusicobject *mobj,
			GtkTreeViewDropPosition droppos,
			GtkTreeIter *pos)
{
	GtkTreeIter iter;
	const gchar *title, *artist, *album, *genre, *comment;
	gint track_no, year, length, bitrate;
	gchar *ch_length = NULL, *ch_track_no = NULL, *ch_year = NULL, *ch_bitrate = NULL, *ch_filename = NULL;
	GtkTreeModel *model = cplaylist->model;

	if (!mobj) {
		g_warning("Dangling entry in current playlist");
		return;
	}

	title = pragha_musicobject_get_title(mobj);
	artist = pragha_musicobject_get_artist(mobj);
	album = pragha_musicobject_get_album(mobj);
	genre = pragha_musicobject_get_genre(mobj);
	track_no = pragha_musicobject_get_track_no(mobj);
	year = pragha_musicobject_get_year(mobj);
	comment = pragha_musicobject_get_comment(mobj);
	length = pragha_musicobject_get_length(mobj);
	bitrate = pragha_musicobject_get_bitrate(mobj);

	if(track_no > 0)
		ch_track_no = g_strdup_printf("%d", track_no);
	if(year > 0)
		ch_year = g_strdup_printf("%d", year);
	if(length > 0)
		ch_length = convert_length_str(length);
	if(bitrate)
		ch_bitrate = g_strdup_printf("%d", bitrate);

	ch_filename = get_display_name(mobj);

	if (droppos == GTK_TREE_VIEW_DROP_AFTER)
		gtk_list_store_insert_after(GTK_LIST_STORE(model), &iter, pos);
	else
		gtk_list_store_insert_before(GTK_LIST_STORE(model), &iter, pos);

	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
	                   P_MOBJ_PTR, mobj,
	                   P_QUEUE, NULL,
	                   P_BUBBLE, FALSE,
	                   P_STATUS_PIXBUF, NULL,
	                   P_TRACK_NO, ch_track_no,
	                   P_TITLE, string_is_not_empty(title) ? title : ch_filename,
	                   P_ARTIST, artist,
	                   P_ALBUM, album,
	                   P_GENRE, genre,
	                   P_BITRATE, ch_bitrate,
	                   P_YEAR, ch_year,
	                   P_COMMENT, comment,
	                   P_LENGTH, ch_length,
	                   P_FILENAME, ch_filename,
	                   P_PLAYED, FALSE,
	                   -1);

	/* Increment global count of tracks */

	cplaylist->no_tracks++;
	cplaylist->unplayed_tracks++;

	/* Have to give control to GTK periodically ... */
	pragha_process_gtk_events ();

	g_free(ch_length);
	g_free(ch_track_no);
	g_free(ch_year);
	g_free(ch_bitrate);
	g_free(ch_filename);
}

/* Append a track to the current playlist */

static void
append_current_playlist_ex(PraghaPlaylist *cplaylist, PraghaMusicobject *mobj, GtkTreePath **path)
{
	GtkTreeIter iter;
	const gchar *title, *artist, *album, *genre, *comment;
	gint track_no, year, length, bitrate;
	gchar *ch_length = NULL, *ch_track_no = NULL, *ch_year = NULL, *ch_bitrate = NULL, *ch_filename = NULL;
	GtkTreeModel *model = cplaylist->model;

	if (!mobj) {
		g_warning("Dangling entry in current playlist");
		return;
	}

	title = pragha_musicobject_get_title(mobj);
	artist = pragha_musicobject_get_artist(mobj);
	album = pragha_musicobject_get_album(mobj);
	genre = pragha_musicobject_get_genre(mobj);
	track_no = pragha_musicobject_get_track_no(mobj);
	year = pragha_musicobject_get_year(mobj);
	comment = pragha_musicobject_get_comment(mobj);
	length = pragha_musicobject_get_length(mobj);
	bitrate = pragha_musicobject_get_bitrate(mobj);

	if(track_no > 0)
		ch_track_no = g_strdup_printf("%d", track_no);
	if(year > 0)
		ch_year = g_strdup_printf("%d", year);
	if(length > 0)
		ch_length = convert_length_str(length);
	if(bitrate)
		ch_bitrate = g_strdup_printf("%d", bitrate);

	ch_filename = get_display_name(mobj);

	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
	                   P_MOBJ_PTR, mobj,
	                   P_QUEUE, NULL,
	                   P_BUBBLE, FALSE,
	                   P_STATUS_PIXBUF, NULL,
	                   P_TRACK_NO, ch_track_no,
	                   P_TITLE, string_is_not_empty(title) ? title : ch_filename,
	                   P_ARTIST, artist,
	                   P_ALBUM, album,
	                   P_GENRE, genre,
	                   P_BITRATE, ch_bitrate,
	                   P_YEAR, ch_year,
	                   P_COMMENT, comment,
	                   P_LENGTH, ch_length,
	                   P_FILENAME, ch_filename,
	                   P_PLAYED, FALSE,
	                   -1);

	/* Increment global count of tracks */

	cplaylist->no_tracks++;
	cplaylist->unplayed_tracks++;

	if(path)
		*path = gtk_tree_model_get_path(model, &iter);

	g_free(ch_length);
	g_free(ch_track_no);
	g_free(ch_year);
	g_free(ch_bitrate);
	g_free(ch_filename);
}

static void
append_current_playlist (PraghaPlaylist *cplaylist, PraghaMusicobject *mobj)
{
	append_current_playlist_ex(cplaylist, mobj, NULL);
}

void
pragha_playlist_append_single_song(PraghaPlaylist *cplaylist, PraghaMusicobject *mobj)
{
	append_current_playlist(cplaylist, mobj);

	g_signal_emit (cplaylist, signals[PLAYLIST_CHANGED], 0);
}

void
pragha_playlist_append_mobj_and_play(PraghaPlaylist *cplaylist, PraghaMusicobject *mobj)
{
	GtkTreePath *path = NULL;

	append_current_playlist_ex(cplaylist, mobj, &path);

	if(path) {
		pragha_playlist_activate_path(cplaylist, path);

		g_signal_emit (cplaylist, signals[PLAYLIST_CHANGED], 0);

		gtk_tree_path_free(path);
	}
}

/* Insert a list of mobj to the current playlist. */

static void
pragha_playlist_insert_mobj_list(PraghaPlaylist *cplaylist,
				 GList *list,
				 GtkTreeViewDropPosition droppos,
				 GtkTreeIter *pos)
{
	PraghaMusicobject *mobj;
	GList *l;

	/* TODO: pragha_playlist_set_changing() should be set cursor automatically. */
	set_watch_cursor (GTK_WIDGET(cplaylist));
	pragha_playlist_set_changing(cplaylist, TRUE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cplaylist->view), NULL);

	for (l = list; l != NULL; l = l->next) {
		mobj = l->data;
		insert_current_playlist(cplaylist, mobj, droppos, pos);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(cplaylist->view), cplaylist->model);

	pragha_playlist_set_changing(cplaylist, FALSE);
	remove_watch_cursor (GTK_WIDGET(cplaylist));

	g_signal_emit (cplaylist, signals[PLAYLIST_CHANGED], 0);
}

/* Append a list of mobj to the current playlist */

void
pragha_playlist_append_mobj_list(PraghaPlaylist *cplaylist, GList *list)
{
	PraghaMusicobject *mobj;
	gint prev_tracks = 0;
	GtkSortType order;
	gint column;
	GList *l;

	prev_tracks = pragha_playlist_get_no_tracks(cplaylist);
	
	/* TODO: pragha_playlist_set_changing() should be set cursor automatically. */
	set_watch_cursor (GTK_WIDGET(cplaylist));
	pragha_playlist_set_changing(cplaylist, TRUE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cplaylist->view), NULL);

	for (l = list; l != NULL; l = l->next) {
		mobj = l->data;
		append_current_playlist(cplaylist, mobj);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(cplaylist->view), cplaylist->model);

	pragha_playlist_set_changing(cplaylist, FALSE);
	remove_watch_cursor (GTK_WIDGET(cplaylist));

	g_signal_emit (cplaylist, signals[PLAYLIST_CHANGED], 0);

	if(gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(cplaylist->model),
	                                        &column, &order))
		select_numered_path_of_current_playlist(cplaylist, prev_tracks, TRUE);
}

/* Test if the song is already in the mobj list */

gboolean
pragha_mobj_list_already_has_title_of_artist(GList *list,
					     const gchar *title,
					     const gchar *artist)
{
	PraghaMusicobject *mobj = NULL;
	GList *i;

	for (i = list; i != NULL; i = i->next) {
		mobj = i->data;
		if((0 == g_ascii_strcasecmp(pragha_musicobject_get_title(mobj), title)) &&
		   (0 == g_ascii_strcasecmp(pragha_musicobject_get_artist(mobj), artist)))
		   	return TRUE;
	}
	return FALSE;
}

/* Test if the song is already in the playlist.*/

gboolean
pragha_playlist_already_has_title_of_artist(PraghaPlaylist *cplaylist,
					    const gchar *title,
					    const gchar *artist)
{
	GtkTreeModel *model = cplaylist->model;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	gboolean ret;

	ret = gtk_tree_model_get_iter_first (model, &iter);
	while (ret) {
		gtk_tree_model_get (model, &iter, P_MOBJ_PTR, &mobj, -1);

		if((0 == g_ascii_strcasecmp(pragha_musicobject_get_title(mobj), title)) &&
		   (0 == g_ascii_strcasecmp(pragha_musicobject_get_artist(mobj), artist)))
		   	return TRUE;

		ret = gtk_tree_model_iter_next(model, &iter);
	}

	return FALSE;
}

/* Clear sort in the current playlist */

void clear_sort_current_playlist(GtkAction *action, PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model = cplaylist->model;

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
			     GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID,
			     GTK_SORT_ASCENDING);
}

/* Save selected tracks as a playlist */

void save_selected_playlist(GtkAction *action, PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model = cplaylist->model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	gchar *playlist;

	/* If current playlist is empty, return immediately. */

	if (!gtk_tree_model_get_iter_first(model, &iter)) {
		g_warning("Current playlist is empty");
		return;
	}

	/* If no tracks have been selected in the current playlist,
	   return immediately. */

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	if (!gtk_tree_selection_count_selected_rows(selection))
		return;

	playlist = get_playlist_name(SAVE_SELECTED, gtk_widget_get_toplevel(GTK_WIDGET(cplaylist)));

	if (playlist) {
		new_playlist(cplaylist, playlist, SAVE_SELECTED);
		pragha_database_change_playlists_done(cplaylist->cdbase);
		g_free(playlist);
	}
}

/* Save current playlist */

void save_current_playlist(GtkAction *action, PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model = cplaylist->model;
	GtkTreeIter iter;
	gchar *playlist = NULL;

	/* If current playlist is empty, return immediately. */

	if(pragha_playlist_is_changing(cplaylist))
		return;

	if (!gtk_tree_model_get_iter_first(model, &iter)) {
		g_warning("Current playlist is empty");
		return;
	}

	playlist = get_playlist_name(SAVE_COMPLETE, gtk_widget_get_toplevel(GTK_WIDGET(cplaylist)));
	if (playlist) {
		new_playlist(cplaylist, playlist, SAVE_COMPLETE);
		pragha_database_change_playlists_done(cplaylist->cdbase);
		g_free(playlist);
	}
}

void export_current_playlist(GtkAction *action, PraghaPlaylist *cplaylist)
{
	GtkTreeIter iter;

	if(pragha_playlist_is_changing(cplaylist))
		return;

	/* If current playlist change or is empty, return immediately. */

	if (!gtk_tree_model_get_iter_first(cplaylist->model, &iter)) {
		g_warning("Current playlist is empty");
		return;
	}

	export_playlist (cplaylist, SAVE_COMPLETE);
}

void export_selected_playlist(GtkAction *action, PraghaPlaylist *cplaylist)
{
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	/* If current playlist change or is empty, return immediately. */

	if(pragha_playlist_is_changing(cplaylist))
		return;

	if (!gtk_tree_model_get_iter_first(cplaylist->model, &iter)) {
		g_warning("Current playlist is empty");
		return;
	}

	/* If no tracks have been selected in the current playlist,
	   return immediately. */

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	if (!gtk_tree_selection_count_selected_rows(selection))
		return;

	export_playlist (cplaylist, SAVE_SELECTED);
}

/*******************/
/* Event Callbacks */
/*******************/

/* Handler for row double click / kboard enter */

static void
pragha_playlist_row_activated_cb (GtkTreeView *current_playlist,
                                  GtkTreePath *path,
                                  GtkTreeViewColumn *column,
                                  PraghaPlaylist *playlist)
{
	PraghaMusicobject *mobj = NULL;
	gboolean shuffle = FALSE;
	GtkTreeIter iter;

	gtk_tree_model_get_iter (playlist->model, &iter, path);
	gtk_tree_model_get (playlist->model, &iter, P_MOBJ_PTR, &mobj, -1);

	if (!mobj)
		return;

	shuffle = pragha_preferences_get_shuffle (playlist->preferences);
	if (shuffle) {
		if (!playlist->rand_track_refs) {
			clear_rand_track_refs (playlist);
			pragha_playlist_clear_dirty_all (playlist);
		}
		else {
			trim_down_rand_track_refs (playlist);
		}
	}

	/* Start playing new track */
	pragha_playlist_update_playback_sequence (playlist, PLAYLIST_NEXT, path);

	g_signal_emit (playlist, signals[PLAYLIST_SET_TRACK], 0, mobj);
}

static void
pragha_playlist_personalize_copy_tag_to_seleccion (PraghaPlaylist *playlist,
                                                   GtkTreeViewColumn *column,
                                                   GtkTreeIter *iter)
{
	GList *list = NULL;
	gint icolumn = 0;
	GtkAction *action = NULL;
	gchar *label = NULL;
	PraghaMusicobject *mobj = NULL;
	gint change = 0;

	gtk_tree_model_get (playlist->model, iter, P_MOBJ_PTR, &mobj, -1);

	/* Get the column clicked and set menu. */

	list = gtk_tree_view_get_columns (GTK_TREE_VIEW(playlist->view));
	icolumn = g_list_index(list, column);

	switch (icolumn) {
		case 1: {
			change = TAG_TNO_CHANGED;
			label = g_strdup_printf(_("Copy \"%i\" to selected track numbers"),
			                        pragha_musicobject_get_track_no(mobj));
			break;
			}
		case 2: {
			change = TAG_TITLE_CHANGED;
			label = g_strdup_printf(_("Copy \"%s\" to selected titles"),
			                        pragha_musicobject_get_title(mobj));
			break;
			}
		case 3: {
			change = TAG_ARTIST_CHANGED;
			label = g_strdup_printf(_("Copy \"%s\" to selected artists"),
			                        pragha_musicobject_get_artist(mobj));
			break;
			}
		case 4: {
			change = TAG_ALBUM_CHANGED;
			label = g_strdup_printf(_("Copy \"%s\" to selected albums"),
			                        pragha_musicobject_get_album(mobj));
			break;
		}
		case 5: {
			change = TAG_GENRE_CHANGED;
			label = g_strdup_printf(_("Copy \"%s\" to selected genres"),
			                        pragha_musicobject_get_genre(mobj));
			break;
		}
		case 7: {
			change = TAG_YEAR_CHANGED;
			label = g_strdup_printf(_("Copy \"%i\" to selected years"),
			                        pragha_musicobject_get_year(mobj));
			break;
		}
		case 8: {
			change = TAG_COMMENT_CHANGED;
			label = g_strdup_printf(_("Copy \"%s\" to selected comments"),
			                        pragha_musicobject_get_comment(mobj));
			break;
		}
		default: {
			break;
		}
	}

	if (change) {
		action = gtk_ui_manager_get_action (playlist->playlist_context_menu,
		                                    "/SelectionPopup/Copy tag to selection");

		g_object_set_data(G_OBJECT(action), "mobj", mobj);
		g_object_set_data(G_OBJECT(action), "change", GINT_TO_POINTER(change));

		pragha_playlist_menu_action_set_visible (playlist, "/SelectionPopup/Copy tag to selection", TRUE);
		gtk_action_set_label(GTK_ACTION(action), label);
	}
	else {
		pragha_playlist_menu_action_set_visible (playlist, "/SelectionPopup/Copy tag to selection", FALSE);
	}

	g_list_free(list);
	g_free(label);
}

/*****************/
/* DnD functions */
/*****************/
/* These two functions are only callbacks that must be passed to
gtk_tree_selection_set_select_function() to chose if GTK is allowed
to change selection itself or if we handle it ourselves */

static gboolean
pragha_playlist_selection_func_true(GtkTreeSelection *selection,
                                    GtkTreeModel *model,
                                    GtkTreePath *path,
                                    gboolean path_currently_selected,
                                    gpointer data)
{
	return TRUE;
}

static gboolean
pragha_playlist_selection_func_false(GtkTreeSelection *selection,
                                     GtkTreeModel *model,
                                     GtkTreePath *path,
                                     gboolean path_currently_selected,
                                     gpointer data)
{
	return FALSE;
}

/* Handler for current playlist click */

static gboolean
pragha_playlist_first_button_press_cb (PraghaPlaylist *playlist,
                                       GdkEventButton *event)
{
	GtkTreePath *path = NULL;
	GtkTreeSelection *selection;
	gboolean ret = FALSE;

	ret = gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW(playlist->view),
	                                     (gint) event->x,(gint) event->y,
	                                     &path, NULL, NULL, NULL);

	if (ret) {
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(playlist->view));

		if (gtk_tree_selection_path_is_selected (selection, path) &&
		    !(event->state & GDK_CONTROL_MASK) &&
		    !(event->state & GDK_SHIFT_MASK)) {
			gtk_tree_selection_set_select_function(selection, &pragha_playlist_selection_func_false, playlist, NULL);
		}
		else {
			gtk_tree_selection_set_select_function(selection, &pragha_playlist_selection_func_true, playlist, NULL);
		}
		gtk_tree_path_free(path);
	}

	return FALSE;
}

static gboolean
pragha_playlist_second_button_press_cb (PraghaPlaylist *playlist,
                                        GdkEventButton *event)
{
	GtkWidget *popup_menu;
	GtkTreeSelection *selection;
	gint n_select = 0;
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	GtkTreeIter iter;
	gboolean ret = FALSE, is_queue = FALSE;

	popup_menu = gtk_ui_manager_get_widget (playlist->playlist_context_menu, "/SelectionPopup");

	ret = gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW(playlist->view),
	                                     (gint) event->x,(gint) event->y,
	                                     &path, &column,
	                                     NULL, NULL);

	if (ret) {
		/* Click on a song.. */
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(playlist->view));

		/* Select the song. */
		if (!gtk_tree_selection_path_is_selected (selection, path)) {
			gtk_tree_selection_unselect_all (selection);
			gtk_tree_selection_select_path (selection, path);
		}

		/* Personalize popup menu acording state. */
		if (gtk_tree_model_get_iter (playlist->model, &iter, path)) {
			gtk_tree_model_get (playlist->model, &iter, P_BUBBLE, &is_queue, -1);

			if (is_queue) {
				pragha_playlist_menu_action_set_visible (playlist, "/SelectionPopup/Queue track", FALSE);
				pragha_playlist_menu_action_set_visible (playlist, "/SelectionPopup/Dequeue track", TRUE);

				pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Deueue track", TRUE);
			}
			else {
				pragha_playlist_menu_action_set_visible (playlist, "/SelectionPopup/Queue track", TRUE);
				pragha_playlist_menu_action_set_visible (playlist, "/SelectionPopup/Dequeue track", FALSE);

				pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Queue track", TRUE);
			}
		}

		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Remove from playlist", TRUE);
		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Crop playlist", TRUE);
		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Clear playlist", TRUE);
		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Save selection", TRUE);
		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Save playlist", TRUE);
		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Edit tags", TRUE);

		n_select = gtk_tree_selection_count_selected_rows (selection);
		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/ToolsMenu", (n_select == 1));

		if(n_select > 1) {
			pragha_playlist_menu_action_set_visible (playlist, "/SelectionPopup/Copy tag to selection", TRUE);
			pragha_playlist_personalize_copy_tag_to_seleccion (playlist, column, &iter);
		}
		else
			pragha_playlist_menu_action_set_visible (playlist, "/SelectionPopup/Copy tag to selection", FALSE);

		gtk_tree_path_free (path);
	}
	else {
		if (playlist->no_tracks == 0) {
			/* Click on emthy playlist. */
			pragha_playlist_menu_action_set_visible (playlist, "/SelectionPopup/Queue track", TRUE);
			pragha_playlist_menu_action_set_visible (playlist, "/SelectionPopup/Dequeue track", FALSE);
		}
		else {
			/* Click on any song. */
			selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(playlist->view));
			gtk_tree_selection_unselect_all (selection);
		}

		pragha_playlist_menu_action_set_visible (playlist, "/SelectionPopup/Copy tag to selection", FALSE);

		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Queue track", FALSE);
		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Remove from playlist", FALSE);
		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Crop playlist", FALSE);
		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Clear playlist", (playlist->no_tracks != 0));
		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Save selection", FALSE);
		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Save playlist", (playlist->no_tracks != 0));
		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/ToolsMenu", FALSE);
		pragha_playlist_menu_action_set_sensitive (playlist, "/SelectionPopup/Edit tags", FALSE);
	}

	gtk_menu_popup (GTK_MENU(popup_menu), NULL, NULL, NULL, NULL, event->button, event->time);

	if (n_select > 1)
		return TRUE;
	else
		return FALSE;
}


static gboolean
pragha_playlist_button_press_cb (GtkWidget *widget,
                                 GdkEventButton *event,
                                 PraghaPlaylist *playlist)
{
	gboolean ret = FALSE;

	switch (event->button) {
		case 1:
			ret = pragha_playlist_first_button_press_cb (playlist, event);
			break;
		case 3:
			ret = pragha_playlist_second_button_press_cb (playlist, event);
			break;
		default:
			ret = FALSE;
			break;
	}

	return ret;
}

static gboolean
current_playlist_button_release_cb(GtkWidget *widget,
				   GdkEventButton *event,
				   PraghaPlaylist *cplaylist)
{
	GtkTreeSelection *selection;
	GtkTreePath *path;
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));

	if((event->state & GDK_CONTROL_MASK) || (event->state & GDK_SHIFT_MASK) || (cplaylist->dragging == TRUE) || (event->button!=1)){
		gtk_tree_selection_set_select_function(selection, &pragha_playlist_selection_func_true, cplaylist, NULL);
		cplaylist->dragging = FALSE;
		return FALSE;
	}

	gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), (gint) event->x,(gint) event->y, &path, NULL, NULL, NULL);

	if (path){
		gtk_tree_selection_set_select_function(selection, &pragha_playlist_selection_func_true, cplaylist, NULL);
		gtk_tree_selection_unselect_all(selection);
		gtk_tree_selection_select_path(selection, path);
		gtk_tree_path_free(path);
	}
	return FALSE;
}

/* Handler for column header right click popup menu */

gboolean header_right_click_cb(GtkWidget *widget,
			       GdkEventButton *event,
			       PraghaPlaylist *cplaylist)
{
	gboolean ret = FALSE;

	switch(event->button) {
	case 3: {
		gtk_menu_popup(GTK_MENU(cplaylist->header_context_menu),
			       NULL, NULL, NULL, NULL, event->button, event->time);
		ret = TRUE;
		break;
	}
	default:
		ret = FALSE;
		break;
	}
	return ret;
}

/*******/
/* DnD */
/*******/

static gboolean
dnd_current_playlist_begin(GtkWidget *widget,
			   GdkDragContext *context,
			   PraghaPlaylist *cplaylist)
{
	cplaylist->dragging = TRUE;
	return FALSE;
}

/* Callback for DnD signal 'drag-data-get' */

static void
drag_current_playlist_get_data (GtkWidget *widget,
				GdkDragContext *context,
				GtkSelectionData *selection_data,
				guint target_type,
				guint time,
				PraghaPlaylist *cplaylist)
{
	g_assert (selection_data != NULL);

	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GList *list, *i;
	GtkTreePath *path;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	guint uri_i = 0;
	gchar **uri_list;

        switch (target_type){
		case TARGET_URI_LIST:
			CDEBUG(DBG_VERBOSE, "DnD: TARGET_URI_LIST");

			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
			list = gtk_tree_selection_get_selected_rows(selection, &model);
			uri_list = g_new(gchar* , gtk_tree_selection_count_selected_rows(selection) + 1);

			for (i=list; i != NULL; i = i->next){
				path = i->data;
				gtk_tree_model_get_iter(model, &iter, path);
				gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

				if (G_LIKELY(mobj &&
				    pragha_musicobject_is_local_file(mobj)))
					uri_list[uri_i++] = g_filename_to_uri(pragha_musicobject_get_file(mobj), NULL, NULL);

				gtk_tree_path_free(path);
			}
			uri_list[uri_i++] = NULL;

			gtk_selection_data_set_uris(selection_data,  uri_list);
			g_strfreev(uri_list);
			g_list_free(list);
			break;
		default:
			break;
	}

}

static gboolean
dnd_current_playlist_drop(GtkWidget *widget,
			  GdkDragContext *context,
			  gint x,
			  gint y,
			  guint time,
			  PraghaPlaylist *cplaylist)
{
	GList *p;

	if (gdk_drag_context_list_targets (context) == NULL)
		return FALSE;

	for (p = gdk_drag_context_list_targets (context); p != NULL; p = p->next) {
		gchar *possible_type;

		possible_type = gdk_atom_name (GDK_POINTER_TO_ATOM (p->data));
		if (!strcmp (possible_type, "REF_LIBRARY")) {
			CDEBUG(DBG_VERBOSE, "DnD: library_tree");

			gtk_drag_get_data(widget,
					  context,
					  GDK_POINTER_TO_ATOM (p->data),
					  time);

			g_free (possible_type);

			return TRUE;
		}
		g_free (possible_type);
	}

	return FALSE;
}

/* Reorder playlist with DnD. */

void
dnd_current_playlist_reorder(GtkTreeModel *model,
			     GtkTreeIter *dest_iter,
			     GtkTreeViewDropPosition pos,
			     PraghaPlaylist *cplaylist)
{
	GtkTreeRowReference *ref;
	GtkTreePath *path = NULL;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GList *list = NULL, *l;

	CDEBUG(DBG_VERBOSE, "Dnd: Reorder");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, NULL);

	/* Clear sort */
	clear_sort_current_playlist(NULL, cplaylist);

	/* No selections */
	if (!list)
		goto exit;

	/* Store references to the selected paths */
	l = list;
	while(l) {
		path = l->data;
		ref = gtk_tree_row_reference_new(model, path);
		l->data = ref;
		gtk_tree_path_free(path);
		l = l->next;
	}

	for (l=list; l != NULL; l = l->next) {
		ref = l->data;
		path = gtk_tree_row_reference_get_path(ref);
		gtk_tree_model_get_iter(model, &iter, path);

		if (pos == GTK_TREE_VIEW_DROP_BEFORE)
			gtk_list_store_move_before(GTK_LIST_STORE(model), &iter, dest_iter);
		else if (pos == GTK_TREE_VIEW_DROP_AFTER)
			gtk_list_store_move_after(GTK_LIST_STORE(model), &iter, dest_iter);

			gtk_tree_path_free(path);
			gtk_tree_row_reference_free(ref);
	}

exit:
	g_list_free(list);
}

/* Callback for DnD signal 'drag-data-received' */

static void
dnd_current_playlist_received(GtkWidget *playlist_view,
			     GdkDragContext *context,
			     gint x,
			     gint y,
			     GtkSelectionData *data,
			     PraghaDndTarget info,
			     guint time,
			     PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreePath *dest_path = NULL;
	GtkTreeIter dest_iter;
	GtkTreeViewDropPosition pos = 0;
	gboolean is_row;
	GdkRectangle vrect, crect;
	gdouble row_align;
	GList *list = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(playlist_view));

	is_row = gtk_tree_view_get_dest_row_at_pos(GTK_TREE_VIEW(playlist_view),
						x, y,
						&dest_path,
						&pos);

	gtk_tree_view_get_visible_rect(GTK_TREE_VIEW(playlist_view), &vrect);
	gtk_tree_view_get_cell_area(GTK_TREE_VIEW(playlist_view), dest_path, NULL, &crect);
	
	row_align = (gdouble)crect.y / (gdouble)vrect.height;

	switch(pos) {
		case GTK_TREE_VIEW_DROP_INTO_OR_BEFORE:
			pos = GTK_TREE_VIEW_DROP_BEFORE;
			break;
		case GTK_TREE_VIEW_DROP_INTO_OR_AFTER:
			pos = GTK_TREE_VIEW_DROP_AFTER;
			break;
		default:
			break;
	}

	if (is_row)
		gtk_tree_model_get_iter (model, &dest_iter, dest_path);

	/* Reorder within current playlist */

	if (gtk_drag_get_source_widget(context) == playlist_view) {
		dnd_current_playlist_reorder(model, &dest_iter, pos, cplaylist);
		goto exit;
	}

	/* Get new tracks to append on playlist */

	switch(info) {
	case TARGET_REF_LIBRARY:
		list = pragha_dnd_library_get_mobj_list (data, cplaylist->cdbase);
		break;
	case TARGET_URI_LIST:
		list = pragha_dnd_uri_list_get_mobj_list (data);
		break;
	case TARGET_PLAIN_TEXT:
		list = pragha_dnd_plain_text_get_mobj_list (data);
		break;
	default:
		g_warning("Unknown DND type");
		break;
	}

	/* Insert mobj list to current playlist. */

	if (is_row) {
		pragha_playlist_insert_mobj_list(cplaylist, list, pos, &dest_iter);
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(playlist_view), dest_path, NULL, TRUE, row_align, 0.0);
	}
	else
		pragha_playlist_append_mobj_list(cplaylist, list);

	g_list_free(list);

exit:
	gtk_tree_path_free(dest_path);
	gtk_drag_finish(context, TRUE, FALSE, time);
}

/* Get a list of all music objects on current playlist */

GList *
pragha_playlist_get_mobj_list(PraghaPlaylist* cplaylist)
{
	GtkTreeModel *model = cplaylist->model;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	GList *list = NULL;
	gboolean valid;

	valid = gtk_tree_model_get_iter_first(model, &iter);
	while (valid) {
		gtk_tree_model_get (model, &iter, P_MOBJ_PTR, &mobj, -1);

		if (G_LIKELY(mobj))
			list = g_list_prepend(list, mobj);

		valid = gtk_tree_model_iter_next(model, &iter);
	}
	return list;
}

/* Get a list of selected music objects on current playlist */

GList *
pragha_playlist_get_selection_mobj_list(PraghaPlaylist* cplaylist)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;
	GList *list = NULL, *mlist = NULL, *i;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list) {
		for (i=list; i != NULL; i = i->next) {
			path = i->data;
			gtk_tree_model_get_iter(model, &iter, path);
			gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);

			if (G_LIKELY(mobj))
				mlist = g_list_prepend(mlist, mobj);

			gtk_tree_path_free(path);
		}
		g_list_free (list);
	}
	return mlist;
}

GList *
pragha_playlist_get_selection_ref_list(PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreePath *path = NULL;
	GList *list, *i;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	/* Get references from the paths and store them in the 'data'
	   portion of the list elements.
	   This idea was inspired by code from 'claws-mail' */

	for (i = list; i != NULL; i = i->next) {
		path = i->data;
		ref = gtk_tree_row_reference_new(model, path);
		i->data = ref;
		gtk_tree_path_free(path);
	}

	return list;
}

/* Get the musicobject of seleceted track on current playlist */

PraghaMusicobject *
pragha_playlist_get_selected_musicobject(PraghaPlaylist* cplaylist)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GList *list;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	PraghaMusicobject *mobj = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(cplaylist->view));
	list = gtk_tree_selection_get_selected_rows(selection, &model);

	if (list != NULL) {
		path = list->data;
		if (gtk_tree_model_get_iter(model, &iter, path)) {
			gtk_tree_model_get(model, &iter, P_MOBJ_PTR, &mobj, -1);
			if (!mobj)
				g_warning("Invalid mobj pointer");
		}

		g_list_free_full (list, (GDestroyNotify) gtk_tree_path_free);
	}

	return mobj;
}

/* Save current playlist state on exit */

void
pragha_playlist_save_playlist_state (PraghaPlaylist* cplaylist)
{
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	gint playlist_id = 0;
	gchar *ref_char = NULL;

	/* Save last playlist. */

	playlist_id = pragha_database_find_playlist (cplaylist->cdbase, SAVE_PLAYLIST_STATE);
	if (!playlist_id)
		playlist_id = pragha_database_add_new_playlist (cplaylist->cdbase, SAVE_PLAYLIST_STATE);
	else
		pragha_database_flush_playlist (cplaylist->cdbase, playlist_id);

	if (!gtk_tree_model_get_iter_first(cplaylist->model, &iter))
		return;

	save_playlist(cplaylist, playlist_id, SAVE_COMPLETE);

	/* Save reference to current song. */

	path = get_current_track (cplaylist);
	if(path) {
		ref_char = gtk_tree_path_to_string (path);
		gtk_tree_path_free(path);

		pragha_preferences_set_string(cplaylist->preferences,
					      GROUP_PLAYLIST,
					      KEY_CURRENT_REF,
					      ref_char);
		g_free (ref_char);
	}
	else {
		pragha_preferences_remove_key(cplaylist->preferences,
					      GROUP_PLAYLIST,
					      KEY_CURRENT_REF);
	}
}

/* Init current playlist on application bringup,
   restore stored playlist */

static void init_playlist_current_playlist(PraghaPlaylist *cplaylist)
{
	gint playlist_id, location_id;
	PraghaMusicobject *mobj;
	GList *list = NULL;

	playlist_id = pragha_database_find_playlist (cplaylist->cdbase, SAVE_PLAYLIST_STATE);

	const gchar *sql = "SELECT file FROM PLAYLIST_TRACKS WHERE playlist = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (cplaylist->cdbase, sql);
	pragha_prepared_statement_bind_int (statement, 1, playlist_id);

	while (pragha_prepared_statement_step (statement)) {
		const gchar *file = pragha_prepared_statement_get_string (statement, 0);
		/* TODO: Fix this negradaaa!. */
		if(g_str_has_prefix(file, "Radio:/") == FALSE) {
			if ((location_id = pragha_database_find_location (cplaylist->cdbase, file)))
				mobj = new_musicobject_from_db(cplaylist->cdbase, location_id);
			else
				mobj = new_musicobject_from_file(file);
		}
		else {
			mobj = new_musicobject_from_location(file + strlen("Radio:/"), file + strlen("Radio:/"));
		}

		if (G_LIKELY(mobj))
			list = g_list_append(list, mobj);
	}

	pragha_prepared_statement_free (statement);

	if (list) {
		pragha_playlist_append_mobj_list(cplaylist, list);
		g_list_free (list);
	}
}

void
pragha_playlist_init_playlist_state (PraghaPlaylist *cplaylist)
{
	gchar *ref = NULL;
	GtkTreePath *path = NULL;

	init_playlist_current_playlist(cplaylist);

	ref = pragha_preferences_get_string(cplaylist->preferences,
	                                    GROUP_PLAYLIST,
	                                    KEY_CURRENT_REF);

	if (!ref)
		return;

	path = gtk_tree_path_new_from_string(ref);
	pragha_playlist_select_path (cplaylist, path, pragha_preferences_get_shuffle(cplaylist->preferences));

	gtk_tree_path_free(path);
	g_free(ref);
}

/* Initialize columns of current playlist */

static void
init_current_playlist_columns(PraghaPlaylist* cplaylist)
{
	gchar **columns;
	const gchar *col_name;
	GtkTreeViewColumn *col;
	GList *list = NULL, *i;
	GSList *j;
	gint k = 0;
	gint *col_widths;
	gsize cnt = 0, isize;

	columns = pragha_preferences_get_string_list(cplaylist->preferences,
						     GROUP_PLAYLIST,
						     KEY_PLAYLIST_COLUMNS,
						     &cnt);
	if (columns) {
		for (isize=0; isize < cnt; isize++) {
			cplaylist->columns =
				g_slist_append(cplaylist->columns,
					       g_strdup(columns[isize]));
		}
		g_strfreev(columns);
	}
	else {
		cplaylist->columns =
			g_slist_append(cplaylist->columns,
				       g_strdup(P_TITLE_STR));
		cplaylist->columns =
			g_slist_append(cplaylist->columns,
				       g_strdup(P_ARTIST_STR));
		cplaylist->columns =
			g_slist_append(cplaylist->columns,
				       g_strdup(P_ALBUM_STR));
		cplaylist->columns =
			g_slist_append(cplaylist->columns,
				       g_strdup(P_LENGTH_STR));
	}

	col_widths = pragha_preferences_get_integer_list(cplaylist->preferences,
							 GROUP_PLAYLIST,
							 KEY_PLAYLIST_COLUMN_WIDTHS,
							 &cnt);
	if (col_widths) {
		for (isize = 0; isize < cnt; isize++) {
			cplaylist->column_widths =
				g_slist_append(cplaylist->column_widths,
					       GINT_TO_POINTER(col_widths[isize]));
		}
		g_free(col_widths);
	}
	else {
		for (isize=0; isize < 4; isize++) {
			cplaylist->column_widths =
				g_slist_append(cplaylist->column_widths,
				       GINT_TO_POINTER(DEFAULT_PLAYLIST_COL_WIDTH));
		}
	}

	/* Mark only the columns that are present in
	   cplaylist->columns as visible.
	   And set their sizes */

	list = gtk_tree_view_get_columns(GTK_TREE_VIEW(cplaylist->view));

	if (list) {
		for (i=list; i != NULL; i = i->next) {
			col = i->data;
			col_name = gtk_tree_view_column_get_title(col);
			if (is_present_str_list(col_name,
						cplaylist->columns)) {
				j = g_slist_nth(cplaylist->column_widths,
						k++);
				gtk_tree_view_column_set_visible(col, TRUE);
				gtk_tree_view_column_set_sizing(col,
						GTK_TREE_VIEW_COLUMN_FIXED);
				if (GPOINTER_TO_INT(j->data) > COL_WIDTH_THRESH)
					gtk_tree_view_column_set_fixed_width(col,
						     GPOINTER_TO_INT(j->data));
				else
					gtk_tree_view_column_set_fixed_width(col,
						     DEFAULT_PLAYLIST_COL_WIDTH);
			}
			else
				gtk_tree_view_column_set_visible(col, FALSE);
		}
		g_list_free(list);
	}
	else
		g_warning("(%s): No columns in playlist view", __func__);

	/* Always show queue and status pixbuf colum */
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cplaylist->view), 0);
	gtk_tree_view_column_set_visible(col, TRUE);
	gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(col, 32);
}

static GtkWidget*
create_header_context_menu(PraghaPlaylist* cplaylist)
{
	GtkWidget *menu;
	GtkWidget *toggle_track,
		*toggle_title,
		*toggle_artist,
		*toggle_album,
		*toggle_genre,
		*toggle_bitrate,
		*toggle_year,
		*toggle_comment,
		*toggle_length,
		*toggle_filename,
		*separator,
		*action_clear_sort;

	menu = gtk_menu_new();

	/* Create the checkmenu items */

	toggle_track = gtk_check_menu_item_new_with_label(_("Track"));
	toggle_title = gtk_check_menu_item_new_with_label(_("Title"));
	toggle_artist = gtk_check_menu_item_new_with_label(_("Artist"));
	toggle_album = gtk_check_menu_item_new_with_label(_("Album"));
	toggle_genre = gtk_check_menu_item_new_with_label(_("Genre"));
	toggle_bitrate = gtk_check_menu_item_new_with_label(_("Bitrate"));
	toggle_year = gtk_check_menu_item_new_with_label(_("Year"));
	toggle_comment = gtk_check_menu_item_new_with_label(_("Comment"));
	toggle_length = gtk_check_menu_item_new_with_label(_("Length"));
	toggle_filename = gtk_check_menu_item_new_with_label(_("Filename"));
	separator = gtk_separator_menu_item_new ();

	action_clear_sort = gtk_image_menu_item_new_with_label(_("Clear sort"));
        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(action_clear_sort),
                gtk_image_new_from_icon_name("view-refresh", GTK_ICON_SIZE_MENU));

	/* Add the items to the menu */

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_track);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_title);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_artist);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_album);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_genre);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_bitrate);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_year);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_comment);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_length);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), toggle_filename);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), action_clear_sort);

	/* Initialize the state of the items */

	if (is_present_str_list(P_TRACK_NO_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_track), TRUE);
	if (is_present_str_list(P_TITLE_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_title), TRUE);
	if (is_present_str_list(P_ARTIST_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_artist), TRUE);
	if (is_present_str_list(P_ALBUM_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_album), TRUE);
	if (is_present_str_list(P_GENRE_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_genre), TRUE);
	if (is_present_str_list(P_BITRATE_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_bitrate), TRUE);
	if (is_present_str_list(P_YEAR_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_year), TRUE);
	if (is_present_str_list(P_COMMENT_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_comment), TRUE);
	if (is_present_str_list(P_LENGTH_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_length), TRUE);
	if (is_present_str_list(P_FILENAME_STR, cplaylist->columns))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_filename), TRUE);

	/* Setup the individual signal handlers */

	g_signal_connect(G_OBJECT(toggle_track), "toggled",
			 G_CALLBACK(playlist_track_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_title), "toggled",
			 G_CALLBACK(playlist_title_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_artist), "toggled",
			 G_CALLBACK(playlist_artist_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_album), "toggled",
			 G_CALLBACK(playlist_album_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_genre), "toggled",
			 G_CALLBACK(playlist_genre_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_bitrate), "toggled",
			 G_CALLBACK(playlist_bitrate_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_year), "toggled",
			 G_CALLBACK(playlist_year_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_comment), "toggled",
			 G_CALLBACK(playlist_comment_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_length), "toggled",
			 G_CALLBACK(playlist_length_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(toggle_filename), "toggled",
			 G_CALLBACK(playlist_filename_column_change_cb), cplaylist);
	g_signal_connect(G_OBJECT(action_clear_sort), "activate",
			 G_CALLBACK(clear_sort_current_playlist_cb), cplaylist);

	gtk_widget_show_all(menu);

	return menu;
}

static GtkUIManager*
pragha_playlist_context_menu_new (PraghaPlaylist *playlist)
{
	GtkUIManager *context_menu = NULL;
	GtkActionGroup *context_actions;
	GError *error = NULL;

	context_actions = gtk_action_group_new("Playlist Context Actions");
	context_menu = gtk_ui_manager_new();

	gtk_action_group_set_translation_domain (context_actions, GETTEXT_PACKAGE);

	if (!gtk_ui_manager_add_ui_from_string(context_menu, playlist_context_menu_xml, -1, &error)) {
		g_critical("Unable to create current playlist context menu, err : %s", error->message);
	}
	gtk_action_group_add_actions(context_actions,
	                             playlist_context_aentries,
	                             G_N_ELEMENTS(playlist_context_aentries),
	                             (gpointer) playlist);

	gtk_ui_manager_insert_action_group(context_menu, context_actions, 0);

	g_object_unref (context_actions);

	return context_menu;
}

static const GtkTargetEntry pentries[] = {
	{"REF_LIBRARY", GTK_TARGET_SAME_APP, TARGET_REF_LIBRARY},
	{"text/uri-list", GTK_TARGET_OTHER_APP, TARGET_URI_LIST},
	{"text/plain", GTK_TARGET_OTHER_APP, TARGET_PLAIN_TEXT}
};

static void
init_playlist_dnd(PraghaPlaylist *cplaylist)
{
	/* Source/Dest: Current Playlist */

	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(cplaylist->view),
					       GDK_BUTTON1_MASK,
					       pentries,
					       G_N_ELEMENTS(pentries),
					       GDK_ACTION_COPY | GDK_ACTION_MOVE);

	gtk_tree_view_enable_model_drag_dest(GTK_TREE_VIEW(cplaylist->view),
					     pentries,
					     G_N_ELEMENTS(pentries),
					     GDK_ACTION_COPY | GDK_ACTION_MOVE);

	g_signal_connect(G_OBJECT(GTK_WIDGET(cplaylist->view)),
			 "drag-begin",
			 G_CALLBACK(dnd_current_playlist_begin),
			 cplaylist);
	 g_signal_connect (G_OBJECT(cplaylist->view),
			 "drag-data-get",
			 G_CALLBACK (drag_current_playlist_get_data),
			 cplaylist);
	g_signal_connect(G_OBJECT(cplaylist->view),
			 "drag-drop",
			 G_CALLBACK(dnd_current_playlist_drop),
			 cplaylist);
	g_signal_connect(G_OBJECT(cplaylist->view),
			 "drag-data-received",
			 G_CALLBACK(dnd_current_playlist_received),
			 cplaylist);
}

static void
create_current_playlist_columns(PraghaPlaylist *cplaylist, GtkTreeView *view)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget *state_pixbuf,
		*label_track,
		*label_title,
		*label_artist,
		*label_album,
		*label_genre,
		*label_bitrate,
		*label_year,
		*label_comment,
		*label_length,
		*label_filename;
	GtkWidget *col_button;

	label_track = gtk_label_new(_("Track"));
	label_title = gtk_label_new(_("Title"));
	label_artist = gtk_label_new(_("Artist"));
	label_album = gtk_label_new(_("Album"));
	label_genre = gtk_label_new(_("Genre"));
	label_bitrate = gtk_label_new(_("Bitrate"));
	label_year = gtk_label_new(_("Year"));
	label_comment = gtk_label_new(_("Comment"));
	label_length = gtk_label_new(_("Length"));
	label_filename = gtk_label_new(_("Filename"));

	state_pixbuf = gtk_image_new_from_icon_name ("audio-volume-high", GTK_ICON_SIZE_MENU);

	/* Column : Queue Bubble and Status Pixbuf */

	column = gtk_tree_view_column_new ();

	renderer = gtk_cell_renderer_bubble_new ();
	gtk_cell_renderer_set_fixed_size (renderer, 12, -1);
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
	gtk_tree_view_column_set_attributes(column, renderer, "markup", P_QUEUE, "show-bubble", P_BUBBLE, NULL);

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", P_STATUS_PIXBUF, NULL);

	gtk_tree_view_column_set_resizable(column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_view_column_set_widget (column, state_pixbuf);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_widget_show (state_pixbuf);

	/* Column : Track No */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	column = gtk_tree_view_column_new_with_attributes(P_TRACK_NO_STR,
							  renderer,
							  "text",
							  P_TRACK_NO,
							  NULL);

	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_TRACK_NO);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_track);
	gtk_widget_show(label_track);
	col_button = gtk_widget_get_ancestor(label_track, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Title */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	column = gtk_tree_view_column_new_with_attributes(P_TITLE_STR,
							  renderer,
							  "text",
							  P_TITLE,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_TITLE);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_title);
	gtk_widget_show(label_title);
	col_button = gtk_widget_get_ancestor(label_title, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Artist */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	column = gtk_tree_view_column_new_with_attributes(P_ARTIST_STR,
							  renderer,
							  "text",
							  P_ARTIST,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_ARTIST);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_artist);
	gtk_widget_show(label_artist);
	col_button = gtk_widget_get_ancestor(label_artist, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Album */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_ALBUM_STR,
							  renderer,
							  "text",
							  P_ALBUM,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_ALBUM);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_album);
	gtk_widget_show(label_album);
	col_button = gtk_widget_get_ancestor(label_album, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Genre */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_GENRE_STR,
							  renderer,
							  "text",
							  P_GENRE,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_GENRE);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_genre);
	gtk_widget_show(label_genre);
	col_button = gtk_widget_get_ancestor(label_genre, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Bitrate */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_BITRATE_STR,
							  renderer,
							  "text",
							  P_BITRATE,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_BITRATE);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_bitrate);
	gtk_widget_show(label_bitrate);
	col_button = gtk_widget_get_ancestor(label_bitrate, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Year */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_YEAR_STR,
							  renderer,
							  "text",
							  P_YEAR,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_YEAR);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_year);
	gtk_widget_show(label_year);
	col_button = gtk_widget_get_ancestor(label_year, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Comment */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_COMMENT_STR,
							  renderer,
							  "text",
							  P_COMMENT,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_COMMENT);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_comment);
	gtk_widget_show(label_comment);
	col_button = gtk_widget_get_ancestor(label_comment, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Length */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_LENGTH_STR,
							  renderer,
							  "text",
							  P_LENGTH,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_LENGTH);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_length);
	gtk_widget_show(label_length);
	col_button = gtk_widget_get_ancestor(label_length, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);

	/* Column : Filename */

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer),1);
	gtk_cell_renderer_set_fixed_size (renderer, 1, -1);
	column = gtk_tree_view_column_new_with_attributes(P_FILENAME_STR,
							  renderer,
							  "text",
							  P_FILENAME,
							  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, P_FILENAME);
	g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_widget(column, label_filename);
	gtk_widget_show(label_filename);
	col_button = gtk_widget_get_ancestor(label_filename, GTK_TYPE_BUTTON);
	g_signal_connect(G_OBJECT(GTK_WIDGET(col_button)), "button-press-event",
			 G_CALLBACK(header_right_click_cb), cplaylist);
}

void
update_current_playlist_view_playback_state_cb (PraghaBackend *backend, GParamSpec *pspec, PraghaPlaylist *cplaylist)
{
	update_current_playlist_view_track(cplaylist, backend);
}

static GtkWidget*
create_current_playlist_view (PraghaPlaylist *cplaylist)
{
	GtkWidget *current_playlist;
	GtkListStore *store;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeSortable *sortable;
	const GBindingFlags binding_flags = G_BINDING_SYNC_CREATE;

	/* Create the tree store */

	store = gtk_list_store_new(N_P_COLUMNS,
				   G_TYPE_POINTER,	/* Pointer to musicobject */
				   G_TYPE_STRING,	/* Queue No String */
				   G_TYPE_BOOLEAN,	/* Show Bublle Queue */
				   GDK_TYPE_PIXBUF,	/* Playback status pixbuf */
				   G_TYPE_STRING,	/* Tag : Track No */
				   G_TYPE_STRING,	/* Tag : Title */
				   G_TYPE_STRING,	/* Tag : Artist */
				   G_TYPE_STRING,	/* Tag : Album */
				   G_TYPE_STRING,	/* Tag : Genre */
				   G_TYPE_STRING,	/* Tag : Bitrate */
				   G_TYPE_STRING,	/* Tag : Year */
				   G_TYPE_STRING,	/* Tag : Comment */
				   G_TYPE_STRING,	/* Tag : Length */
				   G_TYPE_STRING,	/* Filename */
				   G_TYPE_BOOLEAN);	/* Played flag */

	/* Create the tree view */

	current_playlist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(current_playlist));
	sortable = GTK_TREE_SORTABLE(model);

	/* Disable interactive search */

	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(current_playlist), FALSE);

	/* Set the sort functions */

	gtk_tree_sortable_set_sort_func(sortable,
					P_TRACK_NO,
					compare_track_no,
					NULL,
					NULL);
	gtk_tree_sortable_set_sort_func(sortable,
					P_BITRATE,
					compare_bitrate,
					NULL,
					NULL);
	gtk_tree_sortable_set_sort_func(sortable,
					P_YEAR,
					compare_year,
					NULL,
					NULL);
	gtk_tree_sortable_set_sort_func(sortable,
					P_LENGTH,
					compare_length,
					NULL,
					NULL);

	/* Set selection properties */

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(current_playlist));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	/* Create the columns and cell renderers */

	create_current_playlist_columns(cplaylist, GTK_TREE_VIEW(current_playlist));

	/* Signal handler for double-clicking on a row */

	g_signal_connect(G_OBJECT(current_playlist), "key-press-event",
			  G_CALLBACK (current_playlist_key_press), cplaylist);

	/* Store the treeview in the scrollbar widget */

	g_object_bind_property (cplaylist->preferences, "use-hint", current_playlist, "rules-hint", binding_flags);

	g_object_unref(store);

	return current_playlist;
}

static void
playlist_column_set_visible(PraghaPlaylist* cplaylist, gint column, gboolean visible)
{
	const gchar *col_name;
	GtkTreeViewColumn *col;

	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cplaylist->view),
				       column - 3);

	if (!col) {
		g_warning("Invalid column number");
		return;
	}

	col_name = gtk_tree_view_column_get_title(col);
	gtk_tree_view_column_set_visible(col, visible);
	modify_current_playlist_columns(cplaylist, col_name, visible);
}

/* Callback for adding/deleting track_no column */

static void
playlist_track_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_TRACK_NO, state);
}

/* Callback for adding/deleting title column */

static void
playlist_title_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_TITLE, state);
}

/* Callback for adding/deleting artist column */

static void
playlist_artist_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_ARTIST, state);
}

/* Callback for adding/deleting album column */

static void
playlist_album_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_ALBUM, state);
}

/* Callback for adding/deleting genre column */

static void
playlist_genre_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_GENRE, state);
}

/* Callback for adding/deleting bitrate column */

static void
playlist_bitrate_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_BITRATE, state);
}

/* Callback for adding/deleting year column */

static void
playlist_year_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_YEAR, state);
}

/* Callback for adding/deleting comment column */

static void
playlist_comment_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_COMMENT, state);
}

/* Callback for adding/deleting length column */

static void
playlist_length_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_LENGTH, state);
}

/* Callback for adding/deleting filename column */

static void
playlist_filename_column_change_cb(GtkCheckMenuItem *item, PraghaPlaylist* cplaylist)
{
	gboolean state;
	state = gtk_check_menu_item_get_active(item);

	playlist_column_set_visible(cplaylist, P_FILENAME, state);
}

/* Clear sort in the current playlist */

static void
clear_sort_current_playlist_cb(GtkMenuItem *item, PraghaPlaylist *cplaylist)
{
	GtkTreeModel *model = cplaylist->model;

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
			     GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID,
			     GTK_SORT_ASCENDING);
}

/* Comparison function for track numbers */

static gint
compare_track_no(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	PraghaMusicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (pragha_musicobject_get_track_no(mobj_a) < pragha_musicobject_get_track_no(mobj_b))
		return -1;
	else if (pragha_musicobject_get_track_no(mobj_a) > pragha_musicobject_get_track_no(mobj_b))
		return 1;
	else
		return 0;
}

/* Comparison function for bitrates */

static gint
compare_bitrate(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	PraghaMusicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (pragha_musicobject_get_bitrate(mobj_a) < pragha_musicobject_get_bitrate(mobj_b))
		return -1;
	else if (pragha_musicobject_get_bitrate(mobj_a) > pragha_musicobject_get_bitrate(mobj_b))
		return 1;
	else
		return 0;
}

/* Comparison function for years */

static gint
compare_year(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	PraghaMusicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (pragha_musicobject_get_year(mobj_a) < pragha_musicobject_get_year(mobj_b))
		return -1;
	else if (pragha_musicobject_get_year(mobj_a) > pragha_musicobject_get_year(mobj_b))
		return 1;
	else
		return 0;
}

/* Comparison function for lengths */

static gint
compare_length(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	PraghaMusicobject *mobj_a = NULL, *mobj_b = NULL;

	gtk_tree_model_get(model, a, P_MOBJ_PTR, &mobj_a, -1);
	gtk_tree_model_get(model, b, P_MOBJ_PTR, &mobj_b, -1);

	if (!mobj_a || !mobj_b)
		return 0;
	if (pragha_musicobject_get_length(mobj_a) < pragha_musicobject_get_length(mobj_b))
		return -1;
	else if (pragha_musicobject_get_length(mobj_a) > pragha_musicobject_get_length(mobj_b))
		return 1;
	else
		return 0;
}

gboolean
pragha_playlist_propagate_event(PraghaPlaylist* cplaylist, GdkEventKey *event)
{
	GdkEvent *new_event;
	gboolean ret;

	gtk_widget_grab_focus(cplaylist->view);

	new_event = gdk_event_copy ((GdkEvent *) event);
	ret = gtk_widget_event (GTK_WIDGET (cplaylist->view), new_event);
	gdk_event_free (new_event);

	return ret;
}

void
pragha_playlist_activate_path(PraghaPlaylist* cplaylist, GtkTreePath *path)
{
	GtkTreeViewColumn *col;
	col = gtk_tree_view_get_column(GTK_TREE_VIEW(cplaylist->view), 1);
	gtk_tree_view_row_activated(GTK_TREE_VIEW(cplaylist->view),
				   path,
				   col);
}

void
pragha_playlist_activate_unique_mobj(PraghaPlaylist* cplaylist, PraghaMusicobject *mobj)
{
	GtkTreePath *path = NULL;

	path = current_playlist_path_at_mobj(mobj, cplaylist);

	if(path) {
		pragha_playlist_activate_path(cplaylist, path);
		gtk_tree_path_free (path);
	}
}

gint
pragha_playlist_get_no_tracks (PraghaPlaylist *playlist)
{
	return playlist->no_tracks;
}

gint pragha_playlist_get_total_playtime (PraghaPlaylist *playlist)
{
	GtkTreeIter iter;
	gint total_playtime = 0;
	PraghaMusicobject *mobj = NULL;
	gboolean ret;

	if(playlist->changing)
		return 0;

	ret = gtk_tree_model_get_iter_first (playlist->model, &iter);

	while (ret) {
		gtk_tree_model_get (playlist->model, &iter, P_MOBJ_PTR, &mobj, -1);
		if (mobj)
			total_playtime += pragha_musicobject_get_length (mobj);
		ret = gtk_tree_model_iter_next (playlist->model, &iter);
	}

	return total_playtime;
}

gboolean
pragha_playlist_has_queue(PraghaPlaylist* cplaylist)
{
	if(cplaylist->queue_track_refs)
		return TRUE;
	else
		return FALSE;
}

gboolean
pragha_playlist_is_changing(PraghaPlaylist* cplaylist)
{
	return cplaylist->changing;
}

void
pragha_playlist_set_changing(PraghaPlaylist* cplaylist, gboolean changing)
{
	cplaylist->changing = changing;

	gtk_widget_set_sensitive(GTK_WIDGET(cplaylist), !changing);
}

static void
shuffle_changed_cb (GObject *gobject, GParamSpec *pspec, gpointer user_data)
{
	GtkTreeRowReference *ref;
	PraghaPlaylist *cplaylist = user_data;
	gboolean shuffle = pragha_preferences_get_shuffle(cplaylist->preferences);

	if(!cplaylist->no_tracks)
		return;

	if (shuffle) {
		CDEBUG(DBG_INFO, "Turning shuffle on");
		if (cplaylist->curr_seq_ref) {
			ref = gtk_tree_row_reference_copy(cplaylist->curr_seq_ref);
			reset_rand_track_refs(cplaylist, ref);
		}
	}
	else {
		CDEBUG(DBG_INFO, "Turning shuffle off");
		pragha_playlist_clear_dirty_all (cplaylist);
		if (cplaylist->curr_rand_ref)
			cplaylist->curr_seq_ref =
				gtk_tree_row_reference_copy(cplaylist->curr_rand_ref);
		else
			cplaylist->curr_seq_ref = NULL;
	}
}

GtkWidget *
pragha_playlist_get_view(PraghaPlaylist* cplaylist)
{
	return cplaylist->view;
}

GtkTreeModel *
pragha_playlist_get_model(PraghaPlaylist* cplaylist)
{
	return cplaylist->model;
}

GtkUIManager *
pragha_playlist_get_context_menu(PraghaPlaylist* cplaylist)
{
	return cplaylist->playlist_context_menu;
}

gint
pragha_playlist_append_plugin_action (PraghaPlaylist *cplaylist,
                                      GtkActionGroup *action_group,
                                      const gchar *menu_xml)
{
	GtkUIManager *ui_manager;
	GError *error = NULL;
	gint merge_id;

	ui_manager = cplaylist->playlist_context_menu;
	gtk_ui_manager_insert_action_group (ui_manager, action_group, -1);

	merge_id = gtk_ui_manager_add_ui_from_string (ui_manager,
	                                              menu_xml,
	                                              -1,
	                                              &error);

	if (error) {
		g_warning ("Adding plugin to playlist menu: %s", error->message);
		g_error_free (error);
	}

	return merge_id;
}

void
pragha_playlist_remove_plugin_action (PraghaPlaylist *cplaylist,
                                      GtkActionGroup *action_group,
                                      gint merge_id)
{
	gtk_ui_manager_remove_ui (cplaylist->playlist_context_menu, merge_id);
	gtk_ui_manager_remove_action_group (cplaylist->playlist_context_menu, action_group);
	g_object_unref (action_group);
}

PraghaDatabase *
pragha_playlist_get_database(PraghaPlaylist* cplaylist)
{
	return cplaylist->cdbase;
}

static void
pragha_playlist_save_preferences(PraghaPlaylist* cplaylist)
{
	GtkTreeViewColumn *col;
	const gchar *col_name;
	gchar **columns;
	gint cnt = 0, i = 0, *col_widths;
	GSList *list;
	GList *cols, *j;

	/* Save list of columns visible in current playlist */

	if (cplaylist->columns) {
		list = cplaylist->columns;
		cnt = g_slist_length(cplaylist->columns);
		columns = g_new0(gchar *, cnt);

		for (i=0; i<cnt; i++) {
			columns[i] = list->data;
			list = list->next;
		}

		pragha_preferences_set_string_list (cplaylist->preferences,
		                                    GROUP_PLAYLIST,
		                                    KEY_PLAYLIST_COLUMNS,
		                                    (const gchar **)columns,
		                                    cnt);
		g_free(columns);
	}

	/* Save column widths */

	cols = gtk_tree_view_get_columns(GTK_TREE_VIEW(cplaylist->view));
	cnt = g_list_length(cols);
	if (cols) {
		col_widths = g_new0(gint, cnt);
		for (j=cols, i=0; j != NULL; j = j->next) {
			col = j->data;
			col_name = gtk_tree_view_column_get_title(col);
			if (is_present_str_list(col_name,
						cplaylist->columns))
				col_widths[i++] =
					gtk_tree_view_column_get_width(col);
		}
		pragha_preferences_set_integer_list (cplaylist->preferences,
		                                     GROUP_PLAYLIST,
		                                     KEY_PLAYLIST_COLUMN_WIDTHS,
		                                     col_widths,
		                                     i);
		g_list_free(cols);
		g_free(col_widths);
	}
}

static void
pragha_playlist_init_pixbuf(PraghaPlaylist* cplaylist)
{
	GtkIconTheme *icontheme = gtk_icon_theme_get_default();

	cplaylist->playing_pixbuf =
		gtk_icon_theme_load_icon (icontheme,
					  "media-playback-start",
					  16, 0, NULL);
	cplaylist->paused_pixbuf =
		gtk_icon_theme_load_icon (icontheme,
					  "media-playback-pause",
					  16, 0, NULL);
}

static void
pragha_playlist_update_menu_playlist_changes (PraghaDatabase *database,
                                              PraghaPlaylist *playlist)
{
	update_playlist_changes_on_menu (playlist);
}

static void
pragha_playlist_init (PraghaPlaylist *playlist)
{
	/* Get usefuls instances */

	playlist->cdbase = pragha_database_get ();
	playlist->preferences = pragha_preferences_get ();

	playlist->view = create_current_playlist_view (playlist);
	playlist->model = g_object_ref (gtk_tree_view_get_model(GTK_TREE_VIEW(playlist->view)));

	/* Setup the scrolled window */

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(playlist),
	                                GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(playlist),
	                                     GTK_SHADOW_IN);
	gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW(playlist), NULL);
	gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW(playlist), NULL);

	/* Attach view to scroll window */

	gtk_container_add (GTK_CONTAINER(playlist), playlist->view);

	/* Init columns */


	init_current_playlist_columns (playlist);

	/* Init pixbufs */

	pragha_playlist_init_pixbuf (playlist);

	/* Init drag and drop */

	init_playlist_dnd (playlist);

	/* Create menus */

	playlist->header_context_menu = create_header_context_menu (playlist);
	playlist->playlist_context_menu = pragha_playlist_context_menu_new (playlist);

	/* Init the rest of flags */

	playlist->rand = g_rand_new();
	playlist->changing = FALSE;
	playlist->dragging = FALSE;
	playlist->track_error = NULL;
	playlist->rand_track_refs = NULL;
	playlist->queue_track_refs = NULL;

	/* Conect signals */

	g_signal_connect (playlist->preferences, "notify::shuffle",
	                  G_CALLBACK (shuffle_changed_cb), playlist);

	g_signal_connect (G_OBJECT(playlist->view), "row-activated",
	                  G_CALLBACK(pragha_playlist_row_activated_cb), playlist);

	g_signal_connect (G_OBJECT(playlist->view), "button-press-event",
	                  G_CALLBACK(pragha_playlist_button_press_cb), playlist);
	g_signal_connect (G_OBJECT(playlist->view), "button-release-event",
	                  G_CALLBACK(current_playlist_button_release_cb), playlist);

	g_signal_connect (G_OBJECT(playlist->cdbase), "PlaylistsChanged",
	                  G_CALLBACK(pragha_playlist_update_menu_playlist_changes), playlist);

	pragha_playlist_update_menu_playlist_changes (playlist->cdbase, playlist);

	gtk_widget_show_all (GTK_WIDGET(playlist));
}

static void
pragha_playlist_dispose (GObject *object)
{
	PraghaPlaylist *playlist = PRAGHA_PLAYLIST (object);

	if (playlist->preferences) {
		g_signal_handlers_disconnect_by_func (playlist->preferences, shuffle_changed_cb, playlist);
		g_object_unref (playlist->preferences);
		playlist->preferences = NULL;
	}

	if (playlist->model) {
		g_object_unref (playlist->model);
		playlist->model = NULL;
	}

	if (playlist->playlist_context_menu) {
		g_object_unref (playlist->playlist_context_menu);
		playlist->playlist_context_menu = NULL;
	}

	if (playlist->playing_pixbuf) {
		g_object_unref (playlist->playing_pixbuf);
		playlist->playing_pixbuf = NULL;
	}

	if (playlist->paused_pixbuf) {
		g_object_unref (playlist->paused_pixbuf);
		playlist->paused_pixbuf = NULL;
	}

	if (playlist->cdbase) {
		g_object_unref (playlist->cdbase);
		playlist->cdbase = NULL;
	}

	(*G_OBJECT_CLASS (pragha_playlist_parent_class)->dispose) (object);
}

static void
pragha_playlist_finalize (GObject *object)
{
	PraghaPlaylist *playlist = PRAGHA_PLAYLIST (object);

	if (playlist->track_error)
		g_error_free (playlist->track_error);

	free_str_list (playlist->columns);
	g_slist_free (playlist->column_widths);

	g_rand_free (playlist->rand);

	(*G_OBJECT_CLASS (pragha_playlist_parent_class)->finalize) (object);
}

static void
pragha_playlist_unrealize (GtkWidget *widget)
{
	PraghaPlaylist *playlist = PRAGHA_PLAYLIST (widget);

	pragha_playlist_save_preferences (playlist);

	(*GTK_WIDGET_CLASS (pragha_playlist_parent_class)->unrealize) (widget);
}

static void
pragha_playlist_class_init (PraghaPlaylistClass *klass)
{
	GObjectClass  *gobject_class;
	GtkWidgetClass *gtkobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->dispose = pragha_playlist_dispose;
	gobject_class->finalize = pragha_playlist_finalize;

	gtkobject_class = GTK_WIDGET_CLASS (klass);
	gtkobject_class->unrealize = pragha_playlist_unrealize;

	/*
	 * Signals:
	 */
	signals[PLAYLIST_SET_TRACK] =
		g_signal_new ("playlist-set-track",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaPlaylistClass, playlist_set_track),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__POINTER,
		              G_TYPE_NONE, 1, G_TYPE_POINTER);

	signals[PLAYLIST_CHANGE_TAGS] =
		g_signal_new ("playlist-change-tags",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaPlaylistClass, playlist_change_tags),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__UINT_POINTER,
		              G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_POINTER);

	signals[PLAYLIST_CHANGED] =
		g_signal_new ("playlist-changed",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (PraghaPlaylistClass, playlist_changed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);
}

PraghaPlaylist *
pragha_playlist_new (void)
{
	return g_object_new (PRAGHA_TYPE_PLAYLIST, NULL);
}
