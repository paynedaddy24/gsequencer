/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2015 Joël Krähemann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __AGS_EFFECT_BULK_H__
#define __AGS_EFFECT_BULK_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <ags/audio/ags_audio.h>

#define AGS_TYPE_EFFECT_BULK                (ags_effect_bulk_get_type())
#define AGS_EFFECT_BULK(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_EFFECT_BULK, AgsEffectBulk))
#define AGS_EFFECT_BULK_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_EFFECT_BULK, AgsEffectBulkClass))
#define AGS_IS_EFFECT_BULK(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_EFFECT_BULK))
#define AGS_IS_EFFECT_BULK_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_EFFECT_BULK))
#define AGS_EFFECT_BULK_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_EFFECT_BULK, AgsEffectBulkClass))

#define AGS_EFFECT_BULK_PLUGIN(ptr) ((AgsEffectBulkPlugin *)(ptr))

#define AGS_EFFECT_BULK_DEFAULT_VERSION "0.4.3\0"
#define AGS_EFFECT_BULK_DEFAULT_BUILD_ID "CEST 20-03-2015 08:24\0"

#define AGS_EFFECT_BULK_COLUMNS_COUNT (4)

typedef struct _AgsEffectBulk AgsEffectBulk;
typedef struct _AgsEffectBulkClass AgsEffectBulkClass;

typedef struct _AgsEffectBulkPlugin AgsEffectBulkPlugin;

typedef enum{
  AGS_EFFECT_BULK_CONNECTED        = 1,
  AGS_EFFECT_BULK_HIDE_BUTTONS     = 1 << 1,
  AGS_EFFECT_BULK_HIDE_ENTRIES     = 1 << 2,
  AGS_EFFECT_BULK_SHOW_LABELS      = 1 << 3,
}AgsEffectBulkFlags;

struct _AgsEffectBulk
{
  GtkVBox vbox;

  guint flags;

  gchar *name;

  gchar *version;
  gchar *build_id;

  GType *channel_type;
  AgsAudio *audio;
  gulong set_audio_channels_handler;
  gulong set_pads_handler;

  GtkButton *add;
  GtkButton *remove;

  GtkVBox *bulk_member;
  GtkTable *table;

  GList *plugin;
  GtkDialog *plugin_browser;
};

struct _AgsEffectBulkClass
{
  GtkVBoxClass vbox;

  GList* (*add_effect)(AgsEffectBulk *effect_bulk,
		       gchar *filename,
		       gchar *effect);
  void (*remove_effect)(AgsEffectBulk *effect_bulk,
			guint nth);

  void (*resize_audio_channels)(AgsEffectBulk *effect_bulk,
				guint new_size,
				guint old_size);
  void (*resize_pads)(AgsEffectBulk *effect_bulk,
		      guint new_size,
		      guint old_size);
};

struct _AgsEffectBulkPlugin
{
  gchar *filename;
  gchar *effect;
};

GType ags_effect_bulk_get_type(void);

AgsEffectBulkPlugin* ags_effect_bulk_plugin_alloc(gchar *filename,
						  gchar *effect);

GList* ags_effect_bulk_add_effect(AgsEffectBulk *effect_bulk,
				  gchar *filename,
				  gchar *effect);
void ags_effect_bulk_remove_effect(AgsEffectBulk *effect_bulk,
				   guint nth);

void ags_effect_bulk_resize_audio_channels(AgsEffectBulk *effect_bulk,
					   guint new_size,
					   guint old_size);
void ags_effect_bulk_resize_pads(AgsEffectBulk *effect_bulk,
				 guint new_size,
				 guint old_size);

AgsEffectBulk* ags_effect_bulk_new(AgsAudio *audio,
				   GType channel_type);

#endif /*__AGS_EFFECT_BULK_H__*/
