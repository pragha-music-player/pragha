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

#ifdef HAVE_PLPARSER
#include <totem-pl-parser.h>
#else
#include <fcntl.h>
#include "xml_helper.h"
#endif

#include "pragha-playlists-mgmt.h"
#include "pragha-hig.h"
#include "pragha-file-utils.h"
#include "pragha-utils.h"
#include "pragha-musicobject-mgmt.h"
#include "pragha-debug.h"
#include "pragha.h"

/* Build a dialog to get a new playlist name */

static gchar *
get_playlist_dialog(enum playlist_mgmt type, GtkWidget *parent)
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
	pragha_hig_workarea_table_finish(table, &row);

	dialog = gtk_dialog_new_with_buttons(NULL,
			     GTK_WINDOW(parent),
			     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			     GTK_STOCK_CANCEL,
			     GTK_RESPONSE_CANCEL,
			     GTK_STOCK_OK,
			     GTK_RESPONSE_ACCEPT,
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
get_playlist_name(enum playlist_mgmt type, GtkWidget *parent)
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
		if (pragha_process_gtk_events ()) {
			g_list_free(list);
			return 0;
		}
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
typedef struct {
	TotemPlPlaylist *playlist;
	gchar           *folder_saved;
} PraghaPlParser;

static void
pragha_parser_append_foreach_playlist (GtkTreeModel *model,
                                       GtkTreePath  *path,
                                       GtkTreeIter  *iter,
                                       gpointer      data)
{
	TotemPlPlaylistIter pl_iter;
	PraghaMusicobject *mobj;
	const gchar *filename;
	gchar *base_uri = NULL, *uri = NULL;

	PraghaPlParser *parser = data;

	gtk_tree_model_get (model, iter, P_MOBJ_PTR, &mobj, -1);

	filename = pragha_musicobject_get_file(mobj);
	base_uri = get_display_filename(filename, TRUE);

	if (g_ascii_strcasecmp(base_uri, parser->folder_saved) == 0)
		uri = get_display_filename(filename, FALSE);
	else
		uri = g_strdup(filename);

	totem_pl_playlist_append (parser->playlist, &pl_iter);
	totem_pl_playlist_set (parser->playlist, &pl_iter,
	                       TOTEM_PL_PARSER_FIELD_URI, uri,
	                       NULL);

	g_free(uri);
	g_free(base_uri);
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
pragha_parser_save_full_track_list(PraghaPlaylist *cplaylist, gchar *filename)
{
	PraghaPlParser *parser;
	TotemPlPlaylist *playlist;
	TotemPlParser *pl;
	GFile *file;
	gboolean ret = TRUE;

	pl = totem_pl_parser_new ();
	playlist = totem_pl_playlist_new ();
	file = g_file_new_for_path (filename);

	parser = g_slice_new (PraghaPlParser);
	parser->playlist = playlist;
	parser->folder_saved = get_display_filename(filename, TRUE);

	gtk_tree_model_foreach(pragha_playlist_get_model(cplaylist),
	                       pragha_parser_append_foreach_track_list,
	                       parser);

	if (totem_pl_parser_save (pl, playlist, file, "Title", TOTEM_PL_PARSER_M3U, NULL) != TRUE) {
        g_error ("Playlist writing failed.");
        ret = FALSE;
    }

	g_free(parser->folder_saved);
	g_slice_free (PraghaPlParser, parser);

	g_object_unref (playlist);
	g_object_unref (pl);
	g_object_unref (file);

	return ret;
}

static gboolean
pragha_parser_save_selection_track_list(PraghaPlaylist *cplaylist, gchar *filename)
{
	PraghaPlParser *parser;
	TotemPlPlaylist *playlist;
	TotemPlParser *pl;
	GtkTreeSelection *selection;
	GFile *file;
	gboolean ret = TRUE;

	pl = totem_pl_parser_new ();
	playlist = totem_pl_playlist_new ();
	file = g_file_new_for_path (filename);

	parser = g_slice_new (PraghaPlParser);
	parser->playlist = playlist;
	parser->folder_saved = get_display_filename(filename, TRUE);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(pragha_playlist_get_view(cplaylist)));
	gtk_tree_selection_selected_foreach(selection,
	                                    pragha_parser_append_foreach_playlist,
	                                    parser);

	if (totem_pl_parser_save (pl, playlist, file, "Title", TOTEM_PL_PARSER_M3U, NULL) != TRUE) {
        g_error ("Playlist writing failed.");
        ret = FALSE;
    }

	g_free(parser->folder_saved);
	g_slice_free (PraghaPlParser, parser);

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

		g_list_foreach(list, (GFunc) g_object_unref, NULL);
		g_list_free(list);
	}

	return ret;
}

/**********************/
/* External functions */
/**********************/

/* Append the given playlist to the current playlist */

void add_playlist_current_playlist(gchar *playlist, struct con_win *cwin)
{
	GList *list = NULL;

	list = add_playlist_to_mobj_list(cwin->cdbase, playlist, list);

	if(list)
		pragha_playlist_append_mobj_list(cwin->cplaylist, list);
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
			mobj = new_musicobject_from_file (file);

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

	pragha_hig_workarea_table_finish(table, &row);

	dialog = gtk_dialog_new_with_buttons(_("Rename"),
			     GTK_WINDOW(parent),
			     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			     GTK_STOCK_CANCEL,
			     GTK_RESPONSE_CANCEL,
			     GTK_STOCK_OK,
			     GTK_RESPONSE_ACCEPT,
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

	dialog = gtk_file_chooser_dialog_new(_("Export playlist to file"),
	                                     GTK_WINDOW(parent),
	                                     GTK_FILE_CHOOSER_ACTION_SAVE,
	                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                                     GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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
void export_playlist (PraghaPlaylist* cplaylist, gint choice)
{
	gchar *filename = NULL;

	filename = playlist_export_dialog_get_filename(_("Playlists"),
	                                               gtk_widget_get_toplevel(GTK_WIDGET(pragha_playlist_get_widget(cplaylist))));
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
	}

	g_free(filename);
}
#else
void export_playlist (PraghaPlaylist* cplaylist, gint choice)
{
	gchar *filename = NULL;
	GIOChannel *chan = NULL;
	GError *err = NULL;

	filename = playlist_export_dialog_get_filename(_("Playlists"),
	                                               gtk_widget_get_toplevel(GTK_WIDGET(pragha_playlist_get_widget(cplaylist))));

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
static void _on_pl_entry_parsed(TotemPlParser *parser, gchar *uri,
				gpointer metadata, GSList **plitems)
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

GSList *pragha_totem_pl_parser_parse_from_uri(const gchar *uri)
{
	static TotemPlParser *pl_parser = NULL;
	GSList *plitems = NULL;
	gchar *base;

	pl_parser = totem_pl_parser_new ();
	g_object_set(pl_parser, "recurse", TRUE, NULL);

	g_signal_connect(G_OBJECT(pl_parser), "entry-parsed",
				G_CALLBACK(_on_pl_entry_parsed), &plitems);

	base = get_display_filename(uri, TRUE);

	if (totem_pl_parser_parse_with_base(pl_parser, uri, base, FALSE)
	    != TOTEM_PL_PARSER_RESULT_SUCCESS) {
		/* An error happens while parsing */
		goto bad;
	}
	g_object_unref (pl_parser);

bad:
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

		if (pragha_process_gtk_events ()) {
			g_free(filename);
			g_free(f_file);
			g_free(str);
			goto exit_chan;
		}

		g_free(f_file);
	continue_read:
		g_free(filename);
		g_free(str);
	}

	CDEBUG(DBG_INFO, "Loaded M3U playlist: %s", file);

	g_free(base);

exit_chan:
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
pragha_pl_parser_parse (enum playlist_type format, const gchar *filename)
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
	enum playlist_type format = PL_FORMAT_UNKNOWN;
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
		mobj = new_musicobject_from_file(i->data);
		if (G_LIKELY(mobj))
			mlist = g_list_append(mlist, mobj);

		if (pragha_process_gtk_events ())
			return NULL;

		g_free(i->data);
	}
	g_slist_free(list);

	return mlist;
}

void pragha_pl_parser_open_from_file_by_extension (const gchar *file, struct con_win *cwin)
{
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
		mobj = new_musicobject_from_file(i->data);
		if (G_LIKELY(mobj)) {
			added++;
			mlist = g_list_append(mlist, mobj);
		}

		if (pragha_process_gtk_events ())
			return;

		g_free(i->data);
	}
	pragha_playlist_append_mobj_list(cwin->cplaylist, mlist);

	summary = g_strdup_printf(_("Added %d songs from %d of the imported playlist."), added, try);

	statusbar = pragha_statusbar_get ();
	pragha_statusbar_set_misc_text (statusbar, summary);
	g_object_unref (statusbar);

	g_free(summary);

	g_slist_free(list);
	g_list_free(mlist);
}

/* Appennd a tracks list to a playlist using the given type */

static void
append_files_to_playlist(PraghaDatabase *cdbase, GSList *list, gint playlist_id)
{
	gchar *file;
	GSList *i = NULL;

	pragha_database_begin_transaction (cdbase);

	for (i=list; i != NULL; i = i->next) {
		file = i->data;
		pragha_database_add_playlist_track (cdbase, playlist_id, file);
		g_free(file);
	}

	pragha_database_commit_transaction (cdbase);
}

/* Save tracks to a playlist using the given type */

void
save_playlist(PraghaPlaylist* cplaylist,
              gint playlist_id,
              enum playlist_mgmt type)
{
	PraghaMusicobject *mobj = NULL;
	GList *mlist = NULL, *i;
	GSList *files = NULL;
	gchar *file = NULL;

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

	if(mlist != NULL) {
		for (i=mlist; i != NULL; i = i->next) {
			mobj = i->data;
			if (pragha_musicobject_is_local_file(mobj)) {
			    	file = g_strdup(pragha_musicobject_get_file(mobj));
				files = g_slist_prepend(files, file);
			}
			else if(pragha_musicobject_get_file_type(mobj) == FILE_HTTP) {
				/* TODO: Fix this negradaaa!. */
				file = g_strdup_printf("Radio:/%s", pragha_musicobject_get_file(mobj));
				files = g_slist_prepend(files, file);
			}
		}
		g_list_free(mlist);
	}

	if(files != NULL) {
		append_files_to_playlist(pragha_playlist_get_database(cplaylist), files, playlist_id);
		g_slist_free(files);
	}
}

void
new_playlist(PraghaPlaylist* cplaylist,
             const gchar *playlist,
             enum playlist_mgmt type)
{
	gint playlist_id = 0;

	if (string_is_empty(playlist)) {
		g_warning("Playlist name is NULL");
		return;
	}

	if ((playlist_id = pragha_database_find_playlist (pragha_playlist_get_database(cplaylist), playlist))) {
		if (overwrite_existing_playlist(playlist, gtk_widget_get_toplevel(GTK_WIDGET(pragha_playlist_get_widget(cplaylist)))))
			pragha_database_delete_playlist (pragha_playlist_get_database(cplaylist), playlist);
		else
			return;
	}

	playlist_id = pragha_database_add_new_playlist (pragha_playlist_get_database(cplaylist), playlist);
	save_playlist(cplaylist, playlist_id, type);
}

void append_playlist(PraghaPlaylist* cplaylist, const gchar *playlist, gint type)
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

void new_radio (PraghaPlaylist* cplaylist, const gchar *uri, const gchar *name)
{
	gint radio_id = 0;

	if (string_is_empty(name)) {
		g_warning("Radio name is NULL");
		return;
	}

	if ((radio_id = pragha_database_find_radio (pragha_playlist_get_database(cplaylist), name))) {
		if (overwrite_existing_playlist(name, gtk_widget_get_toplevel(GTK_WIDGET(pragha_playlist_get_widget(cplaylist)))))
			pragha_database_delete_radio (pragha_playlist_get_database(cplaylist), name);
		else
			return;
	}

	radio_id = pragha_database_add_new_radio (pragha_playlist_get_database(cplaylist), name);

	pragha_database_add_radio_track (pragha_playlist_get_database(cplaylist), radio_id, uri);
}

enum playlist_mgmt
replace_or_append_dialog(PraghaPlaylist *cplaylist, const gchar *playlist, gint type)
{
	GtkWidget *dialog;
	GtkWidget *table, *radio_replace, *radio_add;
	gchar *string_options = NULL;
	gint result;
	guint row = 0;
	enum playlist_mgmt choise = EXPORT_PLAYLIST;

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

	pragha_hig_workarea_table_finish(table, &row);

	dialog = gtk_dialog_new_with_buttons(NULL,
			     GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pragha_playlist_get_widget(cplaylist)))),
			     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			     GTK_STOCK_CANCEL,
			     GTK_RESPONSE_CANCEL,
			     GTK_STOCK_OK,
			     GTK_RESPONSE_ACCEPT,
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

void playlist_save_selection(GtkMenuItem *menuitem, PraghaPlaylist *cplaylist)
{
	enum playlist_mgmt choise;
	const gchar *playlist;

	playlist = gtk_menu_item_get_label (menuitem);

	choise = replace_or_append_dialog(cplaylist, playlist, SAVE_SELECTED);
	switch(choise) {
		case NEW_PLAYLIST:
			new_playlist(cplaylist, playlist, SAVE_SELECTED);
			break;
		case APPEND_PLAYLIST:
			append_playlist(cplaylist, playlist, SAVE_SELECTED);
			break;
		default:
			break;
	}
}

void playlist_save_complete_playlist(GtkMenuItem *menuitem, PraghaPlaylist *cplaylist)
{
	enum playlist_mgmt choise;
	const gchar *playlist;

	playlist = gtk_menu_item_get_label (menuitem);

	choise = replace_or_append_dialog(cplaylist, playlist, SAVE_COMPLETE);
	switch(choise) {
		case NEW_PLAYLIST:
			new_playlist(cplaylist, playlist, SAVE_COMPLETE);
			break;
		case APPEND_PLAYLIST:
			append_playlist(cplaylist, playlist, SAVE_COMPLETE);
			break;
		default:
			break;
	}
}

static void
update_playlist_changes_save_selection_popup_playlist (PraghaPlaylist *cplaylist)
{
	GtkWidget *submenu, *menuitem;
	
	submenu = gtk_menu_new ();

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (gtk_ui_manager_get_widget (pragha_playlist_get_context_menu(cplaylist), "/SelectionPopup/Save selection")), submenu);

	menuitem = gtk_image_menu_item_new_with_label (_("New playlist"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
	g_signal_connect(menuitem, "activate", G_CALLBACK(save_selected_playlist), cplaylist);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_image_menu_item_new_with_label (_("Export"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_stock (GTK_STOCK_FLOPPY, GTK_ICON_SIZE_MENU));
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
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
	g_signal_connect(menuitem, "activate", G_CALLBACK(save_current_playlist), cplaylist);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_image_menu_item_new_with_label (_("Export"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_stock (GTK_STOCK_FLOPPY, GTK_ICON_SIZE_MENU));
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

static void
update_playlist_changes_save_playlist_mainmenu (struct con_win *cwin)
{
	GtkWidget *submenu, *menuitem;
	GtkAccelGroup* accel_group;

	submenu = gtk_menu_new ();

	gtk_menu_item_set_submenu (GTK_MENU_ITEM(gtk_ui_manager_get_widget(cwin->bar_context_menu,"/Menubar/PlaylistMenu/Save playlist")), submenu);

	menuitem = gtk_image_menu_item_new_with_label (_("New playlist"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
	g_signal_connect(menuitem, "activate", G_CALLBACK(save_current_playlist), cwin->cplaylist);

	accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow), accel_group);
	gtk_menu_set_accel_group(GTK_MENU(submenu), accel_group);
	gtk_accel_map_add_entry ("<SubMenu>/New playlist", gdk_keyval_from_name ("s"), GDK_CONTROL_MASK);
	gtk_menu_item_set_accel_path (GTK_MENU_ITEM(menuitem), "<SubMenu>/New playlist");
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_image_menu_item_new_with_label (_("Export"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_stock (GTK_STOCK_FLOPPY, GTK_ICON_SIZE_MENU));
	g_signal_connect(menuitem, "activate", G_CALLBACK(export_current_playlist), cwin->cplaylist);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	const gchar *sql = "SELECT name FROM PLAYLIST WHERE name != ? ORDER BY name COLLATE NOCASE";
	PraghaPreparedStatement *statement = pragha_database_create_statement (cwin->cdbase, sql);
	pragha_prepared_statement_bind_string (statement, 1, SAVE_PLAYLIST_STATE);

	while (pragha_prepared_statement_step (statement)) {
		const gchar *name = pragha_prepared_statement_get_string (statement, 0);
		menuitem = gtk_image_menu_item_new_with_label (name);
		g_signal_connect (menuitem, "activate", G_CALLBACK(playlist_save_complete_playlist), cwin->cplaylist);
		gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menuitem);
	}

	pragha_prepared_statement_free (statement);

	gtk_widget_show_all (submenu);
}

static void
update_playlist_changes_save_selection_mainmenu (struct con_win *cwin)
{
	GtkWidget *submenu, *menuitem;
	GtkAccelGroup* accel_group;
	
	submenu = gtk_menu_new ();

	gtk_menu_item_set_submenu (GTK_MENU_ITEM(gtk_ui_manager_get_widget(cwin->bar_context_menu,"/Menubar/PlaylistMenu/Save selection")), submenu);

	menuitem = gtk_image_menu_item_new_with_label (_("New playlist"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
	g_signal_connect(menuitem, "activate", G_CALLBACK(save_selected_playlist), cwin->cplaylist);

	accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group(GTK_WINDOW(cwin->mainwindow), accel_group);
	gtk_menu_set_accel_group(GTK_MENU(submenu), accel_group);
	gtk_accel_map_add_entry ("<SubMenu>/Save selection", gdk_keyval_from_name ("s"), GDK_CONTROL_MASK+GDK_SHIFT_MASK);
	gtk_menu_item_set_accel_path (GTK_MENU_ITEM(menuitem), "<SubMenu>/Save selection");
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_image_menu_item_new_with_label (_("Export"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_stock (GTK_STOCK_FLOPPY, GTK_ICON_SIZE_MENU));
	g_signal_connect(menuitem, "activate", G_CALLBACK(export_selected_playlist), cwin->cplaylist);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	const gchar *sql = "SELECT name FROM PLAYLIST WHERE name != ? ORDER BY name COLLATE NOCASE";
	PraghaPreparedStatement *statement = pragha_database_create_statement (cwin->cdbase, sql);
	pragha_prepared_statement_bind_string (statement, 1, SAVE_PLAYLIST_STATE);

	while (pragha_prepared_statement_step (statement)) {
		const gchar *name = pragha_prepared_statement_get_string (statement, 0);
		menuitem = gtk_image_menu_item_new_with_label (name);
		g_signal_connect (menuitem, "activate", G_CALLBACK(playlist_save_selection), cwin->cplaylist);
		gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menuitem);
	}

	pragha_prepared_statement_free (statement);

	gtk_widget_show_all (submenu);
}

void update_playlist_changes_on_menu(struct con_win *cwin)
{
	/* Update main menu. */
	update_playlist_changes_save_playlist_mainmenu(cwin);
	update_playlist_changes_save_selection_mainmenu(cwin);

	/* Update playlist pupup menu. */
	update_playlist_changes_save_playlist_popup_playlist(cwin->cplaylist);
	update_playlist_changes_save_selection_popup_playlist(cwin->cplaylist);
}
