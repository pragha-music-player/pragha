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

#include "pragha-playlists-mgmt.h"

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <glib.h>
#include <glib/gstdio.h>

#ifdef HAVE_PLPARSER
#include <totem-pl-parser.h>
#else
#include <fcntl.h>
#include "xml_helper.h"
#endif

#include "pragha-hig.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha.h"

/* Playlist management */

typedef enum {
	NEW_PLAYLIST,
	APPEND_PLAYLIST,
	EXPORT_PLAYLIST
} PraghaPlaylistAction;

/* Build a dialog to get a new playlist name */

static gchar *
get_playlist_dialog(PraghaPlaylistActionRange type, GtkWidget *parent)
{
	GtkWidget *dialog;
	GtkWidget *table, *label, *entry;
	gchar *playlist = NULL;
	gint result;
	guint row = 0;

	table = pragha_hig_workarea_table_new();

	if(type == SAVE_COMPLETE)
		pragha_hig_workarea_table_add_section_title(table, &row, _("Save playlist"));
	else
		pragha_hig_workarea_table_add_section_title(table, &row, _("Save selection"));

	label = gtk_label_new_with_mnemonic(_("Playlist"));

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry), 255);
	gtk_entry_set_activates_default (GTK_ENTRY(entry), TRUE);
	gtk_widget_grab_focus(GTK_WIDGET(entry));

	pragha_hig_workarea_table_add_row(table, &row, label, entry);

	dialog = gtk_dialog_new_with_buttons (NULL,
	                                     GTK_WINDOW(parent),
	                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                     _("_Cancel"), GTK_RESPONSE_CANCEL,
	                                     _("_Ok"), GTK_RESPONSE_ACCEPT,
	                                     NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	if(type == SAVE_COMPLETE)
		gtk_window_set_title (GTK_WINDOW(dialog), _("Save playlist"));
	else
		gtk_window_set_title (GTK_WINDOW(dialog), _("Save selection"));

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table);
	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result) {
		case GTK_RESPONSE_ACCEPT:
			playlist = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
			break;
		case GTK_RESPONSE_CANCEL:
			break;
		default:
			break;
	}
	gtk_widget_destroy(dialog);

	return playlist;
}

/* Get a new playlist name that is not reserved */

gchar *
get_playlist_name(PraghaPlaylistActionRange type, GtkWidget *parent)
{
	gchar *playlist = NULL;

	do {
		playlist = get_playlist_dialog(type, parent);
		if (playlist && !g_ascii_strcasecmp(playlist, SAVE_PLAYLIST_STATE)) {
			GtkWidget *dialog;
			dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(parent),
			                                            GTK_DIALOG_MODAL,
			                                            GTK_MESSAGE_INFO,
			                                            GTK_BUTTONS_OK,
			                                            _("<b>con_playlist</b> is a reserved playlist name"));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			g_free(playlist);
			continue;
		}
		else {
			break;
		}
	} while (1);

	return playlist;
}

static gboolean
overwrite_existing_playlist(const gchar *playlist, GtkWidget *parent)
{
	GtkWidget *dialog;
	gint response;

	dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
	                                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                GTK_MESSAGE_QUESTION,
	                                GTK_BUTTONS_YES_NO,
	                                _("Do you want to overwrite the playlist: %s ?"), playlist);

	response = gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

	return (response == GTK_RESPONSE_YES);
}

GIOChannel*
create_m3u_playlist(gchar *file)
{
	GIOChannel *chan = NULL;
	GIOStatus status;
	GError *err = NULL;
	gsize bytes = 0;

	chan = g_io_channel_new_file(file, "w+", &err);
	if (!chan) {
		g_critical("Unable to create M3U playlist IO channel: %s", file);
		goto exit_failure;
	}

	status = g_io_channel_write_chars(chan, "#EXTM3U\n", -1, &bytes, &err);
	if (status != G_IO_STATUS_NORMAL) {
		g_critical("Unable to write to M3U playlist: %s", file);
		goto exit_failure;
	}

	CDEBUG(DBG_INFO, "Created M3U playlist file: %s", file);
	return chan;

exit_failure:
	g_error_free(err);
	err = NULL;

	if (chan) {
		g_io_channel_shutdown(chan, FALSE, &err);
		g_io_channel_unref(chan);
	}

	return NULL;
}

static gint
save_mobj_list_to_m3u_playlist(GList *list, GIOChannel *chan, gchar *filename)
{
	gchar *str = NULL, *uri = NULL, *base_m3u = NULL, *base = NULL;
	PraghaMusicobject *mobj = NULL;
	GIOStatus status;
	gsize bytes = 0;
	GError *err = NULL;
	GList *i;
	gint ret = 0;

	base_m3u = get_display_filename(filename, TRUE);

	/* Export all selected tracks to the given file */
	for (i=list; i != NULL; i = i->next) {
		mobj = i->data;
		if (pragha_musicobject_is_local_file(mobj)) {
			base = get_display_filename(pragha_musicobject_get_file(mobj), TRUE);

			if (g_ascii_strcasecmp(base_m3u, base) == 0)
				uri = get_display_filename(pragha_musicobject_get_file(mobj), FALSE);
			else
				uri = g_strdup(pragha_musicobject_get_file(mobj));

			/* Format: "#EXTINF:seconds, title" */
			str = g_strdup_printf("#EXTINF:%d,%s\n%s\n",
					      pragha_musicobject_get_length(mobj),
					      pragha_musicobject_get_title(mobj),
					      uri);

			status = g_io_channel_write_chars(chan, str, -1, &bytes, &err);
			if (status != G_IO_STATUS_NORMAL) {
				g_critical("Unable to write to M3U playlist: %s", filename);
				ret = -1;
				goto exit_list;
			}
			g_free(base);
			g_free(uri);
		}

		/* Have to give control to GTK periodically ... */
		pragha_process_gtk_events ();
	}

exit_list:
	g_free(base_m3u);
	if (err) {
		g_error_free(err);
		err = NULL;
	}
	return ret;
}

#ifdef HAVE_PLPARSER
static void
pragha_parser_append_foreach_playlist (GtkTreeModel *model,
                                       GtkTreePath  *path,
                                       GtkTreeIter  *iter,
                                       gpointer      data)
{
	TotemPlPlaylistIter pl_iter;
	PraghaMusicobject *mobj;
	const gchar *filename;
	gchar *uri = NULL;

	TotemPlPlaylist *playlist = data;

	gtk_tree_model_get (model, iter, P_MOBJ_PTR, &mobj, -1);

	filename = pragha_musicobject_get_file(mobj);

	uri = g_filename_to_uri (filename, NULL, NULL);

	totem_pl_playlist_append (playlist, &pl_iter);
	totem_pl_playlist_set (playlist, &pl_iter,
	                       TOTEM_PL_PARSER_FIELD_URI, uri,
	                       NULL);

	g_free(uri);
}

static gboolean
pragha_parser_append_foreach_track_list (GtkTreeModel *model,
                                         GtkTreePath  *path,
                                         GtkTreeIter  *iter,
                                         gpointer      data)
{
	pragha_parser_append_foreach_playlist (model,
		                                   path,
		                                   iter,
		                                   data);
	return FALSE;
}

static gboolean
pragha_parser_save_full_track_list (PraghaPlaylist *cplaylist,
                                    const gchar    *filename)
{
	TotemPlPlaylist *playlist;
	TotemPlParser *pl;
	GFile *file;
	gboolean ret = TRUE;

	pl = totem_pl_parser_new ();
	playlist = totem_pl_playlist_new ();
	file = g_file_new_for_path (filename);

	gtk_tree_model_foreach(pragha_playlist_get_model(cplaylist),
	                       pragha_parser_append_foreach_track_list,
	                       playlist);


	if (totem_pl_parser_save (pl, playlist, file, "Title", TOTEM_PL_PARSER_M3U, NULL) != TRUE) {
		g_error ("Playlist writing failed.");
		ret = FALSE;
    }

	g_object_unref (playlist);
	g_object_unref (pl);
	g_object_unref (file);

	return ret;
}

static gboolean
pragha_parser_save_selection_track_list (PraghaPlaylist *cplaylist,
                                         const gchar    *filename)
{
	TotemPlPlaylist *playlist;
	TotemPlParser *pl;
	GtkTreeSelection *selection;
	GFile *file;
	gboolean ret = TRUE;

	pl = totem_pl_parser_new ();
	playlist = totem_pl_playlist_new ();
	file = g_file_new_for_path (filename);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(pragha_playlist_get_view(cplaylist)));
	gtk_tree_selection_selected_foreach(selection,
	                                    pragha_parser_append_foreach_playlist,
	                                    playlist);

	if (totem_pl_parser_save (pl, playlist, file, "Title", TOTEM_PL_PARSER_M3U, NULL) != TRUE) {
        g_error ("Playlist writing failed.");
        ret = FALSE;
    }

	g_object_unref (playlist);
	g_object_unref (pl);
	g_object_unref (file);

	return ret;
}

#else
static gint
save_complete_m3u_playlist(PraghaPlaylist* cplaylist, GIOChannel *chan, gchar *filename)
{
	gint ret = 0;
	GList *list = NULL;

	list = pragha_playlist_get_mobj_list(cplaylist);

	if(list != NULL) {
		ret = save_mobj_list_to_m3u_playlist(list, chan, filename);
		g_list_free(list);
	}

	return ret;
}

static gint
save_selected_to_m3u_playlist(PraghaPlaylist* cplaylist, GIOChannel *chan, gchar *filename)
{
	GList *list = NULL;
	gint ret = 0;

	list = pragha_playlist_get_selection_mobj_list(cplaylist);

	if (list != NULL) {
		ret = save_mobj_list_to_m3u_playlist(list, chan, filename);
		g_list_free(list);
	}

	return ret;
}
#endif

gint
save_m3u_playlist(GIOChannel *chan, gchar *playlist, gchar *filename, PraghaDatabase *cdbase)
{
	GList *list = NULL;
	gint ret = 0;

	list = add_playlist_to_mobj_list(cdbase, playlist, list);

	if (list != NULL) {
		ret = save_mobj_list_to_m3u_playlist(list, chan, filename);

		g_list_free_full (list, (GDestroyNotify) g_object_unref);
	}

	return ret;
}

/**********************/
/* External functions */
/**********************/

/* Append the given playlist to the current playlist */

void add_playlist_current_playlist(gchar *splaylist, PraghaApplication *pragha)
{
	PraghaPlaylist *playlist;
	PraghaDatabase *cdbase;
	GList *list = NULL;

	cdbase = pragha_application_get_database (pragha);
	list = add_playlist_to_mobj_list (cdbase, splaylist, list);

	if(list) {
		playlist = pragha_application_get_playlist (pragha);
		pragha_playlist_append_mobj_list (playlist, list);
		g_list_free (list);
	}
}

/* Append the given playlist to the mobj list. */

GList *
add_playlist_to_mobj_list(PraghaDatabase *cdbase,
                          const gchar *playlist,
                          GList *list)
{
	gint playlist_id, location_id;
	PraghaMusicobject *mobj;

	playlist_id = pragha_database_find_playlist (cdbase, playlist);

	if(playlist_id == 0)
		goto bad;

	const gchar *sql = "SELECT file FROM PLAYLIST_TRACKS WHERE playlist = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (cdbase, sql);
	pragha_prepared_statement_bind_int (statement, 1, playlist_id);

	while (pragha_prepared_statement_step (statement)) {
		const gchar *file = pragha_prepared_statement_get_string (statement, 0);

		if ((location_id = pragha_database_find_location (cdbase, file)))
			mobj = new_musicobject_from_db(cdbase, location_id);
		else
			mobj = new_musicobject_from_file (file, NULL);

		if (G_LIKELY(mobj))
			list = g_list_append(list, mobj);
	}

	pragha_prepared_statement_free (statement);
bad:

	return list;
}

/* Append the given radio to the mobj list. */

GList *
add_radio_to_mobj_list(PraghaDatabase *cdbase,
                       const gchar *radio,
                       GList *list)
{
	gint radio_id;
	PraghaMusicobject *mobj;

	radio_id = pragha_database_find_radio (cdbase, radio);

	if(radio_id == 0)
		goto bad;

	const gchar *sql = "SELECT uri FROM RADIO_TRACKS WHERE radio = ?";
	PraghaPreparedStatement *statement = pragha_database_create_statement (cdbase, sql);
	pragha_prepared_statement_bind_int (statement, 1, radio_id);

	while (pragha_prepared_statement_step (statement)) {
		const gchar *uri = pragha_prepared_statement_get_string (statement, 0);
		mobj = new_musicobject_from_location (uri, radio);
		if (G_LIKELY(mobj))
			list = g_list_append(list, mobj);
	}

	pragha_prepared_statement_free (statement);
bad:

	return list;
}

/* Build a dialog to get a new playlist name */

gchar* rename_playlist_dialog(const gchar *oplaylist, GtkWidget *parent)
{
	GtkWidget *dialog;
	GtkWidget *table, *entry;
	gchar *playlist = NULL;
	gint result;
	guint row = 0;

	/* Create dialog window */

	table = pragha_hig_workarea_table_new();
	pragha_hig_workarea_table_add_section_title(table, &row, _("Choose a new name"));

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry), 255);
	gtk_entry_set_activates_default (GTK_ENTRY(entry), TRUE);
	pragha_hig_workarea_table_add_wide_control(table, &row, entry);

	dialog = gtk_dialog_new_with_buttons (_("Rename"),
	                                      GTK_WINDOW(parent),
	                                      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                      _("_Cancel"), GTK_RESPONSE_CANCEL,
	                                      _("_Ok"), GTK_RESPONSE_ACCEPT,
	                                      NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	gtk_entry_set_text(GTK_ENTRY(entry), oplaylist);

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table);

	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result) {
	case GTK_RESPONSE_ACCEPT:
		/* Get playlist name */
		playlist = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
		break;
	case GTK_RESPONSE_CANCEL:
		break;
	default:
		break;
	}
	gtk_widget_destroy(dialog);

	return playlist;
}

gboolean
delete_existing_item_dialog(const gchar *item, GtkWidget *parent)
{
	gboolean choice = FALSE;
	gint ret;
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				_("Do you want to delete the item: %s ?"),
				item);

	ret = gtk_dialog_run(GTK_DIALOG(dialog));

	switch(ret) {
	case GTK_RESPONSE_YES:
		choice = TRUE;
		break;
	case GTK_RESPONSE_NO:
		choice = FALSE;
		break;
	default:
		break;
	}

	gtk_widget_destroy(dialog);

	return choice;
}

/* Export selection/current playlist to a M3U file */

gchar *
playlist_export_dialog_get_filename(const gchar *prefix, GtkWidget *parent)
{
	GtkWidget *dialog;
	gchar *filename = NULL, *playlistm3u = NULL;
	gint resp;

	dialog = gtk_file_chooser_dialog_new (_("Export playlist to file"),
	                                      GTK_WINDOW(parent),
	                                      GTK_FILE_CHOOSER_ACTION_SAVE,
	                                      _("_Cancel"), GTK_RESPONSE_CANCEL,
	                                      _("_Save"), GTK_RESPONSE_ACCEPT,
	                                      NULL);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
	                                               TRUE);

	playlistm3u = g_strdup_printf("%s.m3u", prefix);
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(dialog), playlistm3u);
	g_free(playlistm3u);

	resp = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (resp) {
		case GTK_RESPONSE_ACCEPT:
			filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
			break;
		default:
			break;
	}
	gtk_widget_destroy(dialog);

	return filename;
}

#ifdef HAVE_PLPARSER
void export_playlist (PraghaPlaylist* cplaylist, PraghaPlaylistActionRange choice)
{
	gchar *filename = NULL;

	filename = playlist_export_dialog_get_filename(_("Playlists"),
	                                               gtk_widget_get_toplevel(GTK_WIDGET(cplaylist)));
	if (!filename)
		return;

	switch(choice) {
	case SAVE_COMPLETE:
		if (pragha_parser_save_full_track_list(cplaylist, filename) == FALSE)
			g_warning("Unable to save M3U playlist: %s", filename);
		break;
	case SAVE_SELECTED:
		if (pragha_parser_save_selection_track_list(cplaylist, filename) == FALSE)
			g_warning("Unable to save M3U playlist: %s", filename);
		break;
	default:
		break;
	}

	g_free(filename);
}
#else
void export_playlist (PraghaPlaylist* cplaylist, PraghaPlaylistActionRange choice)
{
	gchar *filename = NULL;
	GIOChannel *chan = NULL;
	GError *err = NULL;

	filename = playlist_export_dialog_get_filename(_("Playlists"),
	                                               gtk_widget_get_toplevel(GTK_WIDGET(cplaylist)));

	if (!filename)
		goto exit;

	chan = create_m3u_playlist(filename);
	if (!chan) {
		g_warning("Unable to create M3U playlist file: %s", filename);
		goto exit;
	}

	switch(choice) {
	case SAVE_COMPLETE:
		if (save_complete_m3u_playlist(cplaylist, chan, filename) < 0) {
			g_warning("Unable to save M3U playlist: %s", filename);
			goto exit;
		}
		break;
	case SAVE_SELECTED:
		if (save_selected_to_m3u_playlist(cplaylist, chan, filename) < 0) {
			g_warning("Unable to save M3U playlist: %s", filename);
			goto exit;
		}
		break;
	default:
		break;
	}

	if (chan) {
		if (g_io_channel_shutdown(chan, TRUE, &err) != G_IO_STATUS_NORMAL) {
			g_critical("Unable to save M3U playlist: %s", filename);
			g_error_free(err);
			err = NULL;
		} else {
			CDEBUG(DBG_INFO, "Saved M3U playlist: %s", filename);
		}
		g_io_channel_unref(chan);
	}

exit:
	g_free(filename);
}
#endif

#ifdef HAVE_PLPARSER
static void
_on_pl_entry_parsed (TotemPlParser *parser,
                     gchar         *uri,
                     gpointer       metadata,
                     GSList       **plitems)
{
	gchar *filename = NULL;

	if (uri != NULL) {
		/* Convert the uri into a filename to taglib.*/
		if (g_str_has_prefix (uri, "file:"))
			filename = g_filename_from_uri (uri, NULL, NULL);
		else
			filename = g_strdup(uri);

		*plitems = g_slist_append(*plitems, filename);
	}
}

GSList *
pragha_totem_pl_parser_parse_from_uri (const gchar *uri)
{
	TotemPlParser *pl_parser = NULL;
	GSList *plitems = NULL;
	gchar *base;

	pl_parser = totem_pl_parser_new ();
	//g_object_set (pl_parser, "recurse", FALSE, NULL);
	g_signal_connect (G_OBJECT(pl_parser), "entry-parsed",
	                  G_CALLBACK(_on_pl_entry_parsed), &plitems);

	base = get_display_filename(uri, TRUE);

	switch (totem_pl_parser_parse_with_base(pl_parser, uri, base, FALSE)) {
		case TOTEM_PL_PARSER_RESULT_UNHANDLED:
		case TOTEM_PL_PARSER_RESULT_IGNORED:
			/* maybe it's the actual stream URL, then */
			plitems = g_slist_append(plitems, g_strdup(uri));
			break;
		case TOTEM_PL_PARSER_RESULT_ERROR:
			g_warning ("An error happens while parsing %s", uri);
			break;
		case TOTEM_PL_PARSER_RESULT_SUCCESS:
		default:
			break;
	}
	g_object_unref (pl_parser);

	g_free(base);

	return plitems;
}
#else
GSList *
pragha_pl_parser_parse_xspf (const gchar *filename)
{
	XMLNode *xml = NULL, *xi, *xl;
	gchar *contents, *f_file, *uri, *base;
	GSList *list = NULL;
	GError *err = NULL;
	GFile *file;
	gsize size;

	file = g_file_new_for_path (filename);

	if (!g_file_load_contents (file, NULL, &contents, &size, NULL, NULL)) {
		goto out;
    	}

	if (g_utf8_validate (contents, -1, NULL) == FALSE) {
		gchar *fixed;
		fixed = g_convert (contents, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
		if (fixed != NULL) {
			g_free (contents);
			contents = fixed;
		}
	}

	base = get_display_filename(filename, TRUE);

	xml = tinycxml_parse(contents);

	xi = xmlnode_get(xml,CCA { "playlist","trackList","track",NULL},NULL,NULL);
	for(;xi;xi= xi->next) {
		xl = xmlnode_get(xi,CCA {"track","location",NULL},NULL,NULL);

		if (xl && xl->content) {
			f_file = g_filename_from_uri(xl->content, NULL, &err);

			if (!f_file) {
				g_warning("Unable to get filename from UTF-8 string: %s",
					  xl->content);
				g_error_free(err);
				err = NULL;
				continue;
			}

			if (g_path_is_absolute(f_file))
				uri = g_strdup(f_file);
			else {
				uri = g_build_filename (base, f_file, NULL);
			}
			list = g_slist_append (list, uri);
			g_free(f_file);
		}
	}

	xmlnode_free(xml);
	g_free(contents);
	g_free(base);

out:
	g_object_unref (file);
	
	return list;
}

GSList *
pragha_pl_parser_parse_pls (const gchar *file)
{
	GKeyFile *plskeyfile;
	GError *error = NULL;
	GSList *list = NULL;
	guint i, nentries;
	gchar key[128], *file_entry = NULL, *uri = NULL, *base = NULL;

	base = get_display_filename(file, TRUE);

	plskeyfile = g_key_file_new();

	if (!g_key_file_load_from_file(plskeyfile, file, G_KEY_FILE_NONE, &error)) {
		g_critical("Unable to load pls playlist, err: %s", error->message);
	}
	else {
		nentries = g_key_file_get_integer (plskeyfile, "playlist", "NumberOfEntries", NULL);
		if (nentries > 0) {
			for (i = 1; i <= nentries; i++) {
				g_snprintf (key, 128, "File%d", i);
				file_entry = g_key_file_get_string(plskeyfile, "playlist", key, NULL);
				if (NULL == file_entry)
					continue;

				if (g_path_is_absolute(file_entry))
					uri = g_strdup(file_entry);
				else {
					uri = g_build_filename (base, file_entry, NULL);
				}
				list = g_slist_append (list, uri);
				g_free(file_entry);
			}
		}
	}
	g_key_file_free (plskeyfile);
	g_free(base);

	return list;
}

/* Load a M3U playlist, and add tracks to current playlist */

GSList *
pragha_pl_parser_parse_m3u (const gchar *file)
{
	GError *err = NULL;
	GIOChannel *chan = NULL;
	gint fd;
	gsize len, term;
	gchar *str, *filename, *f_file, *uri, *base;
	GSList *list = NULL;

	fd = g_open(file, O_RDONLY, 0);
	if (fd == -1) {
		g_critical("Unable to open file : %s",
			   file);
		return NULL;
	}

	chan = g_io_channel_unix_new(fd);
	if (!chan) {
		g_critical("Unable to open an IO channel for file: %s", file);
		goto exit_close;
	}

	base = get_display_filename(file, TRUE);

	while (g_io_channel_read_line(chan, &str, &len, &term, &err) ==
	       G_IO_STATUS_NORMAL) {

		if (!str || !len)
			break;

		/* Skip lines containing #EXTM3U or #EXTINF */

		if (g_strrstr(str, "#EXTM3U") || g_strrstr(str, "#EXTINF")) {
			g_free(str);
			continue;
		}

		filename = g_strndup(str, term);

		f_file = g_filename_from_utf8(filename, -1, NULL, NULL, &err);
		if (!f_file) {
			g_warning("Unable to get filename from UTF-8 string: %s",
				  filename);
			g_error_free(err);
			err = NULL;
			goto continue_read;
		}

		if (g_path_is_absolute(f_file))
			uri = g_strdup(f_file);
		else {
			uri = g_build_filename (base, f_file, NULL);
		}
		list = g_slist_append (list, uri);

		pragha_process_gtk_events ();

		g_free(f_file);
	continue_read:
		g_free(filename);
		g_free(str);
	}

	CDEBUG(DBG_INFO, "Loaded M3U playlist: %s", file);

	g_free(base);

	if (g_io_channel_shutdown(chan, TRUE, &err) != G_IO_STATUS_NORMAL) {
		g_critical("Unable to open M3U playlist: %s", file);
		g_error_free(err);
		err = NULL;
	}
	g_io_channel_unref(chan);

exit_close:
	close(fd);

	return list;
}

GSList *
pragha_pl_parser_parse (PraghaPlaylistType format, const gchar *filename)
{
	GSList *list = NULL;

	switch (format)
	{
	case PL_FORMAT_M3U:
		list = pragha_pl_parser_parse_m3u (filename);
		break;
	case PL_FORMAT_PLS:
		list = pragha_pl_parser_parse_pls (filename);
		break;
	case PL_FORMAT_ASX:
		//list = pragha_pl_parser_parse_asx (filename);
		break;
	case PL_FORMAT_XSPF:
		list = pragha_pl_parser_parse_xspf (filename);
		break;
	default:
	break;
    }

    return list;
}

GSList *pragha_pl_parser_parse_from_file_by_extension (const gchar *filename)
{
	PraghaPlaylistType format = PL_FORMAT_UNKNOWN;
	GSList *list = NULL;

	if ((format = pragha_pl_parser_guess_format_from_extension (filename)) != PL_FORMAT_UNKNOWN) {
		list = pragha_pl_parser_parse (format, filename);
	}
	else {
		g_debug ("Unable to guess playlist format : %s", filename);
	}

	return list;
}
#endif

GList *
pragha_pl_parser_append_mobj_list_by_extension (GList *mlist, const gchar *file)
{
	GSList *list = NULL, *i = NULL;
	PraghaMusicobject *mobj;

#ifdef HAVE_PLPARSER
	gchar *uri = g_filename_to_uri (file, NULL, NULL);
	list = pragha_totem_pl_parser_parse_from_uri(uri);
	g_free (uri);
#else
	list = pragha_pl_parser_parse_from_file_by_extension (file);
#endif

	for (i = list; i != NULL; i = i->next) {
		mobj = new_musicobject_from_file(i->data, NULL);
		if (G_LIKELY(mobj))
			mlist = g_list_append(mlist, mobj);

		pragha_process_gtk_events ();

		g_free(i->data);
	}
	g_slist_free(list);

	return mlist;
}

void pragha_pl_parser_open_from_file_by_extension (const gchar *file, PraghaApplication *pragha)
{
	PraghaPlaylist *playlist;
	GSList *list = NULL, *i = NULL;
	GList *mlist = NULL;
	gchar *summary;
	gint try = 0, added = 0;
	PraghaStatusbar *statusbar;
	PraghaMusicobject *mobj;

#ifdef HAVE_PLPARSER
	gchar *uri = g_filename_to_uri (file, NULL, NULL);
	list = pragha_totem_pl_parser_parse_from_uri(uri);
	g_free (uri);
#else
	list = pragha_pl_parser_parse_from_file_by_extension (file);
#endif

	for (i = list; i != NULL; i = i->next) {
		try++;
		mobj = new_musicobject_from_file(i->data, NULL);
		if (G_LIKELY(mobj)) {
			added++;
			mlist = g_list_append(mlist, mobj);
		}

		pragha_process_gtk_events ();

		g_free(i->data);
	}

	playlist = pragha_application_get_playlist (pragha);
	pragha_playlist_append_mobj_list (playlist, mlist);

	summary = g_strdup_printf(_("Added %d songs from %d of the imported playlist."), added, try);

	statusbar = pragha_statusbar_get ();
	pragha_statusbar_set_misc_text (statusbar, summary);
	g_object_unref (statusbar);

	g_free(summary);

	g_slist_free(list);
	g_list_free(mlist);
}

gchar *
pragha_pl_get_first_playlist_item (const gchar *uri)
{
	gchar *file = NULL;
#ifdef HAVE_PLPARSER
	GSList *list = pragha_totem_pl_parser_parse_from_uri(uri);
	if (list != NULL) {
		file = g_strdup(list->data);
		g_slist_free_full(list, g_free);
	}
	else {
		file = g_strdup(uri);
	}
#else
	/* TODO: Check if local uri and parse it */
	file = g_strdup(uri);
#endif
	return file;
}

/* Save tracks to a playlist using the given type */

void
save_playlist(PraghaPlaylist* cplaylist,
              gint playlist_id,
              PraghaPlaylistActionRange type)
{
	PraghaDatabase *cdbase = NULL;
	GList *mlist = NULL, *i;
	const gchar *filename = NULL;

	switch(type) {
	case SAVE_COMPLETE:
		mlist = pragha_playlist_get_mobj_list(cplaylist);
		break;
	case SAVE_SELECTED:
		mlist = pragha_playlist_get_selection_mobj_list(cplaylist);
		break;
	default:
		break;
	}

	cdbase = pragha_playlist_get_database (cplaylist);
	pragha_database_begin_transaction (cdbase);
	if (mlist != NULL)
	{
		for (i = mlist; i != NULL; i = i->next)
		{
			filename = pragha_musicobject_get_file (PRAGHA_MUSICOBJECT(i->data));
			if(pragha_musicobject_get_source (PRAGHA_MUSICOBJECT(i->data)) == FILE_HTTP)
			{
				/* TODO: Fix this negradaaa!. */
				gchar *file = g_strdup_printf("Radio:/%s", filename);
				pragha_database_add_playlist_track (cdbase, playlist_id, file);
				g_free (file);
			}
			else
			{
				filename = pragha_musicobject_get_file (PRAGHA_MUSICOBJECT(i->data));
				pragha_database_add_playlist_track (cdbase, playlist_id, filename);
			}
		}
		g_list_free(mlist);
	}
	pragha_database_commit_transaction (cdbase);
}

void
new_playlist(PraghaPlaylist* cplaylist,
             const gchar *playlist,
             PraghaPlaylistActionRange type)
{
	gint playlist_id = 0;

	if (string_is_empty(playlist)) {
		g_warning("Playlist name is NULL");
		return;
	}

	if ((playlist_id = pragha_database_find_playlist (pragha_playlist_get_database(cplaylist), playlist))) {
		if (overwrite_existing_playlist(playlist, gtk_widget_get_toplevel(GTK_WIDGET(cplaylist))))
			pragha_database_delete_playlist (pragha_playlist_get_database(cplaylist), playlist);
		else
			return;
	}

	playlist_id = pragha_database_add_new_playlist (pragha_playlist_get_database(cplaylist), playlist);
	save_playlist(cplaylist, playlist_id, type);
}

void append_playlist(PraghaPlaylist* cplaylist, const gchar *playlist, PraghaPlaylistActionRange type)
{
	gint playlist_id;

	if (string_is_empty(playlist)) {
		g_warning("Playlist name is NULL");
		return;
	}

	playlist_id = pragha_database_find_playlist (pragha_playlist_get_database(cplaylist), playlist);

	if (!playlist_id) {
		g_warning("Playlist doesn't exist\n");
		return;
	}

	save_playlist(cplaylist, playlist_id, type);
}

gchar *
new_radio (PraghaPlaylist *playlist,
           const gchar    *uri,
           const gchar    *basename)
{
	PraghaDatabase *cdbase;
	gchar *name = NULL;
	gint radio_id = 0, i = 0;

	if (string_is_empty(basename)) {
		g_warning("Radio name is NULL");
		return NULL;
	}

	cdbase = pragha_playlist_get_database(playlist);

	if (!pragha_database_find_radio (cdbase, basename)) {
		name = g_strdup (basename);
	}
	else {
		/* Get a new name */
		do {
			if (name)
				g_free (name);
			name = g_strdup_printf ("%s %i", basename, ++i);
		} while (pragha_database_find_radio (cdbase, name));
	}

  	/* Save a new radio */

  	radio_id = pragha_database_add_new_radio (cdbase, name);
	pragha_database_add_radio_track (cdbase, radio_id, uri);

	return name;
}

PraghaPlaylistAction
replace_or_append_dialog(PraghaPlaylist *cplaylist, const gchar *playlist, PraghaPlaylistActionRange type)
{
	GtkWidget *dialog;
	GtkWidget *table, *radio_replace, *radio_add;
	gchar *string_options = NULL;
	gint result;
	guint row = 0;
	PraghaPlaylistAction choise = EXPORT_PLAYLIST;

	/* Create dialog window */

	table = pragha_hig_workarea_table_new();

	pragha_hig_workarea_table_add_section_title(table, &row, _("What do you want to do?"));

	string_options = g_strdup_printf(_("Replace the playlist \"%s\""), playlist);
	radio_replace = gtk_radio_button_new_with_label_from_widget (NULL, string_options);
	pragha_hig_workarea_table_add_wide_control(table, &row, radio_replace);
	g_free(string_options);

	string_options = g_strdup_printf(_("Add to playlist \"%s\""), playlist);
	radio_add = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON(radio_replace), string_options);
	pragha_hig_workarea_table_add_wide_control(table, &row, radio_add);
	g_free(string_options);

	dialog = gtk_dialog_new_with_buttons (NULL,
	                                      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(cplaylist))),
	                                      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                      _("_Cancel"), GTK_RESPONSE_CANCEL,
	                                      _("_Ok"), GTK_RESPONSE_ACCEPT,
	                                      NULL);

	if(type == SAVE_COMPLETE)
		gtk_window_set_title (GTK_WINDOW(dialog), _("Save playlist"));
	else
		gtk_window_set_title (GTK_WINDOW(dialog), _("Save selection"));

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table);

	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result) {
		case GTK_RESPONSE_ACCEPT:
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_replace)))
				choise = NEW_PLAYLIST;
			else
				choise = APPEND_PLAYLIST;
			break;
		case GTK_RESPONSE_CANCEL:
		default:
			break;
	}
	gtk_widget_destroy(dialog);

	return choise;
}


void
pragha_playlist_save_selection (PraghaPlaylist *playlist, const gchar *name)
{
	PraghaPlaylistAction choise;

	choise = replace_or_append_dialog (playlist, name, SAVE_SELECTED);
	switch(choise) {
		case NEW_PLAYLIST:
			new_playlist (playlist, name, SAVE_SELECTED);
			break;
		case APPEND_PLAYLIST:
			append_playlist (playlist, name, SAVE_SELECTED);
			break;
		default:
			break;
	}
}

void
pragha_playlist_save_playlist (PraghaPlaylist *playlist, const gchar *name)
{
	PraghaPlaylistAction choise;

	choise = replace_or_append_dialog (playlist, name, SAVE_COMPLETE);
	switch(choise) {
		case NEW_PLAYLIST:
			new_playlist (playlist, name, SAVE_COMPLETE);
			break;
		case APPEND_PLAYLIST:
			append_playlist (playlist, name, SAVE_COMPLETE);
			break;
		default:
			break;
	}
}

static void
playlist_save_selection (GtkMenuItem *menuitem, PraghaPlaylist *cplaylist)
{
	const gchar *playlist;
	playlist = gtk_menu_item_get_label (menuitem);
	pragha_playlist_save_selection (cplaylist, playlist);
}

static void
playlist_save_complete_playlist (GtkMenuItem *menuitem, PraghaPlaylist *cplaylist)
{
	const gchar *playlist;
	playlist = gtk_menu_item_get_label (menuitem);
	pragha_playlist_save_playlist (cplaylist, playlist);
}

static void
update_playlist_changes_save_selection_popup_playlist (PraghaPlaylist *cplaylist)
{
	GtkWidget *submenu, *menuitem;
	
	submenu = gtk_menu_new ();

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (gtk_ui_manager_get_widget (pragha_playlist_get_context_menu(cplaylist), "/SelectionPopup/Save selection")), submenu);

	menuitem = gtk_image_menu_item_new_with_label (_("New playlist"));
	g_signal_connect(menuitem, "activate", G_CALLBACK(save_selected_playlist), cplaylist);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_image_menu_item_new_with_label (_("Export"));
	g_signal_connect(menuitem, "activate", G_CALLBACK(export_selected_playlist), cplaylist);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	const gchar *sql = "SELECT name FROM PLAYLIST WHERE name != ? ORDER BY name COLLATE NOCASE";
	PraghaPreparedStatement *statement = pragha_database_create_statement (pragha_playlist_get_database(cplaylist), sql);
	pragha_prepared_statement_bind_string (statement, 1, SAVE_PLAYLIST_STATE);

	while (pragha_prepared_statement_step (statement)) {
		const gchar *name = pragha_prepared_statement_get_string (statement, 0);
		menuitem = gtk_image_menu_item_new_with_label (name);
		g_signal_connect (menuitem, "activate", G_CALLBACK(playlist_save_selection), cplaylist);
		gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menuitem);
	}

	pragha_prepared_statement_free (statement);

	gtk_widget_show_all (submenu);
}

static void
update_playlist_changes_save_playlist_popup_playlist (PraghaPlaylist *cplaylist)
{
	GtkWidget *submenu, *menuitem;
	
	submenu = gtk_menu_new ();

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (gtk_ui_manager_get_widget (pragha_playlist_get_context_menu(cplaylist), "/SelectionPopup/Save playlist")), submenu);

	menuitem = gtk_image_menu_item_new_with_label (_("New playlist"));
	g_signal_connect(menuitem, "activate", G_CALLBACK(save_current_playlist), cplaylist);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_image_menu_item_new_with_label (_("Export"));
	g_signal_connect(menuitem, "activate", G_CALLBACK(export_current_playlist), cplaylist);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	const gchar *sql = "SELECT name FROM PLAYLIST WHERE name != ? ORDER BY name COLLATE NOCASE";
	PraghaPreparedStatement *statement = pragha_database_create_statement (pragha_playlist_get_database(cplaylist), sql);
	pragha_prepared_statement_bind_string (statement, 1, SAVE_PLAYLIST_STATE);

	while (pragha_prepared_statement_step (statement)) {
		const gchar *name = pragha_prepared_statement_get_string (statement, 0);
		menuitem = gtk_image_menu_item_new_with_label (name);
		g_signal_connect (menuitem, "activate", G_CALLBACK(playlist_save_complete_playlist), cplaylist);
		gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menuitem);
	}

	pragha_prepared_statement_free (statement);

	gtk_widget_show_all (submenu);
}

void
update_playlist_changes_on_menu (PraghaPlaylist *playlist)
{
	update_playlist_changes_save_playlist_popup_playlist (playlist);
	update_playlist_changes_save_selection_popup_playlist (playlist);
}
