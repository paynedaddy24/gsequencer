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

#include <math.h>

void ags_recycling_thread_class_init(AgsRecyclingThreadClass *recycling_thread);
void ags_recycling_thread_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_recycling_thread_init(AgsRecyclingThread *recycling_thread);
void ags_recycling_thread_connect(AgsConnectable *connectable);
void ags_recycling_thread_disconnect(AgsConnectable *connectable);
void ags_recycling_thread_finalize(GObject *gobject);

void ags_recycling_thread_run(AgsThread *thread);

static gpointer ags_recycling_thread_parent_class = NULL;
static AgsConnectableInterface *ags_recycling_thread_parent_connectable_interface;

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

  thread->run = ags_recycling_thread_run;
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
}

void
ags_recycling_thread_connect(AgsConnectable *connectable)
{
  ags_recycling_thread_parent_connectable_interface->connect(connectable);

  /* empty */
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
ags_recycling_thread_run(AgsThread *thread)
{
  //TODO:JK: implement me
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
