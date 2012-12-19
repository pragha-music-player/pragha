#ifndef _pragha_list_h_included_
#define _pragha_list_h_included_

#include <gtk/gtk.h>
#include "pragha-musicobject.h"

/* Some boilerplate GObject defines. 'klass' is used
 *   instead of 'class', because 'class' is a C++ keyword */

#define PRAGHA_TYPE_LIST            (pragha_list_get_type ())
#define PRAGHA_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRAGHA_TYPE_LIST, PraghaList))
#define PRAGHA_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PRAGHA_TYPE_LIST, PraghaListClass))
#define PRAGHA_IS_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRAGHA_TYPE_LIST))
#define PRAGHA_IS_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PRAGHA_TYPE_LIST))
#define PRAGHA_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PRAGHA_TYPE_LIST, PraghaListClass))

/* The data columns that we export via the tree model interface */

enum
{
  PRAGHA_LIST_COL_MOBJ = 0,
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
  PRAGHA_LIST_N_COLUMNS,
} ;

typedef struct _PraghaSong       PraghaSong;
typedef struct _PraghaList       PraghaList;
typedef struct _PraghaListClass  PraghaListClass;

/* PraghaSong: this structure represents a row */

struct _PraghaSong
{
  /* data - you can extend this */
  PraghaMusicobject *mobj;

  gboolean played;

  /* admin stuff used by the custom list model */
  guint     pos;   /* pos within the array */
};



/* PraghaList: this structure contains everything we need for our
 *             model implementation. You can add extra fields to
 *             this structure, e.g. hashtables to quickly lookup
 *             rows or whatever else you might need, but it is
 *             crucial that 'parent' is the first member of the
 *             structure.                                          */

struct _PraghaList
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



/* PraghaListClass: more boilerplate GObject stuff */

struct _PraghaListClass
{
  GObjectClass parent_class;
};


GType             pragha_list_get_type (void);

PraghaList       *pragha_list_new (void);

void              pragha_list_append_song (PraghaList        *pragha_list,
                                           PraghaMusicobject *mobj);

#endif /* _pragha_list_h_included_ */