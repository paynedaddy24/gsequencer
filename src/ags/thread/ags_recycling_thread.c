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

#include <ags/thread/ags_recycling_thread.h>

#include <ags/object/ags_connectable.h>

#include <ags/audio/ags_devout.h>
#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_recall_id.h>

#include <math.h>

void ags_recycling_thread_class_init(AgsRecyclingThreadClass *recycling_thread);
void ags_recycling_thread_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_recycling_thread_init(AgsRecyclingThread *recycling_thread);
void ags_recycling_thread_connect(AgsConnectable *connectable);
void ags_recycling_thread_disconnect(AgsConnectable *connectable);
void ags_recycling_thread_finalize(GObject *gobject);

void ags_recycling_thread_start(AgsThread *thread);

void ags_recycling_thread_real_play(AgsRecyclingThread *recycling_thread,
				    GObject *current,
				    AgsRecycling *first_recycling, AgsRecycling *last_recycling,
				    AgsRecallID *recall_id,
				    gint stage, gboolean do_recall);

void ags_recycling_thread_fifo(AgsRecyclingThread *thread);

enum{
  PLAY,
  LAST_SIGNAL,
};

static gpointer ags_recycling_thread_parent_class = NULL;
static AgsConnectableInterface *ags_recycling_thread_parent_connectable_interface;

static guint recycling_thread_signals[LAST_SIGNAL];

GType
ags_recycling_thread_get_type()
{
  static GType ags_type_recycling_thread = 0;

  if(!ags_type_recycling_thread){
    static const GTypeInfo ags_recycling_thread_info = {
      sizeof (AgsRecyclingThreadClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_recycling_thread_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRecyclingThread),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_recycling_thread_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_recycling_thread_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    ags_type_recycling_thread = g_type_register_static(AGS_TYPE_THREAD,
						       "AgsRecyclingThread\0",
						       &ags_recycling_thread_info,
						    0);
    
    g_type_add_interface_static(ags_type_recycling_thread,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_recycling_thread);
}

void
ags_recycling_thread_class_init(AgsRecyclingThreadClass *recycling_thread)
{
  GObjectClass *gobject;
  AgsThreadClass *thread;

  ags_recycling_thread_parent_class = g_type_class_peek_parent(recycling_thread);

  /* GObject */
  gobject = (GObjectClass *) recycling_thread;

  gobject->finalize = ags_recycling_thread_finalize;

  /* AgsThread */
  thread = (AgsThreadClass *) recycling_thread;

  thread->start = ags_recycling_thread_start;

  /* AgsRecyclingThread */
  recycling_thread->play = ags_recycling_thread_play;

  /* signals */
  recycling_thread_signals[PLAY] = 
    g_signal_new("play\0",
		 G_TYPE_FROM_CLASS(recycling_thread),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsRecycling_ThreadClass, play),
		 NULL, NULL,
		 g_cclosure_user_marshal_VOID__OBJECT_OBJECT_OBJECT_OBJECT_INT_BOOLEAN,
		 G_TYPE_NONE, 6,
		 G_TYPE_OBJECT,
		 G_TYPE_OBJECT, G_TYPE_OBJECT,
		 G_TYPE_OBJECT,
		 G_TYPE_INT, G_TYPE_BOOLEAN);
}

void
ags_recycling_thread_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_recycling_thread_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_recycling_thread_connect;
  connectable->disconnect = ags_recycling_thread_disconnect;
}

void
ags_recycling_thread_init(AgsRecyclingThread *recycling_thread)
{
  recycling_thread->recycling = NULL;
  pthread_mutex_init(&(recycling_thread->iteration_mutex), NULL);
  pthread_cond_init(&(recycling_thread->iteration_cond), NULL);
}

void
ags_recycling_thread_connect(AgsConnectable *connectable)
{
  AgsThread *thread;

  thread = AGS_THREAD(connectable);

  ags_recycling_thread_parent_connectable_interface->connect(connectable);


}

void
ags_recycling_thread_disconnect(AgsConnectable *connectable)
{
  ags_recycling_thread_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_recycling_thread_finalize(GObject *gobject)
{
  AgsRecyclingThread *recycling_thread;

  recycling_thread = AGS_RECYCLING_THREAD(gobject);

  /*  */
  G_OBJECT_CLASS(ags_recycling_thread_parent_class)->finalize(gobject);
}

void
ags_recycling_thread_start(AgsThread *thread)
{
  AGS_THREAD_CLASS(ags_recycling_thread_parent_class)->start(thread);
}

void
ags_recycling_thread_real_play_channel(AgsRecyclingThread *recycling_thread,
				       GObject *channel,
				       AgsRecallID *recall_id,
				       gint stage, gboolean do_recall)
{
  while((AGS_RECYCLING_THREAD_RUNNING & (recycling_thread->flags)) != 0){
    ags_recycling_thread_fifo(recycling_thread);

    ags_channel_play(AGS_CHANNEL(channel),
		     recall_id,
		     stage, do_recall);
  }
}

void
ags_recycling_thread_play_channel(AgsRecyclingThread *recycling_thread,
				  GObject *channel,
				  AgsRecallID *recall_id,
				  gint stage, gboolean do_recall)
{
}

void
ags_recycling_thread_real_play_audio(AgsRecyclingThread *recycling_thread,
				     GObject *output, GObject *audio,
				     AgsRecycling *first_recycling, AgsRecycling *last_recycling,
				     AgsRecallID *recall_id, AgsGroupId next_group_id,
				     gint stage, gboolean do_recall)
{
  while((AGS_RECYCLING_THREAD_RUNNING & (recycling_thread->flags)) != 0){
    ags_recycling_thread_fifo(recycling_thread);

    if(next_group_id != recall_id->group_id){
      AgsRecycling *next_first_recycling, *next_last_recycling;
      
      next_first_recycling = ags_output_find_first_input_recycling(AGS_OUTPUT(output));
      next_last_recycling = ags_output_find_last_input_recycling(AGS_OUTPUT(output));
      
      ags_audio_play(audio,
		     next_first_recycling, next_last_recycling,
		     recall_id,
		     stage,
		     input_do_recall);
    }
    
    ags_audio_play(audio,
		   first_recycling, last_recycling,
		   recall_id,
		   stage, do_recall);    
  }
}

void
ags_recycling_thread_play(AgsRecyclingThread *recycling_thread,
			  GObject *output, GObject *audio,
			  AgsRecycling *first_recycling, AgsRecycling *last_recycling,
			  AgsRecallID *recall_id, AgsGroupId next_group_id,
			  gint stage, gboolean do_recall)
{
  g_return_if_fail(AGS_IS_RECYCLING_THREAD(recycling_thread));
  g_return_if_fail(AGS_IS_AUDIO(current) &&
		   AGS_IS_CHANNEL(output));

  g_object_ref((GObject *) recycling_thread);
  g_signal_emit(G_OBJECT(recycling_thread),
		recycling_thread_signals[PLAY], 0,
		output, audio,
		first_recycling, last_recycling,
		recall_id, next_group_id,
		stage, do_recall);
  g_object_unref((GObject *) recycling_thread);
}

void
ags_recycling_thread_fifo(AgsRecyclingThread *recycling_thread)
{
  pthread_mutex_lock(&(recycling_thread->iteration_mutex));

  recycling_thread->flags |= AGS_RECYCLING_THREAD_WAIT;

  while((AGS_RECYCLING_THREAD_WAIT & (recycling_thread->flags)) != 0 &&
	(AGS_RECYCLING_THREAD_DONE & (recycling_thread->flags)) == 0){
    pthread_cond_wait(&(recycling_thread->iteration_cond),
		      &(recycling_thread->iteration_mutex));
  }

  recycling_thread->flags &= (~AGS_RECYCLING_THREAD_WAIT);

  pthread_mutex_unlock(&(recycling_thread->iteration_mutex));
}

AgsRecyclingThread*
ags_recycling_thread_new(GObject *recycling)
{
  AgsRecyclingThread *recycling_thread;
  
  recycling_thread = (AgsRecyclingThread *) g_object_new(AGS_TYPE_RECYCLING_THREAD,
							 NULL);

  recycling_thread->recycling = recycling;

  return(recycling_thread);
}
