/*************************************************************************/
/* Copyright (C) 2012 matias <mati86dl@gmail.com>			 */
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

#include "pragha-preferences.h"
#include "pragha.h"

G_DEFINE_TYPE(PraghaPreferences, pragha_preferences, G_TYPE_OBJECT)

struct _PraghaPreferencesPrivate
{
   GKeyFile  *rc_keyfile;
   gchar     *rc_filepath;

   /* Search preferences. */
   gboolean   instant_search;
   gboolean   approximate_search;
   /* Playlist preferences. */
   gboolean   shuffle;
   gboolean   repeat;
   gboolean   use_hint;
   gboolean   restore_playlist;
   /* Audio preferences. */
   gchar     *audio_sink;
   gchar     *audio_device;
   gboolean   software_mixer;
   gchar     *audio_cd_device;
   /* Window preferences. */
   gboolean   lateral_panel;
   /* Misc preferences. */
   gboolean   add_recursively;
};

enum
{
   PROP_0,
   PROP_INSTANT_SEARCH,
   PROP_APPROXIMATE_SEARCH,
   PROP_SHUFFLE,
   PROP_REPEAT,
   PROP_USE_HINT,
   PROP_RESTORE_PLAYLIST,
   PROP_AUDIO_SINK,
   PROP_AUDIO_DEVICE,
   PROP_SOFTWARE_MIXER,
   PROP_AUDIO_CD_DEVICE,
   PROP_LATERAL_PANEL,
   PROP_ADD_RECURSIVELY,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

/**
 * pragha_preferences_get_double_list:
 *
 */
gdouble *
pragha_preferences_get_double_list (PraghaPreferences *preferences,
                                    const gchar *group_name,
                                    const gchar *key)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), NULL);

   return g_key_file_get_double_list(preferences->priv->rc_keyfile,
                                     group_name,
                                     key,
                                     NULL,
                                     NULL);
}

/**
 * pragha_preferences_set_double_list
 *
 */
void
pragha_preferences_set_double_list (PraghaPreferences *preferences,
                                    const gchar *group_name,
                                    const gchar *key,
                                    gdouble list[],
                                    gsize length)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   g_key_file_set_double_list(preferences->priv->rc_keyfile,
                              group_name,
                              key,
                              list,
                              length);
}

/**
 * pragha_preferences_get_integer_list:
 *
 */
gint *
pragha_preferences_get_integer_list (PraghaPreferences *preferences,
                                     const gchar *group_name,
                                     const gchar *key,
                                     gsize *length)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), NULL);

   return g_key_file_get_integer_list(preferences->priv->rc_keyfile,
                                      group_name,
                                      key,
                                      length,
                                      NULL);
}

/**
 * pragha_preferences_set_integer_list
 *
 */
void
pragha_preferences_set_integer_list (PraghaPreferences *preferences,
                                     const gchar *group_name,
                                     const gchar *key,
                                     gint list[],
                                     gsize length)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   g_key_file_set_integer_list(preferences->priv->rc_keyfile,
                               group_name,
                               key,
                               list,
                               length);
}

/**
 * pragha_preferences_get_string:
 *
 */
gchar *
pragha_preferences_get_string (PraghaPreferences *preferences,
                               const gchar *group_name,
                               const gchar *key)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), NULL);

   return g_key_file_get_string(preferences->priv->rc_keyfile,
                                group_name,
                                key,
                                NULL);
}

/**
 * pragha_preferences_set_string:
 *
 */
void
pragha_preferences_set_string (PraghaPreferences *preferences,
                               const gchar *group_name,
                               const gchar *key,
                               const gchar *string)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   g_key_file_set_string(preferences->priv->rc_keyfile,
                         group_name,
                         key,
                         string);
}

/**
 * pragha_preferences_get_string_list:
 *
 */
gchar **
pragha_preferences_get_string_list (PraghaPreferences *preferences,
                                    const gchar *group_name,
                                    const gchar *key,
                                    gsize *length)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), NULL);

   return g_key_file_get_string_list(preferences->priv->rc_keyfile,
                                     group_name,
                                     key,
                                     length,
                                     NULL);
}

/**
 * pragha_preferences_set_string_list
 *
 */
void
pragha_preferences_set_string_list (PraghaPreferences *preferences,
                                     const gchar *group_name,
                                     const gchar *key,
                                     const gchar * const list[],
                                     gsize length)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   g_key_file_set_string_list(preferences->priv->rc_keyfile,
                              group_name,
                              key,
                              list,
                              length);
}

/**
 * pragha_preferences_remove_key:
 *
 */
void
pragha_preferences_remove_key (PraghaPreferences *preferences,
                               const gchar *group_name,
                               const gchar *key)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   if (g_key_file_has_group(preferences->priv->rc_keyfile, group_name) &&
       g_key_file_has_key(preferences->priv->rc_keyfile,
                          group_name,
                          key,
                          NULL))
      g_key_file_remove_key(preferences->priv->rc_keyfile,
                            group_name,
                            key,
                            NULL);
}

/**
 * pragha_preferences_get_instant_search:
 *
 */
gboolean
pragha_preferences_get_instant_search (PraghaPreferences *preferences)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), FALSE);

   return preferences->priv->instant_search;
}

/**
 * pragha_preferences_set_instant_search:
 *
 */
void
pragha_preferences_set_instant_search (PraghaPreferences *preferences,
                                       gboolean instant_search)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   preferences->priv->instant_search = instant_search;

   g_object_notify_by_pspec(G_OBJECT(preferences), gParamSpecs[PROP_INSTANT_SEARCH]);
}


/**
 * pragha_preferences_get_approximate_search:
 *
 */
gboolean
pragha_preferences_get_approximate_search (PraghaPreferences *preferences)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), FALSE);

   return preferences->priv->approximate_search;
}

/**
 * pragha_preferences_set_approximate_search:
 *
 */
void
pragha_preferences_set_approximate_search (PraghaPreferences *preferences,
                                           gboolean approximate_search)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   preferences->priv->approximate_search = approximate_search;

   g_object_notify_by_pspec(G_OBJECT(preferences), gParamSpecs[PROP_APPROXIMATE_SEARCH]);
}

/**
 * pragha_preferences_get_shuffle:
 *
 */
gboolean
pragha_preferences_get_shuffle (PraghaPreferences *preferences)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), FALSE);

   return preferences->priv->shuffle;
}

/**
 * pragha_preferences_set_shuffle:
 *
 */
void
pragha_preferences_set_shuffle (PraghaPreferences *preferences,
                                gboolean shuffle)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   preferences->priv->shuffle = shuffle;

   g_object_notify_by_pspec(G_OBJECT(preferences), gParamSpecs[PROP_SHUFFLE]);
}

/**
 * pragha_preferences_get_repeat:
 *
 */
gboolean
pragha_preferences_get_repeat (PraghaPreferences *preferences)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), FALSE);

   return preferences->priv->repeat;
}

/**
 * pragha_preferences_set_repeat:
 *
 */
void
pragha_preferences_set_repeat (PraghaPreferences *preferences,
                               gboolean repeat)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   preferences->priv->repeat = repeat;

   g_object_notify_by_pspec(G_OBJECT(preferences), gParamSpecs[PROP_REPEAT]);
}

/**
 * pragha_preferences_get_use_hint:
 *
 */
gboolean
pragha_preferences_get_use_hint (PraghaPreferences *preferences)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), FALSE);

   return preferences->priv->use_hint;
}

/**
 * pragha_preferences_set_use_hint:
 *
 */
void
pragha_preferences_set_use_hint (PraghaPreferences *preferences,
                                 gboolean use_hint)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   preferences->priv->use_hint = use_hint;

   g_object_notify_by_pspec(G_OBJECT(preferences), gParamSpecs[PROP_USE_HINT]);
}

/**
 * pragha_preferences_get_restore_playlist:
 *
 */
gboolean
pragha_preferences_get_restore_playlist (PraghaPreferences *preferences)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), FALSE);

   return preferences->priv->restore_playlist;
}

/**
 * pragha_preferences_set_restore_playlist:
 *
 */
void
pragha_preferences_set_restore_playlist (PraghaPreferences *preferences,
                                         gboolean restore_playlist)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   preferences->priv->restore_playlist = restore_playlist;

   g_object_notify_by_pspec(G_OBJECT(preferences), gParamSpecs[PROP_RESTORE_PLAYLIST]);
}

/**
 * pragha_preferences_get_audio_sink:
 *
 */
const gchar *
pragha_preferences_get_audio_sink (PraghaPreferences *preferences)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), NULL);

   return preferences->priv->audio_sink;
}

/**
 * pragha_preferences_set_audio_sink:
 *
 */
void
pragha_preferences_set_audio_sink (PraghaPreferences *preferences,
                                   const gchar *audio_sink)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   g_free(preferences->priv->audio_sink);
   preferences->priv->audio_sink = g_strdup(audio_sink);

   g_object_notify_by_pspec(G_OBJECT(preferences), gParamSpecs[PROP_AUDIO_SINK]);
}

/**
 * pragha_preferences_get_audio_device:
 *
 */
const gchar *
pragha_preferences_get_audio_device (PraghaPreferences *preferences)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), NULL);

   return preferences->priv->audio_device;
}

/**
 * pragha_preferences_set_audio_device:
 *
 */
void
pragha_preferences_set_audio_device (PraghaPreferences *preferences,
                                     const gchar *audio_device)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   g_free(preferences->priv->audio_device);
   preferences->priv->audio_device = g_strdup(audio_device);

   g_object_notify_by_pspec(G_OBJECT(preferences), gParamSpecs[PROP_AUDIO_DEVICE]);
}

/**
 * pragha_preferences_get_software_mixer:
 *
 */
gboolean
pragha_preferences_get_software_mixer (PraghaPreferences *preferences)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), FALSE);

   return preferences->priv->software_mixer;
}

/**
 * pragha_preferences_set_software_mixer:
 *
 */
void
pragha_preferences_set_software_mixer (PraghaPreferences *preferences,
                                       gboolean software_mixer)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   preferences->priv->software_mixer = software_mixer;

   g_object_notify_by_pspec(G_OBJECT(preferences), gParamSpecs[PROP_SOFTWARE_MIXER]);
}

/**
 * pragha_preferences_get_audio_cd_device:
 *
 */
const gchar *
pragha_preferences_get_audio_cd_device (PraghaPreferences *preferences)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), NULL);

   return preferences->priv->audio_cd_device;
}

/**
 * pragha_preferences_set_audio_cd_device:
 *
 */
void
pragha_preferences_set_audio_cd_device (PraghaPreferences *preferences,
                                        const gchar *audio_cd_device)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   g_free(preferences->priv->audio_cd_device);
   preferences->priv->audio_cd_device = g_strdup(audio_cd_device);

   g_object_notify_by_pspec(G_OBJECT(preferences), gParamSpecs[PROP_AUDIO_CD_DEVICE]);
}

/**
 * pragha_preferences_get_lateral_panel:
 *
 */
gboolean
pragha_preferences_get_lateral_panel (PraghaPreferences *preferences)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), FALSE);

   return preferences->priv->lateral_panel;
}

/**
 * pragha_preferences_set_lateral_panel:
 *
 */
void
pragha_preferences_set_lateral_panel (PraghaPreferences *preferences,
                                      gboolean lateral_panel)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   preferences->priv->lateral_panel = lateral_panel;

   g_object_notify_by_pspec(G_OBJECT(preferences), gParamSpecs[PROP_LATERAL_PANEL]);
}

/**
 * pragha_preferences_get_add_recursively:
 *
 */
gboolean
pragha_preferences_get_add_recursively (PraghaPreferences *preferences)
{
   g_return_val_if_fail(PRAGHA_IS_PREFERENCES(preferences), FALSE);

   return preferences->priv->add_recursively;
}

/**
 * pragha_preferences_set_add_recursively:
 *
 */
void
pragha_preferences_set_add_recursively(PraghaPreferences *preferences,
                                       gboolean add_recursively)
{
   g_return_if_fail(PRAGHA_IS_PREFERENCES(preferences));

   preferences->priv->add_recursively = add_recursively;

   g_object_notify_by_pspec(G_OBJECT(preferences), gParamSpecs[PROP_ADD_RECURSIVELY]);
}

static void
pragha_preferences_finalize (GObject *object)
{
   gchar *data = NULL;
   gsize length;
   GError *error = NULL;

   PraghaPreferences *preferences = PRAGHA_PREFERENCES(object);
   PraghaPreferencesPrivate *priv = preferences->priv;

   /* Store new preferences */

   g_key_file_set_boolean(priv->rc_keyfile,
                          GROUP_GENERAL,
                          KEY_INSTANT_SEARCH,
                          priv->instant_search);

   g_key_file_set_boolean(priv->rc_keyfile,
                          GROUP_GENERAL,
                          KEY_APPROXIMATE_SEARCH,
                          priv->approximate_search);

   g_key_file_set_boolean(priv->rc_keyfile,
                          GROUP_PLAYLIST,
                          KEY_SHUFFLE,
                          priv->shuffle);
   g_key_file_set_boolean(priv->rc_keyfile,
                          GROUP_PLAYLIST,
                          KEY_REPEAT,
                          priv->repeat);
   g_key_file_set_boolean(priv->rc_keyfile,
                          GROUP_GENERAL,
                          KEY_USE_HINT,
                          priv->use_hint);
   g_key_file_set_boolean(priv->rc_keyfile,
                          GROUP_PLAYLIST,
                          KEY_SAVE_PLAYLIST,
                          priv->restore_playlist);
   g_key_file_set_string(priv->rc_keyfile,
			 GROUP_AUDIO,
			 KEY_AUDIO_SINK,
			 priv->audio_sink);
   g_key_file_set_string(priv->rc_keyfile,
			 GROUP_AUDIO,
			 KEY_AUDIO_DEVICE,
			 priv->audio_device);
   g_key_file_set_boolean(priv->rc_keyfile,
                          GROUP_AUDIO,
                          KEY_SOFTWARE_MIXER,
                          priv->software_mixer);
   if (string_is_not_empty(priv->audio_cd_device))
      g_key_file_set_string(priv->rc_keyfile,
                            GROUP_AUDIO,
                            KEY_AUDIO_CD_DEVICE,
                            priv->audio_cd_device);
   else
      pragha_preferences_remove_key(preferences,
                                    GROUP_AUDIO,
                                    KEY_AUDIO_CD_DEVICE);
   g_key_file_set_boolean(priv->rc_keyfile,
                          GROUP_WINDOW,
                          KEY_SIDEBAR,
                          priv->lateral_panel);
   g_key_file_set_boolean(priv->rc_keyfile,
                          GROUP_GENERAL,
                          KEY_ADD_RECURSIVELY_FILES,
                          priv->add_recursively);
   /* Save to key file */

   data = g_key_file_to_data(priv->rc_keyfile, &length, NULL);
   if(!g_file_set_contents(priv->rc_filepath, data, length, &error))
      g_critical("Unable to write preferences file : %s", error->message);

   g_free(data);
   g_key_file_free(priv->rc_keyfile);
   g_free(priv->rc_filepath);
   g_free(priv->audio_sink);
   g_free(priv->audio_device);
   g_free(priv->audio_cd_device);

   G_OBJECT_CLASS(pragha_preferences_parent_class)->finalize(object);
}

static void
pragha_preferences_get_property (GObject *object,
                                 guint prop_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
   PraghaPreferences *preferences = PRAGHA_PREFERENCES(object);

   switch (prop_id) {
   case PROP_INSTANT_SEARCH:
      g_value_set_boolean (value, pragha_preferences_get_instant_search(preferences));
      break;
   case PROP_APPROXIMATE_SEARCH:
      g_value_set_boolean (value, pragha_preferences_get_instant_search(preferences));
      break;
   case PROP_SHUFFLE:
      g_value_set_boolean (value, pragha_preferences_get_shuffle(preferences));
      break;
   case PROP_REPEAT:
      g_value_set_boolean (value, pragha_preferences_get_repeat(preferences));
      break;
   case PROP_USE_HINT:
      g_value_set_boolean (value, pragha_preferences_get_use_hint(preferences));
      break;
   case PROP_RESTORE_PLAYLIST:
      g_value_set_boolean (value, pragha_preferences_get_restore_playlist(preferences));
      break;
   case PROP_AUDIO_SINK:
      g_value_set_string (value, pragha_preferences_get_audio_sink(preferences));
      break;
   case PROP_AUDIO_DEVICE:
      g_value_set_string (value, pragha_preferences_get_audio_device(preferences));
      break;
   case PROP_SOFTWARE_MIXER:
      g_value_set_boolean (value, pragha_preferences_get_software_mixer(preferences));
      break;
   case PROP_AUDIO_CD_DEVICE:
      g_value_set_string (value, pragha_preferences_get_audio_cd_device(preferences));
      break;
   case PROP_LATERAL_PANEL:
      g_value_set_boolean (value, pragha_preferences_get_lateral_panel(preferences));
      break;
   case PROP_ADD_RECURSIVELY:
      g_value_set_boolean (value, pragha_preferences_get_add_recursively(preferences));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
pragha_preferences_set_property (GObject *object,
                                 guint prop_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
   PraghaPreferences *preferences = PRAGHA_PREFERENCES(object);

   switch (prop_id) {
   case PROP_INSTANT_SEARCH:
      pragha_preferences_set_instant_search(preferences, g_value_get_boolean(value));
      break;
   case PROP_APPROXIMATE_SEARCH:
      pragha_preferences_set_approximate_search(preferences, g_value_get_boolean(value));
      break;
   case PROP_SHUFFLE:
      pragha_preferences_set_shuffle(preferences, g_value_get_boolean(value));
      break;
   case PROP_REPEAT:
      pragha_preferences_set_repeat(preferences, g_value_get_boolean(value));
      break;
   case PROP_USE_HINT:
      pragha_preferences_set_use_hint(preferences, g_value_get_boolean(value));
      break;
   case PROP_RESTORE_PLAYLIST:
      pragha_preferences_set_restore_playlist(preferences, g_value_get_boolean(value));
      break;
   case PROP_AUDIO_SINK:
      pragha_preferences_set_audio_sink(preferences, g_value_get_string(value));
      break;
   case PROP_AUDIO_DEVICE:
      pragha_preferences_set_audio_device(preferences, g_value_get_string(value));
      break;
   case PROP_SOFTWARE_MIXER:
      pragha_preferences_set_software_mixer(preferences, g_value_get_boolean(value));
      break;
   case PROP_AUDIO_CD_DEVICE:
      pragha_preferences_set_audio_cd_device(preferences, g_value_get_string(value));
      break;
   case PROP_LATERAL_PANEL:
      pragha_preferences_set_lateral_panel(preferences, g_value_get_boolean(value));
      break;
   case PROP_ADD_RECURSIVELY:
      pragha_preferences_set_add_recursively(preferences, g_value_get_boolean(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
pragha_preferences_class_init (PraghaPreferencesClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = pragha_preferences_finalize;
   object_class->get_property = pragha_preferences_get_property;
   object_class->set_property = pragha_preferences_set_property;
   g_type_class_add_private(object_class, sizeof(PraghaPreferencesPrivate));

   /**
    * PraghaPreferences:instant_search:
    *
    */
   gParamSpecs[PROP_INSTANT_SEARCH] =
      g_param_spec_boolean("instant-search",
                           "InstantSearch",
                           "Instant Search Preference",
                           TRUE,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS);

   /**
    * PraghaPreferences:approximate_searches:
    *
    */
   gParamSpecs[PROP_APPROXIMATE_SEARCH] =
      g_param_spec_boolean("approximate-searches",
                           "ApproximateSearches",
                           "Approximate Searches Preference",
                           FALSE,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS);

   /**
    * PraghaPreferences:shuffle:
    *
    */
   gParamSpecs[PROP_SHUFFLE] =
      g_param_spec_boolean("shuffle",
                           "Shuffle",
                           "Shuffle Preference",
                           FALSE,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS);

   /**
    * PraghaPreferences:repeat:
    *
    */
   gParamSpecs[PROP_REPEAT] =
      g_param_spec_boolean("repeat",
                           "Repeat",
                           "Repeat Preference",
                           FALSE,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS);

   /**
    * PraghaPreferences:use_hint:
    *
    */
   gParamSpecs[PROP_USE_HINT] =
      g_param_spec_boolean("use-hint",
                           "UseHint",
                           "Use hint Preference",
                           FALSE,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS);

   /**
    * PraghaPreferences:restore_playlist:
    *
    */
   gParamSpecs[PROP_RESTORE_PLAYLIST] =
      g_param_spec_boolean("restore-playlist",
                           "RestorePlaylist",
                           "Restore Playlist Preference",
                           FALSE,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS);

   /**
    * PraghaPreferences:audio_sink:
    *
    */
   gParamSpecs[PROP_AUDIO_SINK] =
      g_param_spec_string("audio-sink",
                          "AudioSink",
                          "Audio Sink",
                          DEFAULT_SINK,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS);

   /**
    * PraghaPreferences:audio_device:
    *
    */
   gParamSpecs[PROP_AUDIO_DEVICE] =
      g_param_spec_string("audio-device",
                          "AudioDevice",
                          "Audio Device",
                          ALSA_DEFAULT_DEVICE,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS);

   /**
    * PraghaPreferences:software_mixer:
    *
    */
   gParamSpecs[PROP_SOFTWARE_MIXER] =
      g_param_spec_boolean("software-mixer",
                           "SoftwareMixer",
                           "Use Software Mixer",
                           FALSE,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS);

   /**
    * PraghaPreferences:audio_cd_device:
    *
    */
   gParamSpecs[PROP_AUDIO_CD_DEVICE] =
      g_param_spec_string("audio-cd-device",
                          "AudioCDDevice",
                          "Audio CD Device",
                          NULL,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS);

   /**
    * PraghaPreferences:lateral_panel:
    *
    */
   gParamSpecs[PROP_LATERAL_PANEL] =
      g_param_spec_boolean("lateral-panel",
                           "LateralPanel",
                           "Show Lateral Panel Preference",
                           FALSE,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS);

   /**
    * PraghaPreferences:add_recursively:
    *
    */
   gParamSpecs[PROP_ADD_RECURSIVELY] =
      g_param_spec_boolean("add-recursively",
                           "AddRecursively",
                           "Add Recursively Preference",
                           FALSE,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS);

   g_object_class_install_properties(object_class, LAST_PROP, gParamSpecs);
}

static void
pragha_preferences_init (PraghaPreferences *preferences)
{
   gboolean approximate_search, instant_search;
   gboolean shuffle, repeat, use_hint, restore_playlist, lateral_panel, software_mixer;
   gboolean add_recursively;
   gchar *audio_sink, *audio_device, *audio_cd_device;
   const gchar *user_config_dir;
   gchar *pragha_config_dir = NULL;
   GError *error = NULL;

   preferences->priv = G_TYPE_INSTANCE_GET_PRIVATE(preferences,
                                                   PRAGHA_TYPE_PREFERENCES,
                                                   PraghaPreferencesPrivate);

   PraghaPreferencesPrivate *priv = preferences->priv;

   /* First check preferences folder or create it */

   user_config_dir = g_get_user_config_dir();
   pragha_config_dir = g_build_path(G_DIR_SEPARATOR_S, user_config_dir, "/pragha", NULL);

   if (g_file_test(pragha_config_dir, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR) == FALSE) {
      if (g_mkdir(pragha_config_dir, S_IRWXU) == -1) {
         g_free(pragha_config_dir);
         g_critical("Unable to create preferences directory, err: %s", strerror(errno));
         return;
      }
      CDEBUG(DBG_INFO, "Created .config/pragha folder");
   }
   g_free(pragha_config_dir);

   /* Does /pragha/config exist ? */

   priv->rc_filepath = g_build_path(G_DIR_SEPARATOR_S, user_config_dir, "/pragha/config", NULL);

   if (g_file_test(priv->rc_filepath, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) == FALSE) {
      if (g_creat(priv->rc_filepath, S_IRWXU) == -1) {
         g_free(priv->rc_filepath);
         priv->rc_filepath = NULL;
         g_critical("Unable to create config file, err: %s", strerror(errno));
         return;
      }
      CDEBUG(DBG_INFO, "Created .config/pragha/config file");
   }

   /* Open the preferences storage file */

   priv->rc_keyfile = g_key_file_new();

   if (!g_key_file_load_from_file(priv->rc_keyfile,
                                  priv->rc_filepath,
                                  G_KEY_FILE_NONE,
                                  &error)) {
      g_critical("Unable to load config file (Possible first start), err: %s", error->message);
      g_error_free(error);
      return;
   }

   /* Open last preferences */

   approximate_search = g_key_file_get_boolean(priv->rc_keyfile,
                                               GROUP_GENERAL,
                                               KEY_APPROXIMATE_SEARCH,
                                               &error);
   if (error) {
      g_error_free(error);
      error = NULL;
   }
   else {
      pragha_preferences_set_approximate_search(preferences, approximate_search);
   }

   instant_search = g_key_file_get_boolean(priv->rc_keyfile,
                                           GROUP_GENERAL,
                                           KEY_INSTANT_SEARCH,
                                           &error);
   if (error) {
      g_error_free(error);
      error = NULL;
   }
   else {
      pragha_preferences_set_instant_search(preferences, instant_search);
   }

   shuffle = g_key_file_get_boolean(priv->rc_keyfile,
                                    GROUP_PLAYLIST,
                                    KEY_SHUFFLE,
                                    &error);
   if (error) {
      g_error_free(error);
      error = NULL;
   }
   else {
      pragha_preferences_set_shuffle(preferences, shuffle);
   }

   repeat = g_key_file_get_boolean(priv->rc_keyfile,
                                   GROUP_PLAYLIST,
                                   KEY_REPEAT,
                                   &error);
   if (error) {
      g_error_free(error);
      error = NULL;
   }
   else {
      pragha_preferences_set_repeat(preferences, repeat);
   }

   use_hint = g_key_file_get_boolean(priv->rc_keyfile,
                                     GROUP_GENERAL,
                                     KEY_USE_HINT,
                                     &error);
   if (error) {
      g_error_free(error);
      error = NULL;
   }
   else {
      pragha_preferences_set_use_hint(preferences, use_hint);
   }

   restore_playlist = g_key_file_get_boolean(priv->rc_keyfile,
                                             GROUP_PLAYLIST,
                                             KEY_SAVE_PLAYLIST,
                                             &error);
   if (error) {
      g_error_free(error);
      error = NULL;
   }
   else {
      pragha_preferences_set_restore_playlist(preferences, restore_playlist);
   }

   audio_sink = g_key_file_get_string(priv->rc_keyfile,
                                      GROUP_AUDIO,
                                      KEY_AUDIO_SINK,
                                      &error);
   if (error) {
      g_error_free(error);
      error = NULL;
   }
   else {
      pragha_preferences_set_audio_sink(preferences, audio_sink);
   }

   audio_device = g_key_file_get_string(priv->rc_keyfile,
                                        GROUP_AUDIO,
                                        KEY_AUDIO_DEVICE,
                                        &error);
   if (error) {
      g_error_free(error);
      error = NULL;
   }
   else {
      pragha_preferences_set_audio_device(preferences, audio_device);
   }

   software_mixer = g_key_file_get_boolean(priv->rc_keyfile,
                                           GROUP_AUDIO,
                                           KEY_SOFTWARE_MIXER,
                                           &error);
   if (error) {
      g_error_free(error);
      error = NULL;
   }
   else {
      pragha_preferences_set_software_mixer(preferences, software_mixer);
   }

   audio_cd_device = g_key_file_get_string(priv->rc_keyfile,
                                           GROUP_AUDIO,
                                           KEY_AUDIO_CD_DEVICE,
                                           &error);
   if (error) {
      g_error_free(error);
      error = NULL;
   }
   else {
      pragha_preferences_set_audio_cd_device(preferences, audio_cd_device);
   }

   lateral_panel = g_key_file_get_boolean(priv->rc_keyfile,
                                          GROUP_WINDOW,
                                          KEY_SIDEBAR,
                                          &error);
   if (error) {
      g_error_free(error);
      error = NULL;
   }
   else {
      pragha_preferences_set_lateral_panel(preferences, lateral_panel);
   }

   add_recursively = g_key_file_get_boolean(priv->rc_keyfile,
                                            GROUP_GENERAL,
                                            KEY_ADD_RECURSIVELY_FILES,
                                            &error);
   if (error) {
      g_error_free(error);
      error = NULL;
   }
   else {
      pragha_preferences_set_add_recursively(preferences, add_recursively);
   }

   g_free(audio_sink);
   g_free(audio_device);
   g_free(audio_cd_device);
}

GKeyFile*
pragha_preferences_share_key_file(PraghaPreferences *preferences)
{
	return preferences->priv->rc_keyfile;
}

gchar*
pragha_preferences_share_filepath(PraghaPreferences *preferences)
{
	return preferences->priv->rc_filepath;
}

/**
 * pragha_preferences_get:
 *
 * Queries the global #PraghaPreferences instance, which is shared
 * by all modules. The function automatically takes a reference
 * for the caller, so you'll need to call g_object_unref() when
 * you're done with it.
 *
 * Return value: the global #PraghaPreferences instance.
 **/
PraghaPreferences*
pragha_preferences_get (void)
{
   static PraghaPreferences *preferences = NULL;

   if (G_UNLIKELY (preferences == NULL)) {
      preferences = g_object_new(PRAGHA_TYPE_PREFERENCES, NULL);
      g_object_add_weak_pointer(G_OBJECT (preferences),
                                (gpointer) &preferences);
   }
   else {
      g_object_ref (G_OBJECT (preferences));
   }

   return preferences;
}
