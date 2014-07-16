
#include <gobject/gvaluecollector.h>
#include "pragha-model-playlist.h"
#include "pragha-utils.h"

/* boring declarations of local functions */

static void
pragha_model_playlist_init            (PraghaModelPlaylist      *pkg_tree);

static void
pragha_model_playlist_class_init      (PraghaModelPlaylistClass *klass);

static void
pragha_model_playlist_tree_model_init (GtkTreeModelIface        *iface);

static void
pragha_model_playlist_finalize        (GObject                  *object);

static GtkTreeModelFlags
pragha_model_playlist_get_flags       (GtkTreeModel             *tree_model);

static gint
pragha_model_playlist_get_n_columns   (GtkTreeModel             *tree_model);

static GType
pragha_model_playlist_get_column_type (GtkTreeModel             *tree_model,
                                       gint                      index);

static gboolean
pragha_model_playlist_get_iter        (GtkTreeModel             *tree_model,
                                       GtkTreeIter              *iter,
                                       GtkTreePath              *path);

static GtkTreePath *
pragha_model_playlist_get_path        (GtkTreeModel             *tree_model,
                                       GtkTreeIter              *iter);

static void
pragha_model_playlist_get_value       (GtkTreeModel             *tree_model,
                                       GtkTreeIter              *iter,
                                       gint                      column,
                                       GValue                   *value);

static gboolean
pragha_model_playlist_iter_next       (GtkTreeModel             *tree_model,
                                       GtkTreeIter              *iter);

static gboolean
pragha_model_playlist_iter_children   (GtkTreeModel             *tree_model,
                                       GtkTreeIter              *iter,
                                       GtkTreeIter              *parent);

static gboolean
pragha_model_playlist_iter_has_child  (GtkTreeModel             *tree_model,
                                       GtkTreeIter              *iter);

static gint
pragha_model_playlist_iter_n_children (GtkTreeModel             *tree_model,
                                       GtkTreeIter              *iter);

static gboolean
pragha_model_playlist_iter_nth_child  (GtkTreeModel             *tree_model,
                                       GtkTreeIter              *iter,
                                       GtkTreeIter              *parent,
                                       gint                      n);

static gboolean
pragha_model_playlist_iter_parent     (GtkTreeModel             *tree_model,
                                       GtkTreeIter              *iter,
                                       GtkTreeIter             *child);

static GObjectClass *parent_class = NULL;  /* GObject stuff - nothing to worry about */


/*****************************************************************************
 *
 *  pragha_model_playlist_get_type:
 *  here we register our new type and its interfaces
 *  with the type system. If you want to implement
 *  additional interfaces like GtkTreeSortable, you
 *  will need to do it here.
 *
 *****************************************************************************/

GType
pragha_model_playlist_get_type (void)
{
	static GType pragha_model_playlist_type = 0;

	if (pragha_model_playlist_type)
		return pragha_model_playlist_type;

	/* Some boilerplate type registration stuff */
	if (1) {
		static const GTypeInfo pragha_model_playlist_info =
			{
				sizeof (PraghaModelPlaylistClass),
				NULL,                                              /* base_init */
				NULL,                                              /* base_finalize */
				(GClassInitFunc) pragha_model_playlist_class_init,
				NULL,                                              /* class finalize */
				NULL,                                              /* class_data */
				sizeof (PraghaModelPlaylist),
				0,                                                 /* n_preallocs */
				(GInstanceInitFunc) pragha_model_playlist_init
			};

		pragha_model_playlist_type = g_type_register_static (G_TYPE_OBJECT, "PraghaModelPlaylist",
		                                                     &pragha_model_playlist_info, (GTypeFlags)0);
	}

	/* Here we register our GtkTreeModel interface with the type system */
	if (1) {
		static const GInterfaceInfo tree_model_info =
			{
				(GInterfaceInitFunc) pragha_model_playlist_tree_model_init,
				NULL,
				NULL
			};

		g_type_add_interface_static (pragha_model_playlist_type, GTK_TYPE_TREE_MODEL, &tree_model_info);
	}

	return pragha_model_playlist_type;
}


/*****************************************************************************
 *
 *  pragha_model_playlist_class_init: more boilerplate GObject/GType stuff.
 *                          Init callback for the type system,
 *                          called once when our new class is created.
 *
 *****************************************************************************/

static void
pragha_model_playlist_class_init (PraghaModelPlaylistClass *klass)
{
	GObjectClass *object_class;

	parent_class = (GObjectClass*) g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;

	object_class->finalize = pragha_model_playlist_finalize;
}


/*****************************************************************************
 *
 *  pragha_model_playlist_tree_model_init: init callback for the interface registration
 *                               in pragha_model_playlist_get_type. Here we override
 *                               the GtkTreeModel interface functions that
 *                               we implement.
 *
 *****************************************************************************/

static void
pragha_model_playlist_tree_model_init (GtkTreeModelIface *iface)
{
	iface->get_flags       = pragha_model_playlist_get_flags;
	iface->get_n_columns   = pragha_model_playlist_get_n_columns;
	iface->get_column_type = pragha_model_playlist_get_column_type;
	iface->get_iter        = pragha_model_playlist_get_iter;
	iface->get_path        = pragha_model_playlist_get_path;
	iface->get_value       = pragha_model_playlist_get_value;
	iface->iter_next       = pragha_model_playlist_iter_next;
	iface->iter_children   = pragha_model_playlist_iter_children;
	iface->iter_has_child  = pragha_model_playlist_iter_has_child;
	iface->iter_n_children = pragha_model_playlist_iter_n_children;
	iface->iter_nth_child  = pragha_model_playlist_iter_nth_child;
	iface->iter_parent     = pragha_model_playlist_iter_parent;
}


/*****************************************************************************
 *
 *  pragha_model_playlist_init: this is called everytime a new custom list object
 *                    instance is created (we do that in pragha_model_playlist_new).
 *                    Initialise the list structure's fields here.
 *
 *****************************************************************************/

static void
pragha_model_playlist_init (PraghaModelPlaylist *pragha_list)
{
	pragha_list->n_columns        = PRAGHA_LIST_N_COLUMNS;

	pragha_list->column_types[0]  = G_TYPE_POINTER;  /* Pointer to musicobject */
	pragha_list->column_types[1]  = G_TYPE_STRING;   /* Queue No String        */
	pragha_list->column_types[2]  = G_TYPE_BOOLEAN;  /* Show Bublle Queue      */
	pragha_list->column_types[3]  = GDK_TYPE_PIXBUF; /* Playback status pixbuf */
	pragha_list->column_types[4]  = G_TYPE_STRING;   /* Tag : Track No         */
	pragha_list->column_types[5]  = G_TYPE_STRING;   /* Tag : Title            */
	pragha_list->column_types[6]  = G_TYPE_STRING;   /* Tag : Artist           */
	pragha_list->column_types[7]  = G_TYPE_STRING;   /* Tag : Album            */
	pragha_list->column_types[8]  = G_TYPE_STRING;   /* Tag : Genre            */
	pragha_list->column_types[9]  = G_TYPE_STRING;   /* Tag : Bitrate          */
	pragha_list->column_types[10] = G_TYPE_STRING;   /* Tag : Year             */
	pragha_list->column_types[11] = G_TYPE_STRING;   /* Tag : Comment          */
	pragha_list->column_types[12] = G_TYPE_STRING;   /* Tag : Length           */
	pragha_list->column_types[13] = G_TYPE_STRING;   /* Filename               */
	pragha_list->column_types[14] = G_TYPE_STRING;   /* Mimetype               */
	pragha_list->column_types[15] = G_TYPE_BOOLEAN;  /* Played flag            */

	//g_assert ( PRAGHA_LIST_N_COLUMNS == 13);

	pragha_list->num_rows = 0;
	pragha_list->rows     = NULL;

	pragha_list->stamp = g_random_int();  /* Random int to check whether an iter belongs to our model */
}


/*****************************************************************************
 *
 *  pragha_model_playlist_finalize: this is called just before a custom list is
 *                        destroyed. Free dynamically allocated memory here.
 *
 *****************************************************************************/

static void
pragha_model_playlist_finalize (GObject *object)
{
	/*  PraghaModelPlaylist *pragha_list = PRAGHA_MODEL_PLAYLIST(object); */

	/* free all songs and free all memory used by the list */
	#warning IMPLEMENT

	/* must chain up - finalize parent */
	(* parent_class->finalize) (object);
}


/*****************************************************************************
 *
 *  pragha_model_playlist_get_flags: tells the rest of the world whether our tree model
 *                         has any special characteristics. In our case,
 *                         we have a list model (instead of a tree), and each
 *                         tree iter is valid as long as the row in question
 *                         exists, as it only contains a pointer to our struct.
 *
 *****************************************************************************/

static GtkTreeModelFlags
pragha_model_playlist_get_flags (GtkTreeModel *tree_model)
{
	g_return_val_if_fail (PRAGHA_IS_MODEL_PLAYLIST(tree_model), (GtkTreeModelFlags)0);

	return (GTK_TREE_MODEL_LIST_ONLY | GTK_TREE_MODEL_ITERS_PERSIST);
}


/*****************************************************************************
 *
 *  pragha_model_playlist_get_n_columns: tells the rest of the world how many data
 *                             columns we export via the tree model interface
 *
 *****************************************************************************/

static gint
pragha_model_playlist_get_n_columns (GtkTreeModel *tree_model)
{
	 g_return_val_if_fail (PRAGHA_IS_MODEL_PLAYLIST(tree_model), 0);

	return PRAGHA_MODEL_PLAYLIST(tree_model)->n_columns;
}


/*****************************************************************************
 *
 *  pragha_model_playlist_get_column_type: tells the rest of the world which type of
 *                               data an exported model column contains
 *
 *****************************************************************************/

static GType
pragha_model_playlist_get_column_type (GtkTreeModel *tree_model,
                                       gint          index)
{
	g_return_val_if_fail (PRAGHA_IS_MODEL_PLAYLIST(tree_model), G_TYPE_INVALID);
	g_return_val_if_fail (index < PRAGHA_MODEL_PLAYLIST(tree_model)->n_columns && index >= 0, G_TYPE_INVALID);

	return PRAGHA_MODEL_PLAYLIST(tree_model)->column_types[index];
}


/*****************************************************************************
 *
 *  pragha_model_playlist_get_iter: converts a tree path (physical position) into a
 *                        tree iter structure (the content of the iter
 *                        fields will only be used internally by our model).
 *                        We simply store a pointer to our PraghaSong
 *                        structure that represents that row in the tree iter.
 *
 *****************************************************************************/

static gboolean
pragha_model_playlist_get_iter (GtkTreeModel *tree_model,
                                GtkTreeIter  *iter,
                                GtkTreePath  *path)
{
	PraghaModelPlaylist    *pragha_list;
	PraghaSong    *song;
	gint          *indices, n, depth;

	g_assert(PRAGHA_IS_MODEL_PLAYLIST(tree_model));
	g_assert(path!=NULL);

	pragha_list = PRAGHA_MODEL_PLAYLIST(tree_model);

	indices = gtk_tree_path_get_indices(path);
	depth   = gtk_tree_path_get_depth(path);

	/* we do not allow children */
	g_assert(depth == 1); /* depth 1 = top level; a list only has top level nodes and no children */

	n = indices[0]; /* the n-th top level row */

	if ( n >= pragha_list->num_rows || n < 0 )
		return FALSE;

	song = pragha_list->rows[n];

	g_assert(song != NULL);
	g_assert(song->pos == n);

	/* We simply store a pointer to our custom song in the iter */
	iter->stamp      = pragha_list->stamp;
	iter->user_data  = song;
	iter->user_data2 = NULL;   /* unused */
	iter->user_data3 = NULL;   /* unused */

	return TRUE;
}


/*****************************************************************************
 *
 *  pragha_model_playlist_get_path: converts a tree iter into a tree path (ie. the
 *                        physical position of that row in the list).
 *
 *****************************************************************************/

static GtkTreePath *
pragha_model_playlist_get_path (GtkTreeModel *tree_model,
                                GtkTreeIter  *iter)
{
	GtkTreePath  *path;
	PraghaSong *song;

	g_return_val_if_fail (PRAGHA_IS_MODEL_PLAYLIST(tree_model), NULL);
	g_return_val_if_fail (iter != NULL,                         NULL);
	g_return_val_if_fail (iter->user_data != NULL,              NULL);

	song = (PraghaSong*) iter->user_data;

	path = gtk_tree_path_new();
	gtk_tree_path_append_index(path, song->pos);

	return path;
}


/*****************************************************************************
 *
 *  pragha_model_playlist_get_value: Returns a row's exported data columns
 *                         (_get_value is what gtk_tree_model_get uses)
 *
 *****************************************************************************/

static void
pragha_model_playlist_get_value (GtkTreeModel *tree_model,
                                 GtkTreeIter  *iter,
                                 gint          column,
                                 GValue       *value)
{
	PraghaSong          *song;
	PraghaModelPlaylist *pragha_list;
	gchar *ch_length = NULL, *ch_track_no = NULL, *ch_year = NULL, *ch_bitrate = NULL, *ch_filename = NULL;
	const gchar *title = NULL;
	gint track_no, year, length, bitrate;
	
	g_return_if_fail (PRAGHA_IS_MODEL_PLAYLIST (tree_model));
	g_return_if_fail (iter != NULL);
	g_return_if_fail (column < PRAGHA_MODEL_PLAYLIST(tree_model)->n_columns);

	g_value_init (value, PRAGHA_MODEL_PLAYLIST(tree_model)->column_types[column]);

	pragha_list = PRAGHA_MODEL_PLAYLIST(tree_model);

	song = (PraghaSong*) iter->user_data;

	g_return_if_fail (song != NULL);

	if (song->pos >= pragha_list->num_rows)
		g_return_if_reached();

	switch (column) {
		case PRAGHA_LIST_COL_MOBJ:
			g_value_set_pointer(value, song->mobj);
			break;
		case PRAGHA_LIST_COL_QUEUE:
			g_value_set_string(value, song->queue);
			break;
		case PRAGHA_LIST_COL_BUBBLE:
			g_value_set_boolean(value, song->bubble);
			break;
		case PRAGHA_LIST_COL_PIXBUF:
			g_value_set_pointer(value, song->pixbuf);
			break;
		case PRAGHA_LIST_COL_TRACK_NO:
			track_no = pragha_musicobject_get_track_no(song->mobj);
			if(track_no > 0) {
				ch_track_no = g_strdup_printf("%d", track_no);
				g_value_set_string(value, ch_track_no);
				g_free(ch_track_no);
			}
			break;
		case PRAGHA_LIST_COL_TITLE:
			ch_filename = get_display_name (song->mobj);
			title = pragha_musicobject_get_title(song->mobj);
			g_value_set_string (value, string_is_not_empty(title) ? title : ch_filename);
			g_free(ch_filename);
			break;
		case PRAGHA_LIST_COL_ARTIST:
			g_value_set_string(value, pragha_musicobject_get_artist(song->mobj));
			break;
		case PRAGHA_LIST_COL_ALBUM:
			g_value_set_string(value, pragha_musicobject_get_album(song->mobj));
			break;
		case PRAGHA_LIST_COL_GENRE:
			g_value_set_string(value, pragha_musicobject_get_genre(song->mobj));
			break;
		case PRAGHA_LIST_COL_BITRATE:
			bitrate = pragha_musicobject_get_bitrate(song->mobj);
			if(bitrate > 0) {
				ch_bitrate = g_strdup_printf("%d", bitrate);
				g_value_set_string(value, ch_bitrate);
				g_free(ch_bitrate);
			}
			break;
		case PRAGHA_LIST_COL_YEAR:
			year = pragha_musicobject_get_year(song->mobj);
			if(year > 0) {
				ch_year = g_strdup_printf("%d", year);
				g_value_set_string(value, ch_year);
				g_free(ch_year);
			}
			break;
		case PRAGHA_LIST_COL_COMMENT:
			g_value_set_string(value, pragha_musicobject_get_comment(song->mobj));
			break;
		case PRAGHA_LIST_COL_LENGTH:
			length = pragha_musicobject_get_length(song->mobj);
			if (length > 0) {
				ch_length = convert_length_str(length);
				g_value_set_string(value, ch_length);
				g_free(ch_length);
			}
			break;
		case PRAGHA_LIST_COL_FILENAME:
			g_value_set_string(value, pragha_musicobject_get_file(song->mobj));
			break;
		case PRAGHA_LIST_COL_MIME_TYPE:
			g_value_set_string(value, pragha_musicobject_get_mime_type(song->mobj));
			break;
		case PRAGHA_LIST_COL_PLAYED:
			g_value_set_boolean(value, FALSE);
			break;
	}
}


/*****************************************************************************
 *
 *  pragha_model_playlist_set_value: Returns a row's exported data columns
 *                         (_set_value is what gtk_tree_model_get uses)
 *
 *****************************************************************************/

static void
pragha_model_playlist_set_value (GtkTreeModel *tree_model,
                                 GtkTreeIter  *iter,
                                 gint          column,
                                 GValue       *value)
{
	PraghaSong          *song;
	PraghaModelPlaylist *pragha_list;

	g_return_if_fail (PRAGHA_IS_MODEL_PLAYLIST (tree_model));
	g_return_if_fail (iter != NULL);
	g_return_if_fail (column < PRAGHA_MODEL_PLAYLIST(tree_model)->n_columns);

	pragha_list = PRAGHA_MODEL_PLAYLIST(tree_model);

	song = (PraghaSong*) iter->user_data;

	g_return_if_fail (song != NULL);

	if (song->pos >= pragha_list->num_rows)
		g_return_if_reached();

	switch (column) {
		case PRAGHA_LIST_COL_MOBJ:
			break;
		case PRAGHA_LIST_COL_QUEUE:
			if(song->queue)
				g_free (song->queue);
			song->queue = g_strdup(g_value_get_string(value));
			break;
		case PRAGHA_LIST_COL_BUBBLE:
			song->bubble = g_value_get_boolean(value);
			break;
		case PRAGHA_LIST_COL_PIXBUF:
			song->pixbuf = g_value_get_pointer(value);
			break;
		case PRAGHA_LIST_COL_TRACK_NO:
			pragha_musicobject_set_track_no (song->mobj, g_value_get_int(value));
			break;
		case PRAGHA_LIST_COL_TITLE:
			pragha_musicobject_set_title (song->mobj, g_value_get_string(value));
			break;
		case PRAGHA_LIST_COL_ARTIST:
			pragha_musicobject_set_artist (song->mobj, g_value_get_string(value));
			break;
		case PRAGHA_LIST_COL_ALBUM:
			pragha_musicobject_set_album (song->mobj, g_value_get_string(value));
			break;
		case PRAGHA_LIST_COL_GENRE:
			pragha_musicobject_set_genre (song->mobj, g_value_get_string(value));
			break;
		case PRAGHA_LIST_COL_BITRATE:
			break;
		case PRAGHA_LIST_COL_YEAR:
			//pragha_musicobject_set_year(song->mobj, atoi(g_value_get_string(value)));
			break;
		case PRAGHA_LIST_COL_COMMENT:
			pragha_musicobject_set_comment (song->mobj, g_value_get_string(value));
			break;
		case PRAGHA_LIST_COL_LENGTH:
			break;
		case PRAGHA_LIST_COL_FILENAME:
			break;
		case PRAGHA_LIST_COL_MIME_TYPE:
			break;
		case PRAGHA_LIST_COL_PLAYED:
			song->played = g_value_get_boolean(value);
			break;
	}
}

static void
pragha_model_playlist_set_real (GtkTreeModel *tree_model,
                                GtkTreeIter  *iter,
                                va_list       var_args)
{
	PraghaModelPlaylist *pragha_list;
	gint column;

	pragha_list = PRAGHA_MODEL_PLAYLIST(tree_model);

	column = va_arg (var_args, gint);

	while (column != -1) {
		GValue value = G_VALUE_INIT;
		gchar *error = NULL;

		G_VALUE_COLLECT_INIT (&value, pragha_list->column_types[column],
		                      var_args, 0, &error);

		if (error) {
			g_warning ("%s: %s", G_STRLOC, error);
			g_free (error);
			break;
		}

		pragha_model_playlist_set_value (tree_model, iter, column, &value);
		g_value_unset (&value);

		/*GtkTreePath *path;
		path = pragha_model_playlist_get_path (tree_model, iter);
		gtk_tree_model_row_changed (tree_model, path, iter);
		gtk_tree_path_free (path);*/

		column = va_arg (var_args, gint);
	}
}

void
pragha_model_playlist_set (GtkTreeModel *tree_model,
                           GtkTreeIter  *iter,
                           ...)
{
	va_list var_args;

	va_start (var_args, iter);
	pragha_model_playlist_set_real (tree_model, iter, var_args);
	va_end (var_args);
}

/*****************************************************************************
 *
 *  pragha_model_playlist_iter_next: Takes an iter structure and sets it to point
 *                         to the next row.
 *
 *****************************************************************************/

static gboolean
pragha_model_playlist_iter_next (GtkTreeModel  *tree_model,
                                 GtkTreeIter   *iter)
{
	PraghaSong  *song, *nextsong;
	PraghaModelPlaylist    *pragha_list;

	g_return_val_if_fail (PRAGHA_IS_MODEL_PLAYLIST (tree_model), FALSE);

	if (iter == NULL || iter->user_data == NULL)
	return FALSE;

	pragha_list = PRAGHA_MODEL_PLAYLIST(tree_model);

	song = (PraghaSong *) iter->user_data;

	/* Is this the last song in the list? */
	if ((song->pos + 1) >= pragha_list->num_rows)
		return FALSE;

	nextsong = pragha_list->rows[(song->pos + 1)];

	g_assert (nextsong != NULL);
	g_assert (nextsong->pos == (song->pos + 1));

	iter->stamp     = pragha_list->stamp;
	iter->user_data = nextsong;

	return TRUE;
}


/*****************************************************************************
 *
 *  pragha_model_playlist_iter_children: Returns TRUE or FALSE depending on whether
 *                             the row specified by 'parent' has any children.
 *                             If it has children, then 'iter' is set to
 *                             point to the first child. Special case: if
 *                             'parent' is NULL, then the first top-level
 *                             row should be returned if it exists.
 *
 *****************************************************************************/

static gboolean
pragha_model_playlist_iter_children (GtkTreeModel *tree_model,
                                     GtkTreeIter  *iter,
                                     GtkTreeIter  *parent)
{
	PraghaModelPlaylist  *pragha_list;

	g_return_val_if_fail (parent == NULL || parent->user_data != NULL, FALSE);

	/* this is a list, nodes have no children */
	if (parent)
		return FALSE;

	/* parent == NULL is a special case; we need to return the first top-level row */

	g_return_val_if_fail (PRAGHA_IS_MODEL_PLAYLIST (tree_model), FALSE);

	pragha_list = PRAGHA_MODEL_PLAYLIST(tree_model);

	/* No rows => no first row */
	if (pragha_list->num_rows == 0)
		return FALSE;

	/* Set iter to first item in list */
	iter->stamp     = pragha_list->stamp;
	iter->user_data = pragha_list->rows[0];

	return TRUE;
}


/*****************************************************************************
 *
 *  pragha_model_playlist_iter_has_child: Returns TRUE or FALSE depending on whether
 *                              the row specified by 'iter' has any children.
 *                              We only have a list and thus no children.
 *
 *****************************************************************************/

static gboolean
pragha_model_playlist_iter_has_child (GtkTreeModel *tree_model,
                                      GtkTreeIter  *iter)
{
	return FALSE;
}


/*****************************************************************************
 *
 *  pragha_model_playlist_iter_n_children: Returns the number of children the row
 *                               specified by 'iter' has. This is usually 0,
 *                               as we only have a list and thus do not have
 *                               any children to any rows. A special case is
 *                               when 'iter' is NULL, in which case we need
 *                               to return the number of top-level nodes,
 *                               ie. the number of rows in our list.
 *
 *****************************************************************************/

static gint
pragha_model_playlist_iter_n_children (GtkTreeModel *tree_model,
                                       GtkTreeIter  *iter)
{
	PraghaModelPlaylist  *pragha_list;

	g_return_val_if_fail (PRAGHA_IS_MODEL_PLAYLIST (tree_model), -1);
	g_return_val_if_fail (iter == NULL || iter->user_data != NULL, FALSE);

	pragha_list = PRAGHA_MODEL_PLAYLIST(tree_model);

	/* special case: if iter == NULL, return number of top-level rows */
	if (!iter)
		return pragha_list->num_rows;

	return 0; /* otherwise, this is easy again for a list */
}


/*****************************************************************************
 *
 *  pragha_model_playlist_iter_nth_child: If the row specified by 'parent' has any
 *                              children, set 'iter' to the n-th child and
 *                              return TRUE if it exists, otherwise FALSE.
 *                              A special case is when 'parent' is NULL, in
 *                              which case we need to set 'iter' to the n-th
 *                              row if it exists.
 *
 *****************************************************************************/

static gboolean
pragha_model_playlist_iter_nth_child (GtkTreeModel *tree_model,
                                      GtkTreeIter  *iter,
                                      GtkTreeIter  *parent,
                                      gint          n)
{
	PraghaSong          *song;
	PraghaModelPlaylist *pragha_list;

	g_return_val_if_fail (PRAGHA_IS_MODEL_PLAYLIST (tree_model), FALSE);

	pragha_list = PRAGHA_MODEL_PLAYLIST(tree_model);

	/* a list has only top-level rows */
	if (parent)
		return FALSE;

	/* special case: if parent == NULL, set iter to n-th top-level row */

	if (n >= pragha_list->num_rows)
		return FALSE;

	song = pragha_list->rows[n];

	g_assert (song != NULL);
	g_assert (song->pos == n);

	iter->stamp = pragha_list->stamp;
	iter->user_data = song;

	return TRUE;
}


/*****************************************************************************
 *
 *  pragha_model_playlist_iter_parent: Point 'iter' to the parent node of 'child'. As
 *                           we have a list and thus no children and no
 *                           parents of children, we can just return FALSE.
 *
 *****************************************************************************/

static gboolean
pragha_model_playlist_iter_parent (GtkTreeModel *tree_model,
                                   GtkTreeIter  *iter,
                                   GtkTreeIter  *child)
{
	return FALSE;
}


/*****************************************************************************
 *
 *  pragha_model_playlist_new:  This is what you use in your own code to create a
 *                    new custom list tree model for you to use.
 *
 *****************************************************************************/

PraghaModelPlaylist *
pragha_model_playlist_new (void)
{
	PraghaModelPlaylist *newpraghalist;

	newpraghalist = (PraghaModelPlaylist*) g_object_new (PRAGHA_TYPE_MODEL_PLAYLIST, NULL);

	g_assert (newpraghalist != NULL);

	return newpraghalist;
}


/*****************************************************************************
 *
 *  pragha_model_playlist_append_song:  Empty lists are boring. This function can
 *                            be used in your own code to add rows to the
 *                            list. Note how we emit the "row-inserted"
 *                            signal after we have appended the row
 *                            internally, so the tree view and other
 *                            interested objects know about the new row.
 *
 *****************************************************************************/

void
pragha_model_playlist_append_song (PraghaModelPlaylist *pragha_list,
                                   PraghaMusicobject   *mobj)
{
	GtkTreeIter   iter;
	GtkTreePath  *path;
	PraghaSong   *newsong;
	gulong        newsize;
	guint         pos;

	g_return_if_fail (PRAGHA_IS_MODEL_PLAYLIST(pragha_list));

	pos = pragha_list->num_rows;

	pragha_list->num_rows++;

	newsize = pragha_list->num_rows * sizeof(PraghaSong*);

	pragha_list->rows = g_realloc(pragha_list->rows, newsize);

	newsong = g_new0(PraghaSong, 1);

	newsong->mobj = g_object_ref(mobj);

	newsong->played = FALSE;
	newsong->queue  = NULL;
	newsong->bubble = FALSE;
	newsong->pixbuf = NULL;

	pragha_list->rows[pos] = newsong;
	newsong->pos = pos;

	/* inform the tree view and other interested objects
	 *  (e.g. tree row references) that we have inserted
	 *  a new row, and where it was inserted */

	path = gtk_tree_path_new ();
	gtk_tree_path_append_index(path, newsong->pos);

	pragha_model_playlist_get_iter(GTK_TREE_MODEL(pragha_list), &iter, path);

	gtk_tree_model_row_inserted(GTK_TREE_MODEL(pragha_list), path, &iter);

	gtk_tree_path_free(path);
}
