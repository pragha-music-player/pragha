

#ifndef __PRAGHA_PLUGINS_STORE_H__
#define __PRAGHA_PLUGINS_STORE_H__

#include <gtk/gtk.h>
#include <libpeas/peas-engine.h>
#include <libpeas/peas-plugin-info.h>

G_BEGIN_DECLS

typedef enum {
	PRAGHA_PLUGINS_STORE_ENABLED_COLUMN = 0,
	PRAGHA_PLUGINS_STORE_CAN_ENABLE_COLUMN,
	PRAGHA_PLUGINS_STORE_ICON_GICON_COLUMN,
	PRAGHA_PLUGINS_STORE_ICON_VISIBLE_COLUMN,
	PRAGHA_PLUGINS_STORE_INFO_COLUMN,
	PRAGHA_PLUGINS_STORE_INFO_SENSITIVE_COLUMN,
	PRAGHA_PLUGINS_STORE_PLUGIN_COLUMN,
	PRAGHA_PLUGINS_STORE_N_COLUMNS
} PraghaPluginsStoreColumns;

/*
 * The plugin store.
 */
#define PRAGHA_TYPE_PLUGINS_STORE            (pragha_plugins_store_get_type())
#define PRAGHA_PLUGINS_STORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PRAGHA_TYPE_PLUGINS_STORE, PraghaPluginsStore))
#define PRAGHA_PLUGINs_STORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PRAGHA_TYPE_PLUGINS_STORE, PraghaPluginsStoreClass))
#define PRAGHA_IS_PLUGINS_STORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PRAGHA_TYPE_PLUGINS_STORE))
#define PRAGHA_IS_PLUGINS_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PRAGHA_TYPE_PLUGINS_STORE))
#define PRAGHA_PLUGINS_STORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PRAGHA_TYPE_PLUGINS_STORE, PraghaPluginsStoreClass))

typedef struct _PraghaPluginsStore         PraghaPluginsStore;
typedef struct _PraghaPluginsStoreClass    PraghaPluginsStoreClass;
typedef struct _PraghaPluginsStorePrivate  PraghaPluginsStorePrivate;

struct _PraghaPluginsStore {
	GtkListStore parent;
	/*< private > */
	PraghaPluginsStorePrivate *priv;
};
struct _PraghaPluginsStoreClass {
	GtkListStoreClass parent_class;
};

GType                pragha_plugins_store_get_type              (void) G_GNUC_CONST;
PraghaPluginsStore  *pragha_plugins_store_new                   (PeasEngine           *engine);

void                 pragha_plugins_store_reload                (PraghaPluginsStore   *store);

void                 pragha_plugins_store_set_enabled           (PraghaPluginsStore   *store,
                                                                 GtkTreeIter          *iter,
                                                                 gboolean              enabled);
gboolean             pragha_plugins_store_get_enabled           (PraghaPluginsStore   *store,
                                                                 GtkTreeIter          *iter);
void                 pragha_plugins_store_set_all_enabled       (PraghaPluginsStore   *store,
                                                                 gboolean              enabled);
void                 pragha_plugins_store_toggle_enabled        (PraghaPluginsStore   *store,
                                                                 GtkTreeIter          *iter);

gboolean             pragha_plugins_store_can_enable            (PraghaPluginsStore   *store,
                                                                 GtkTreeIter          *iter);

PeasPluginInfo      *pragha_plugins_store_get_plugin            (PraghaPluginsStore   *store,
                                                                 GtkTreeIter          *iter);

gboolean             pragha_plugins_store_get_iter_from_plugin  (PraghaPluginsStore   *store,
                                                                 GtkTreeIter          *iter,
                                                                 const PeasPluginInfo *info);

/*
 * The view
 */
#define PRAGHA_TYPE_PLUGINS_VIEW             (pragha_plugins_view_get_type())
#define PRAGHA_PLUGINS_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), PRAGHA_TYPE_PLUGINS_VIEW, PraghaPluginsView))
#define PRAGHA_PLUGINS_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), PRAGHA_TYPE_PLUGINS_VIEW, PraghaPluginsViewClass))
#define PRAGHA_IS_PLUGINS_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), PRAGHA_TYPE_PLUGINS_VIEW))
#define PRAGHA_IS_PLUGINS_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), PRAGHA_TYPE_PLUGINS_VIEW))
#define PRAGHA_PLUGINS_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), PRAGHA_TYPE_PLUGINS_VIEW, PraghaPluginsViewClass))

typedef struct _PraghaPluginsView        PraghaPluginsView;
typedef struct _PraghaPluginsViewClass   PraghaPluginsViewClass;
typedef struct _PraghaPluginsViewPrivate PraghaPluginsViewPrivate;

struct _PraghaPluginsView {
	GtkTreeView parent;
	/*< private > */
	PraghaPluginsViewPrivate *priv;
};

struct _PraghaPluginsViewClass {
	GtkTreeViewClass parent_class;

	void  (*populate_popup)   (PraghaPluginsView *view,
	                           GtkMenu           *menu);

	/*< private >*/
	gpointer padding[8];
};

GType           pragha_plugins_view_get_type             (void) G_GNUC_CONST;
GtkWidget      *pragha_plugins_view_new                  (PeasEngine        *engine);

void            pragha_plugins_view_set_show_builtin     (PraghaPluginsView *view,
                                                          gboolean           show_builtin);
gboolean        pragha_plugins_view_get_show_builtin     (PraghaPluginsView *view);

void            pragha_plugins_view_set_selected_plugin  (PraghaPluginsView *view,
                                                          PeasPluginInfo   *info);
PeasPluginInfo *pragha_plugins_view_get_selected_plugin  (PraghaPluginsView *view);

G_END_DECLS

#endif /* __PRAGHA_PLUGINS_STORE_H__  */