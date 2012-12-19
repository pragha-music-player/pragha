#include "pragha-list.h"


/* boring declarations of local functions */

static void         pragha_list_init            (PraghaList      *pkg_tree);

static void         pragha_list_class_init      (PraghaListClass *klass);

static void         pragha_list_tree_model_init (GtkTreeModelIface *iface);

static void         pragha_list_finalize        (GObject           *object);

static GtkTreeModelFlags pragha_list_get_flags  (GtkTreeModel      *tree_model);

static gint         pragha_list_get_n_columns   (GtkTreeModel      *tree_model);

static GType        pragha_list_get_column_type (GtkTreeModel      *tree_model,
                                                 gint               index);

static gboolean     pragha_list_get_iter        (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter,
                                                 GtkTreePath       *path);

static GtkTreePath *pragha_list_get_path        (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter);

static void         pragha_list_get_value       (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter,
                                                 gint               column,
                                                 GValue            *value);

static gboolean     pragha_list_iter_next       (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter);

static gboolean     pragha_list_iter_children   (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter,
                                                 GtkTreeIter       *parent);

static gboolean     pragha_list_iter_has_child  (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter);

static gint         pragha_list_iter_n_children (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter);

static gboolean     pragha_list_iter_nth_child  (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter,
                                                 GtkTreeIter       *parent,
                                                 gint               n);

static gboolean     pragha_list_iter_parent     (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter,
                                                 GtkTreeIter       *child);



static GObjectClass *parent_class = NULL;  /* GObject stuff - nothing to worry about */


/*****************************************************************************
 *
 *  pragha_list_get_type: here we register our new type and its interfaces
 *                        with the type system. If you want to implement
 *                        additional interfaces like GtkTreeSortable, you
 *                        will need to do it here.
 *
 *****************************************************************************/

GType
pragha_list_get_type (void)
{
  static GType pragha_list_type = 0;

  if (pragha_list_type)
    return pragha_list_type;

  /* Some boilerplate type registration stuff */
  if (1)
  {
    static const GTypeInfo pragha_list_info =
    {
      sizeof (PraghaListClass),
      NULL,                                         /* base_init */
      NULL,                                         /* base_finalize */
      (GClassInitFunc) pragha_list_class_init,
      NULL,                                         /* class finalize */
      NULL,                                         /* class_data */
      sizeof (PraghaList),
      0,                                           /* n_preallocs */
      (GInstanceInitFunc) pragha_list_init
    };

    pragha_list_type = g_type_register_static (G_TYPE_OBJECT, "PraghaList",
                                               &pragha_list_info, (GTypeFlags)0);
  }

  /* Here we register our GtkTreeModel interface with the type system */
  if (1)
  {
    static const GInterfaceInfo tree_model_info =
    {
      (GInterfaceInitFunc) pragha_list_tree_model_init,
      NULL,
      NULL
    };

    g_type_add_interface_static (pragha_list_type, GTK_TYPE_TREE_MODEL, &tree_model_info);
  }

  return pragha_list_type;
}


/*****************************************************************************
 *
 *  pragha_list_class_init: more boilerplate GObject/GType stuff.
 *                          Init callback for the type system,
 *                          called once when our new class is created.
 *
 *****************************************************************************/

static void
pragha_list_class_init (PraghaListClass *klass)
{
  GObjectClass *object_class;

  parent_class = (GObjectClass*) g_type_class_peek_parent (klass);
  object_class = (GObjectClass*) klass;

  object_class->finalize = pragha_list_finalize;
}

/*****************************************************************************
 *
 *  pragha_list_tree_model_init: init callback for the interface registration
 *                               in pragha_list_get_type. Here we override
 *                               the GtkTreeModel interface functions that
 *                               we implement.
 *
 *****************************************************************************/

static void
pragha_list_tree_model_init (GtkTreeModelIface *iface)
{
  iface->get_flags       = pragha_list_get_flags;
  iface->get_n_columns   = pragha_list_get_n_columns;
  iface->get_column_type = pragha_list_get_column_type;
  iface->get_iter        = pragha_list_get_iter;
  iface->get_path        = pragha_list_get_path;
  iface->get_value       = pragha_list_get_value;
  iface->iter_next       = pragha_list_iter_next;
  iface->iter_children   = pragha_list_iter_children;
  iface->iter_has_child  = pragha_list_iter_has_child;
  iface->iter_n_children = pragha_list_iter_n_children;
  iface->iter_nth_child  = pragha_list_iter_nth_child;
  iface->iter_parent     = pragha_list_iter_parent;
}


/*****************************************************************************
 *
 *  pragha_list_init: this is called everytime a new custom list object
 *                    instance is created (we do that in pragha_list_new).
 *                    Initialise the list structure's fields here.
 *
 *****************************************************************************/

static void
pragha_list_init (PraghaList *pragha_list)
{
  pragha_list->n_columns       = PRAGHA_LIST_N_COLUMNS;

  pragha_list->column_types[0] = G_TYPE_POINTER;  /* PRAGHA_LIST_COL_MOBJ    */
  pragha_list->column_types[1] = G_TYPE_UINT;   /* PRAGHA_LIST_COL_PLAYED      */
  pragha_list->column_types[2] = G_TYPE_STRING;  /* PRAGHA_LIST_COL_MOBJ    */
  pragha_list->column_types[3] = G_TYPE_STRING;   /* PRAGHA_LIST_COL_PLAYED      */
  pragha_list->column_types[4] = G_TYPE_STRING;  /* PRAGHA_LIST_COL_MOBJ    */
  pragha_list->column_types[5] = G_TYPE_STRING;   /* PRAGHA_LIST_COL_PLAYED      */
  pragha_list->column_types[6] = G_TYPE_UINT;  /* PRAGHA_LIST_COL_MOBJ    */
  pragha_list->column_types[7] = G_TYPE_UINT;   /* PRAGHA_LIST_COL_PLAYED      */
  pragha_list->column_types[8] = G_TYPE_STRING;  /* PRAGHA_LIST_COL_MOBJ    */
  pragha_list->column_types[9] = G_TYPE_UINT;   /* PRAGHA_LIST_COL_PLAYED      */
  pragha_list->column_types[10] = G_TYPE_STRING;  /* PRAGHA_LIST_COL_MOBJ    */

  //g_assert ( PRAGHA_LIST_N_COLUMNS == 13);

  pragha_list->num_rows = 0;
  pragha_list->rows     = NULL;

  pragha_list->stamp = g_random_int();  /* Random int to check whether an iter belongs to our model */
}


/*****************************************************************************
 *
 *  pragha_list_finalize: this is called just before a custom list is
 *                        destroyed. Free dynamically allocated memory here.
 *
 *****************************************************************************/

static void
pragha_list_finalize (GObject *object)
{
/*  PraghaList *pragha_list = PRAGHA_LIST(object); */

  /* free all songs and free all memory used by the list */
  #warning IMPLEMENT

  /* must chain up - finalize parent */
  (* parent_class->finalize) (object);
}


/*****************************************************************************
 *
 *  pragha_list_get_flags: tells the rest of the world whether our tree model
 *                         has any special characteristics. In our case,
 *                         we have a list model (instead of a tree), and each
 *                         tree iter is valid as long as the row in question
 *                         exists, as it only contains a pointer to our struct.
 *
 *****************************************************************************/

static GtkTreeModelFlags
pragha_list_get_flags (GtkTreeModel *tree_model)
{
  g_return_val_if_fail (PRAGHA_IS_LIST(tree_model), (GtkTreeModelFlags)0);

  return (GTK_TREE_MODEL_LIST_ONLY | GTK_TREE_MODEL_ITERS_PERSIST);
}


/*****************************************************************************
 *
 *  pragha_list_get_n_columns: tells the rest of the world how many data
 *                             columns we export via the tree model interface
 *
 *****************************************************************************/

static gint
pragha_list_get_n_columns (GtkTreeModel *tree_model)
{
  g_return_val_if_fail (PRAGHA_IS_LIST(tree_model), 0);

  return PRAGHA_LIST(tree_model)->n_columns;
}


/*****************************************************************************
 *
 *  pragha_list_get_column_type: tells the rest of the world which type of
 *                               data an exported model column contains
 *
 *****************************************************************************/

static GType
pragha_list_get_column_type (GtkTreeModel *tree_model,
                             gint          index)
{
  g_return_val_if_fail (PRAGHA_IS_LIST(tree_model), G_TYPE_INVALID);
  g_return_val_if_fail (index < PRAGHA_LIST(tree_model)->n_columns && index >= 0, G_TYPE_INVALID);

  return PRAGHA_LIST(tree_model)->column_types[index];
}


/*****************************************************************************
 *
 *  pragha_list_get_iter: converts a tree path (physical position) into a
 *                        tree iter structure (the content of the iter
 *                        fields will only be used internally by our model).
 *                        We simply store a pointer to our PraghaSong
 *                        structure that represents that row in the tree iter.
 *
 *****************************************************************************/

static gboolean
pragha_list_get_iter (GtkTreeModel *tree_model,
                      GtkTreeIter  *iter,
                      GtkTreePath  *path)
{
  PraghaList    *pragha_list;
  PraghaSong    *song;
  gint          *indices, n, depth;

  g_assert(PRAGHA_IS_LIST(tree_model));
  g_assert(path!=NULL);

  pragha_list = PRAGHA_LIST(tree_model);

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
 *  pragha_list_get_path: converts a tree iter into a tree path (ie. the
 *                        physical position of that row in the list).
 *
 *****************************************************************************/

static GtkTreePath *
pragha_list_get_path (GtkTreeModel *tree_model,
                      GtkTreeIter  *iter)
{
  GtkTreePath  *path;
  PraghaSong *song;
  PraghaList   *pragha_list;

  g_return_val_if_fail (PRAGHA_IS_LIST(tree_model), NULL);
  g_return_val_if_fail (iter != NULL,               NULL);
  g_return_val_if_fail (iter->user_data != NULL,    NULL);

  pragha_list = PRAGHA_LIST(tree_model);

  song = (PraghaSong*) iter->user_data;

  path = gtk_tree_path_new();
  gtk_tree_path_append_index(path, song->pos);

  return path;
}


/*****************************************************************************
 *
 *  pragha_list_get_value: Returns a row's exported data columns
 *                         (_get_value is what gtk_tree_model_get uses)
 *
 *****************************************************************************/

static void
pragha_list_get_value (GtkTreeModel *tree_model,
                       GtkTreeIter  *iter,
                       gint          column,
                       GValue       *value)
{
  PraghaSong    *song;
  PraghaList    *pragha_list;

  g_return_if_fail (PRAGHA_IS_LIST (tree_model));
  g_return_if_fail (iter != NULL);
  g_return_if_fail (column < PRAGHA_LIST(tree_model)->n_columns);

  g_value_init (value, PRAGHA_LIST(tree_model)->column_types[column]);

  pragha_list = PRAGHA_LIST(tree_model);

  song = (PraghaSong*) iter->user_data;

  g_return_if_fail ( song != NULL );

  if(song->pos >= pragha_list->num_rows)
   g_return_if_reached();

  switch(column)
  {
    case PRAGHA_LIST_COL_MOBJ:
      g_value_set_pointer(value, song->mobj);
      break;
    case PRAGHA_LIST_COL_TRACK_NO:
      g_value_set_uint(value, pragha_musicobject_get_track_no(song->mobj));
      break;
    case PRAGHA_LIST_COL_TITLE:
      g_value_set_string(value, pragha_musicobject_get_title(song->mobj));
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
      g_value_set_uint(value, pragha_musicobject_get_bitrate(song->mobj));
      break;
    case PRAGHA_LIST_COL_YEAR:
      g_value_set_uint(value, pragha_musicobject_get_year(song->mobj));
      break;
    case PRAGHA_LIST_COL_COMMENT:
      g_value_set_string(value, pragha_musicobject_get_comment(song->mobj));
      break;
    case PRAGHA_LIST_COL_LENGTH:
      g_value_set_uint(value, pragha_musicobject_get_length(song->mobj));
      break;
    case PRAGHA_LIST_COL_FILENAME:
      g_value_set_string(value, pragha_musicobject_get_file(song->mobj));
      break;
  }
}

/*****************************************************************************
 *
 *  pragha_list_iter_next: Takes an iter structure and sets it to point
 *                         to the next row.
 *
 *****************************************************************************/

static gboolean
pragha_list_iter_next (GtkTreeModel  *tree_model,
                       GtkTreeIter   *iter)
{
  PraghaSong  *song, *nextsong;
  PraghaList    *pragha_list;

  g_return_val_if_fail (PRAGHA_IS_LIST (tree_model), FALSE);

  if (iter == NULL || iter->user_data == NULL)
    return FALSE;

  pragha_list = PRAGHA_LIST(tree_model);

  song = (PraghaSong *) iter->user_data;

  /* Is this the last song in the list? */
  if ((song->pos + 1) >= pragha_list->num_rows)
    return FALSE;

  nextsong = pragha_list->rows[(song->pos + 1)];

  g_assert ( nextsong != NULL );
  g_assert ( nextsong->pos == (song->pos + 1) );

  iter->stamp     = pragha_list->stamp;
  iter->user_data = nextsong;

  return TRUE;
}


/*****************************************************************************
 *
 *  pragha_list_iter_children: Returns TRUE or FALSE depending on whether
 *                             the row specified by 'parent' has any children.
 *                             If it has children, then 'iter' is set to
 *                             point to the first child. Special case: if
 *                             'parent' is NULL, then the first top-level
 *                             row should be returned if it exists.
 *
 *****************************************************************************/

static gboolean
pragha_list_iter_children (GtkTreeModel *tree_model,
                           GtkTreeIter  *iter,
                           GtkTreeIter  *parent)
{
  PraghaList  *pragha_list;

  g_return_val_if_fail (parent == NULL || parent->user_data != NULL, FALSE);

  /* this is a list, nodes have no children */
  if (parent)
    return FALSE;

  /* parent == NULL is a special case; we need to return the first top-level row */

  g_return_val_if_fail (PRAGHA_IS_LIST (tree_model), FALSE);

  pragha_list = PRAGHA_LIST(tree_model);

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
 *  pragha_list_iter_has_child: Returns TRUE or FALSE depending on whether
 *                              the row specified by 'iter' has any children.
 *                              We only have a list and thus no children.
 *
 *****************************************************************************/

static gboolean
pragha_list_iter_has_child (GtkTreeModel *tree_model,
                            GtkTreeIter  *iter)
{
  return FALSE;
}


/*****************************************************************************
 *
 *  pragha_list_iter_n_children: Returns the number of children the row
 *                               specified by 'iter' has. This is usually 0,
 *                               as we only have a list and thus do not have
 *                               any children to any rows. A special case is
 *                               when 'iter' is NULL, in which case we need
 *                               to return the number of top-level nodes,
 *                               ie. the number of rows in our list.
 *
 *****************************************************************************/

static gint
pragha_list_iter_n_children (GtkTreeModel *tree_model,
                             GtkTreeIter  *iter)
{
  PraghaList  *pragha_list;

  g_return_val_if_fail (PRAGHA_IS_LIST (tree_model), -1);
  g_return_val_if_fail (iter == NULL || iter->user_data != NULL, FALSE);

  pragha_list = PRAGHA_LIST(tree_model);

  /* special case: if iter == NULL, return number of top-level rows */
  if (!iter)
    return pragha_list->num_rows;

  return 0; /* otherwise, this is easy again for a list */
}


/*****************************************************************************
 *
 *  pragha_list_iter_nth_child: If the row specified by 'parent' has any
 *                              children, set 'iter' to the n-th child and
 *                              return TRUE if it exists, otherwise FALSE.
 *                              A special case is when 'parent' is NULL, in
 *                              which case we need to set 'iter' to the n-th
 *                              row if it exists.
 *
 *****************************************************************************/

static gboolean
pragha_list_iter_nth_child (GtkTreeModel *tree_model,
                            GtkTreeIter  *iter,
                            GtkTreeIter  *parent,
                            gint          n)
{
  PraghaSong  *song;
  PraghaList    *pragha_list;

  g_return_val_if_fail (PRAGHA_IS_LIST (tree_model), FALSE);

  pragha_list = PRAGHA_LIST(tree_model);

  /* a list has only top-level rows */
  if(parent)
    return FALSE;

  /* special case: if parent == NULL, set iter to n-th top-level row */

  if( n >= pragha_list->num_rows )
    return FALSE;

  song = pragha_list->rows[n];

  g_assert( song != NULL );
  g_assert( song->pos == n );

  iter->stamp = pragha_list->stamp;
  iter->user_data = song;

  return TRUE;
}


/*****************************************************************************
 *
 *  pragha_list_iter_parent: Point 'iter' to the parent node of 'child'. As
 *                           we have a list and thus no children and no
 *                           parents of children, we can just return FALSE.
 *
 *****************************************************************************/

static gboolean
pragha_list_iter_parent (GtkTreeModel *tree_model,
                         GtkTreeIter  *iter,
                         GtkTreeIter  *child)
{
  return FALSE;
}


/*****************************************************************************
 *
 *  pragha_list_new:  This is what you use in your own code to create a
 *                    new custom list tree model for you to use.
 *
 *****************************************************************************/

PraghaList *
pragha_list_new (void)
{
  PraghaList *newpraghalist;

  newpraghalist = (PraghaList*) g_object_new (PRAGHA_TYPE_LIST, NULL);

  g_assert( newpraghalist != NULL );

  return newpraghalist;
}


/*****************************************************************************
 *
 *  pragha_list_append_song:  Empty lists are boring. This function can
 *                            be used in your own code to add rows to the
 *                            list. Note how we emit the "row-inserted"
 *                            signal after we have appended the row
 *                            internally, so the tree view and other
 *                            interested objects know about the new row.
 *
 *****************************************************************************/

void
pragha_list_append_song (PraghaList        *pragha_list,
                         PraghaMusicobject *mobj)
{
  GtkTreeIter   iter;
  GtkTreePath  *path;
  PraghaSong   *newsong;
  gulong        newsize;
  guint         pos;

  g_return_if_fail (PRAGHA_IS_LIST(pragha_list));

  pos = pragha_list->num_rows;

  pragha_list->num_rows++;

  newsize = pragha_list->num_rows * sizeof(PraghaSong*);

  pragha_list->rows = g_realloc(pragha_list->rows, newsize);

  newsong = g_new0(PraghaSong, 1);

  newsong->mobj = g_object_ref(mobj);

  pragha_list->rows[pos] = newsong;
  newsong->pos = pos;

  /* inform the tree view and other interested objects
   *  (e.g. tree row references) that we have inserted
   *  a new row, and where it was inserted */

  path = gtk_tree_path_new();
  gtk_tree_path_append_index(path, newsong->pos);

  pragha_list_get_iter(GTK_TREE_MODEL(pragha_list), &iter, path);

  gtk_tree_model_row_inserted(GTK_TREE_MODEL(pragha_list), path, &iter);

  gtk_tree_path_free(path);
}