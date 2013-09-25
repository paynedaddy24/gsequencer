/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2013 Joël Krähemann
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

#include <ags/audio/task/ags_export_wav.h>

#include <ags-lib/object/ags_connectable.h>

#include <ags/main.h>

void ags_export_wav_class_init(AgsExportWavClass *export_wav);
void ags_export_wav_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_export_wav_init(AgsExportWav *export_wav);
void ags_export_wav_connect(AgsConnectable *connectable);
void ags_export_wav_disconnect(AgsConnectable *connectable);
void ags_export_wav_finalize(GObject *gobject);

void ags_export_wav_launch(AgsTask *task);

static gpointer ags_export_wav_parent_class = NULL;
static AgsConnectableInterface *ags_export_wav_parent_connectable_interface;

GType
ags_export_wav_get_type()
{
  static GType ags_type_export_wav = 0;

  if(!ags_type_export_wav){
    static const GTypeInfo ags_export_wav_info = {
      sizeof (AgsExportWavClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_export_wav_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsExportWav),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_export_wav_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_export_wav_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_export_wav = g_type_register_static(AGS_TYPE_TASK,
						 "AgsExportWav\0",
						 &ags_export_wav_info,
						 0);

    g_type_add_interface_static(ags_type_export_wav,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_export_wav);
}

void
ags_export_wav_class_init(AgsExportWavClass *export_wav)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_export_wav_parent_class = g_type_class_peek_parent(export_wav);

  /* gobject */
  gobject = (GObjectClass *) export_wav;

  gobject->finalize = ags_export_wav_finalize;

  /* task */
  task = (AgsTaskClass *) export_wav;

  task->launch = ags_export_wav_launch;
}

void
ags_export_wav_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_export_wav_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_export_wav_connect;
  connectable->disconnect = ags_export_wav_disconnect;
}

void
ags_export_wav_init(AgsExportWav *export_wav)
{
  export_wav->flags = 0;
}

void
ags_export_wav_connect(AgsConnectable *connectable)
{
  ags_export_wav_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_export_wav_disconnect(AgsConnectable *connectable)
{
  ags_export_wav_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_export_wav_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_export_wav_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_export_wav_launch(AgsTask *task)
{
  AgsServer *server;
  AgsExportWav *export_wav;
  AgsAudioLoop *audio_loop;

  export_wav = AGS_EXPORT_WAV(task);

  audio_loop = AGS_AUDIO_LOOP(export_wav->audio_loop);

  /* to implement me */
}

AgsExportWav*
ags_export_wav_new(GObject *audio_loop,
		     AgsDevoutPlay *devout_play)
{
  AgsExportWav *export_wav;

  export_wav = (AgsExportWav *) g_object_new(AGS_TYPE_EXPORT_WAV,
					     NULL);

  return(export_wav);
}
