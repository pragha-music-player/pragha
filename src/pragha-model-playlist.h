#ifndef _pragha_list_h_included_
#define _pragha_list_h_included_

#include <gtk/gtk.h>
#include "pragha-musicobject.h"

/* Some boilerplate GObject defines. 'klass' is used
 *   instead of 'class', because 'class' is a C++ keyword */

#define PRAGHA_TYPE_MODEL_PLAYLIST            (pragha_model_playlist_get_type ())
#define PRAGHA_MODEL_PLAYLIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_MODEL_PLAYLIST, PraghaModelPlaylist))
#define PRAGHA_MODEL_PLAYLIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PRAGHA_TYPE_MODEL_PLAYLIST, PraghaModelPlaylistClass))
#define PRAGHA_IS_MODEL_PLAYLIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_MODEL_PLAYLIST))
#define PRAGHA_IS_MODEL_PLAYLIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PRAGHA_TYPE_MODEL_PLAYLIST))
#define PRAGHA_MODEL_PLAYLIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PRAGHA_TYPE_MODEL_PLAYLIST, PraghaModelPlaylistClass))

/* The data columns that we export via the tree model interface */

enum
{
	PRAGHA_LIST_COL_MOBJ = 0,
	PRAGHA_LIST_COL_QUEUE,
	PRAGHA_LIST_COL_BUBBLE,
	PRAGHA_LIST_COL_PIXBUF,
	PRAGHA_LIST_COL_TRACK_NO,
	PRAGHA_LIST_COL_TITLE,
	PRAGHA_LIST_COL_ARTIST,
	PRAGHA_LIST_COL_ALBUM,
	PRAGHA_LIST_COL_GENRE,
	PRAGHA_LIST_COL_BITRATE,
	PRAGHA_LIST_COL_YEAR,
	PRAGHA_LIST_COL_COMMENT,
	PRAGHA_LIST_COL_LENGTH,
	PRAGHA_LIST_COL_FILENAME,
	PRAGHA_LIST_COL_MIME_TYPE,
	PRAGHA_LIST_COL_PLAYED,
	PRAGHA_LIST_N_COLUMNS
} ;

typedef struct _PraghaSong                PraghaSong;
typedef struct _PraghaModelPlaylist       PraghaModelPlaylist;
typedef struct _PraghaModelPlaylistClass  PraghaModelPlaylistClass;

/* PraghaSong: this structure represents a row */

struct _PraghaSong
{
	/* data - you can extend this */
	PraghaMusicobject *mobj;

	gboolean           played;
	gchar             *queue;
	gboolean           bubble;
	gpointer           pixbuf;

	/* admin stuff used by the custom list model */
	guint              pos;   /* pos within the array */
};



/* PraghaModelPlaylist:
 * this structure contains everything we need for our
 * model implementation. You can add extra fields to
 * this structure, e.g. hashtables to quickly lookup
 * rows or whatever else you might need, but it is
 * crucial that 'parent' is the first member of the
 * structure.
 */

struct _PraghaModelPlaylist
{
	GObject         parent;      /* this MUST be the first member */

	guint           num_rows;    /* number of rows that we have   */
	PraghaSong    **rows;        /* a dynamically allocated array of pointers to
	                              *   the PraghaSong structure for each row    */

	/* These two fields are not absolutely necessary, but they    */
	/*   speed things up a bit in our get_value implementation    */
	gint            n_columns;
	GType           column_types[PRAGHA_LIST_N_COLUMNS];

	gint            stamp;       /* Random integer to check whether an iter belongs to our model */
};



/* PraghaModelPlaylistClass: more boilerplate GObject stuff */

struct _PraghaModelPlaylistClass
{
	GObjectClass parent_class;
};


GType                pragha_model_playlist_get_type (void);

PraghaModelPlaylist *pragha_model_playlist_new (void);

void
pragha_model_playlist_set (GtkTreeModel *tree_model,
                           GtkTreeIter  *iter,
                           ...);

void
pragha_model_playlist_append_song (PraghaModelPlaylist *pragha_list,
                                   PraghaMusicobject   *mobj);

#endif /* _pragha_list_h_included_ */