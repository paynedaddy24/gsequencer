/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2005-2011 Joël Krähemann
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

#include <ags/audio/recall/ags_copy_audio_signal.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_run_connectable.h>

#include <ags/audio/ags_devout.h>
#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_id.h>

#include <ags/audio/recall/ags_copy_channel.h>

#include <stdlib.h>

void ags_copy_audio_signal_class_init(AgsCopyAudioSignalClass *copy_audio_signal);
void ags_copy_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_copy_audio_signal_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable);
void ags_copy_audio_signal_init(AgsCopyAudioSignal *copy_audio_signal);
void ags_copy_audio_signal_connect(AgsConnectable *connectable);
void ags_copy_audio_signal_disconnect(AgsConnectable *connectable);
void ags_copy_audio_signal_run_connect(AgsRunConnectable *run_connectable);
void ags_copy_audio_signal_run_disconnect(AgsRunConnectable *run_connectable);
void ags_copy_audio_signal_finalize(GObject *gobject);

void ags_copy_audio_signal_run_inter(AgsRecall *recall);
AgsRecall* ags_copy_audio_signal_duplicate(AgsRecall *recall,
					   AgsRecallID *recall_id,
					   guint *n_params, GParameter *parameter);

static gpointer ags_copy_audio_signal_parent_class = NULL;
static AgsConnectableInterface *ags_copy_audio_signal_parent_connectable_interface;
static AgsRunConnectableInterface *ags_copy_audio_signal_parent_run_connectable_interface;

GType
ags_copy_audio_signal_get_type()
{
  static GType ags_type_copy_audio_signal = 0;

  if(!ags_type_copy_audio_signal){
    static const GTypeInfo ags_copy_audio_signal_info = {
      sizeof (AgsCopyAudioSignalClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_copy_audio_signal_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsCopyAudioSignal),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_copy_audio_signal_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_copy_audio_signal_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_run_connectable_interface_info = {
      (GInterfaceInitFunc) ags_copy_audio_signal_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_copy_audio_signal = g_type_register_static(AGS_TYPE_RECALL_AUDIO_SIGNAL,
							"AgsCopyAudioSignal\0",
							&ags_copy_audio_signal_info,
							0);

    g_type_add_interface_static(ags_type_copy_audio_signal,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_copy_audio_signal,
				AGS_TYPE_RUN_CONNECTABLE,
				&ags_run_connectable_interface_info);
  }

  return(ags_type_copy_audio_signal);
}

void
ags_copy_audio_signal_class_init(AgsCopyAudioSignalClass *copy_audio_signal)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;

  ags_copy_audio_signal_parent_class = g_type_class_peek_parent(copy_audio_signal);

  /* GObjectClass */
  gobject = (GObjectClass *) copy_audio_signal;

  gobject->finalize = ags_copy_audio_signal_finalize;

  /* AgsRecallClass */
  recall = (AgsRecallClass *) copy_audio_signal;

  recall->run_inter = ags_copy_audio_signal_run_inter;  
  recall->duplicate = ags_copy_audio_signal_duplicate;
}

void
ags_copy_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_copy_audio_signal_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_copy_audio_signal_connect;
  connectable->disconnect = ags_copy_audio_signal_disconnect;
}

void
ags_copy_audio_signal_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable)
{
  ags_copy_audio_signal_parent_run_connectable_interface = g_type_interface_peek_parent(run_connectable);

  run_connectable->connect = ags_copy_audio_signal_run_connect;
  run_connectable->disconnect = ags_copy_audio_signal_run_disconnect;
}

void
ags_copy_audio_signal_init(AgsCopyAudioSignal *copy_audio_signal)
{
  AGS_RECALL(copy_audio_signal)->child_type = G_TYPE_NONE;
}

void
ags_copy_audio_signal_connect(AgsConnectable *connectable)
{
  /* call parent */
  ags_copy_audio_signal_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_copy_audio_signal_disconnect(AgsConnectable *connectable)
{
  /* call parent */
  ags_copy_audio_signal_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_copy_audio_signal_run_connect(AgsRunConnectable *run_connectable)
{
  /* call parent */
  ags_copy_audio_signal_parent_run_connectable_interface->connect(run_connectable);

  /* empty */
}

void
ags_copy_audio_signal_run_disconnect(AgsRunConnectable *run_connectable)
{
  /* call parent */
  ags_copy_audio_signal_parent_run_connectable_interface->disconnect(run_connectable);

  /* empty */
}

void
ags_copy_audio_signal_finalize(GObject *gobject)
{
  /* empty */

  /* call parent */
  G_OBJECT_CLASS(ags_copy_audio_signal_parent_class)->finalize(gobject);
}

void
ags_copy_audio_signal_run_inter(AgsRecall *recall)
{
  AgsCopyAudioSignal *copy_audio_signal;
  AgsAudioSignal *source, *destination;
  AgsAttack *attack;
  GList *stream_source, *stream_destination;

  AGS_RECALL_CLASS(ags_copy_audio_signal_parent_class)->run_inter(recall);

  copy_audio_signal = AGS_COPY_AUDIO_SIGNAL(recall);

  source = AGS_RECALL_AUDIO_SIGNAL(copy_audio_signal)->source;
  stream_source = source->stream_current;

  if(stream_source == NULL){
    ags_recall_done(recall);
    return;
  }

  //FIXME:JK: attack probably needs to be removed
  destination = AGS_RECALL_AUDIO_SIGNAL(copy_audio_signal)->destination;
  stream_destination = destination->stream_current;
  attack = AGS_RECALL_AUDIO_SIGNAL(copy_audio_signal)->attack;

  if(stream_destination->next == NULL)
    ags_audio_signal_add_stream(destination);

  ags_audio_signal_copy_buffer_to_buffer((short *) stream_destination->data, 1,
					 (short *) stream_source->data, 1,
					 attack->first_length);

  //  if(attack->first_start != 0){
  //    ags_audio_signal_copy_buffer_to_buffer((short *) stream_destination->data, 1,
  //					   &(((short *) stream_source->data)[attack->first_length]), 1,
  //					   attack->first_start);
  //  }
}

AgsRecall*
ags_copy_audio_signal_duplicate(AgsRecall *recall,
				AgsRecallID *recall_id,
				guint *n_params, GParameter *parameter)
{
  AgsCopyAudioSignal *copy;

  copy = (AgsCopyAudioSignal *) AGS_RECALL_CLASS(ags_copy_audio_signal_parent_class)->duplicate(recall,
												recall_id,
												n_params, parameter);

  return((AgsRecall *) copy);
}

AgsCopyAudioSignal*
ags_copy_audio_signal_new(AgsAudioSignal *destination,
			  AgsAudioSignal *source,
			  AgsDevout *devout,
			  AgsAttack *attack)
{
  AgsCopyAudioSignal *copy_audio_signal;

  copy_audio_signal = (AgsCopyAudioSignal *) g_object_new(AGS_TYPE_COPY_AUDIO_SIGNAL,
							  "destination\0", destination,
							  "source\0", source,
							  "devout\0", devout,
							  "attack\0", attack,
							  NULL);

  return(copy_audio_signal);
}