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

#include <ags/thread/ags_thread.h>

#include <ags/object/ags_connectable.h>

#include <ags/audio/ags_devout.h>

void ags_thread_class_init(AgsThreadClass *thread);
void ags_thread_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_thread_init(AgsThread *thread);
void ags_thread_set_property(GObject *gobject,
			     guint prop_id,
			     const GValue *value,
			     GParamSpec *param_spec);
void ags_thread_get_property(GObject *gobject,
			     guint prop_id,
			     GValue *value,
			     GParamSpec *param_spec);
void ags_thread_connect(AgsConnectable *connectable);
void ags_thread_disconnect(AgsConnectable *connectable);
void ags_thread_finalize(GObject *gobject);

void ags_thread_set_devout(AgsThread *thread, GObject *devout);

void ags_thread_real_start(AgsThread *thread);
void* ags_thread_loop(void *ptr);
void ags_thread_real_run(AgsThread *thread);
void ags_thread_real_stop(AgsThread *thread);

enum{
  PROP_0,
  PROP_DEVOUT,
};

enum{
  START,
  RUN,
  STOP,
  LAST_SIGNAL,
};

static gpointer ags_thread_parent_class = NULL;
static guint thread_signals[LAST_SIGNAL];

GType
ags_thread_get_type()
{
  static GType ags_type_thread = 0;

  if(!ags_type_thread){
    static const GTypeInfo ags_thread_info = {
      sizeof (AgsThreadClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_thread_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsThread),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_thread_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_thread_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_thread = g_type_register_static(G_TYPE_OBJECT,
					     "AgsThread\0",
					     &ags_thread_info,
					     0);
    
    g_type_add_interface_static(ags_type_thread,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_thread);
}

void
ags_thread_class_init(AgsThreadClass *thread)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_thread_parent_class = g_type_class_peek_parent(thread);

  /* GObject */
  gobject = (GObjectClass *) thread;

  gobject->set_property = ags_thread_set_property;
  gobject->get_property = ags_thread_get_property;

  gobject->finalize = ags_thread_finalize;

  /* properties */
  param_spec = g_param_spec_object("devout\0",
				   "devout assigned to\0",
				   "The AgsDevout it is assigned to.\0",
				   AGS_TYPE_DEVOUT,
				   G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DEVOUT,
				  param_spec);

  /* AgsThread */
  thread->start = ags_thread_real_start;
  thread->run = NULL;
  thread->stop = ags_thread_real_stop;

  /* signals */
  thread_signals[START] =
    g_signal_new("start\0",
		 G_TYPE_FROM_CLASS (thread),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsThreadClass, start),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  thread_signals[RUN] =
    g_signal_new("run\0",
		 G_TYPE_FROM_CLASS (thread),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsThreadClass, run),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  thread_signals[STOP] =
    g_signal_new("stop\0",
		 G_TYPE_FROM_CLASS (thread),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsThreadClass, stop),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);
}

void
ags_thread_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->connect = ags_thread_connect;
  connectable->disconnect = ags_thread_disconnect;
}

void
ags_thread_init(AgsThread *thread)
{
  thread->flags = 0;

  pthread_mutex_init(&(thread->mutex), NULL);
  pthread_cond_init(&(thread->cond), NULL);
  thread->unlocked = NULL;

  thread->first_barrier = TRUE;
  thread->wait_count[0] = 1;
  thread->wait_count[1] = 1;

  thread->devout = NULL;

  thread->parent = NULL;
  thread->next = NULL;
  thread->prev = NULL;
  thread->children = NULL;

  thread->data = NULL;
}

void
ags_thread_set_property(GObject *gobject,
			guint prop_id,
			const GValue *value,
			GParamSpec *param_spec)
{
  AgsThread *thread;

  thread = AGS_THREAD(gobject);

  switch(prop_id){
  case PROP_DEVOUT:
    {
      AgsDevout *devout;

      devout = (AgsDevout *) g_value_get_object(value);

      if(thread->devout != NULL){
	g_object_unref(G_OBJECT(thread->devout));
      }

      if(devout != NULL){
	g_object_ref(G_OBJECT(devout));
      }

      thread->devout = devout;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_thread_get_property(GObject *gobject,
			guint prop_id,
			GValue *value,
			GParamSpec *param_spec)
{
  AgsThread *thread;

  thread = AGS_THREAD(gobject);

  switch(prop_id){
  case PROP_DEVOUT:
    {
      g_value_set_object(value, G_OBJECT(thread->devout));
    }
    break;
  }
}

void
ags_thread_connect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_thread_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_thread_finalize(GObject *gobject)
{
  AgsThread *thread;

  thread = AGS_THREAD(gobject);

  G_OBJECT_CLASS(ags_thread_parent_class)->finalize(gobject);

  /* empty */
  pthread_mutex_destroy(&(thread->mutex));
  pthread_cond_destroy(&(thread->cond));

}

void
ags_thread_set_devout(AgsThread *thread, GObject *devout)
{
}

void
ags_thread_lock(AgsThread *thread)
{
  pthread_mutex_t mutex;
  
  mutex = ags_thread_get_toplevel(thread)->mutex;

  pthread_mutex_lock(&mutex);

  pthread_mutex_lock(&(thread->mutex));
  thread->flags |= AGS_THREAD_LOCKED;

  pthread_mutex_unlock(&mutex);
}

void
ags_thread_unlock(AgsThread *thread)
{
  pthread_mutex_t mutex;
  
  mutex = ags_thread_get_toplevel(thread)->mutex;

  pthread_mutex_lock(&mutex);
  thread->flags &= (~AGS_THREAD_LOCKED);
  pthread_mutex_unlock(&(thread->mutex));

  pthread_mutex_unlock(&mutex);
}

/**
 * ags_thread_get_toplevel:
 * @thread
 *
 * Retrieve toplevel thread.
 */
AgsThread*
ags_thread_get_toplevel(AgsThread *thread)
{
  if(thread == NULL){
    return(NULL);
  }

  while(thread->parent != NULL){
    thread = thread->parent;
  }

  return(thread);
}

/**
 * ags_thread_first:
 * @thread
 *
 * Retrieve first sibling.
 */
AgsThread*
ags_thread_first(AgsThread *thread)
{
  if(thread == NULL){
    return(NULL);
  }

  while(thread->prev != NULL){
    thread = thread->prev;
  }

  return(thread);
}

/**
 * ags_thread_parental_is_locked:
 * @thread
 *
 * Check the AGS_THREAD_LOCKED flag in parental levels.
 */
gboolean
ags_thread_parental_is_locked(AgsThread *thread)
{
  AgsThread *current;

  if(thread == NULL){
    return(FALSE);
  }

  current = thread->parent;

  while(current != NULL){
    if((AGS_THREAD_LOCKED & (current->flags)) != 0){
      return(TRUE);
    }

    current = current->parent;
  }

  return(FALSE);
}

gboolean
ags_thread_sibling_is_locked(AgsThread *thread)
{
  if(thread == NULL){
    return(FALSE);
  }

  thread = ags_thread_first(thread);

  while(thread->next != NULL){
    if((AGS_THREAD_LOCKED & (thread->flags)) != 0){
      return(TRUE);
    }

    thread = thread->next;
  }

  return(FALSE);
}

gboolean
ags_thread_children_is_locked(AgsThread *thread)
{
  auto gboolean ags_thread_children_is_locked_recursive(AgsThread *thread);

  gboolean ags_thread_children_is_locked_recursive(AgsThread *thread){
    AgsThread *current;

    if(thread == NULL){
      return(FALSE);
    }

    current = thread;

    while(current != NULL){
      if((AGS_THREAD_LOCKED & (thread->flags)) != 0){
	return(TRUE);
      }

      if(ags_thread_children_is_locked_recursive(current->children)){
	return(TRUE);
      }

      current = current->next;
    }
  }

  if(thread == NULL){
    return(FALSE);
  }

  return(ags_thread_children_is_locked_recursive(thread->children));
}

gboolean
ags_thread_parental_is_unlocked(AgsThread *thread, pthread_mutex_t lock)
{
  AgsThread *current;

  if(thread == NULL){
    return(TRUE);
  }

  current = thread->parent;

  while(current != NULL){
    if(g_list_find(current->unlocked, lock) == NULL){
      return(FALSE);
    }
  }

  return(TRUE);
}

gboolean
ags_thread_sibling_is_unlocked(AgsThread *thread, pthread_mutex_t lock)
{
  AgsThread *current;

  if(thread == NULL){
    return(TRUE);
  }
 
  current = ags_thread_first(thread);

  while(current != NULL){
    if(g_list_find(current->unlocked, lock) == NULL){
      return(FALSE);
    }

    current = current->next;
  }

  return(TRUE);
}

gboolean
ags_thread_children_is_unlocked(AgsThread *thread, pthread_mutex_t lock)
{
  auto void ags_thread_children_is_unlocked_recursive(AgsThread *thread, pthread_mutex_t lock);

  void ags_thread_children_is_unlocked_recursive(AgsThread *thread, pthread_mutex_t lock){
    AgsThread *current;

    if(thread == NULL){
      return(TRUE);
    }
    
    while(current != NULL){
      if(g_list_find(current->unlocked, lock) == NULL){
	return(FALSE);
      }

      if(!ags_thread_children_is_unlocked(current->children, lock)){
	return(FALSE);
      }

      current = current->next;
    }

    return(TRUE);
  }

  return(ags_thread_children_is_unlocked_recursive(thread->children, lock));
}

void
ags_thread_signal_parent(AgsThread *thread, gboolean broadcast)
{
  AgsThread *current;

  current = thread->parent;

  while(current != NULL){
    if((AGS_THREAD_WAIT_FOR_CHILDREN & (current->flags)) != 0){
      if(!broadcast){
	pthread_cond_signal(&(current->cond));
      }else{
	pthread_cond_broadcast(&(current->cond));
      }
    }

    current = current->parent;
  }
}

void
ags_thread_signal_sibling(AgsThread *thread, gboolean broadcast)
{
  AgsThread *current;

  current = ags_thread_first(thread);

  while(current != NULL){
    if((AGS_THREAD_WAIT_FOR_SIBLING & (current->flags)) != 0){
      if(!broadcast){
	pthread_cond_signal(&(current->cond));
      }else{
	pthread_cond_broadcast(&(current->cond));
      }
    }
  }
}

void
ags_thread_signal_children(AgsThread *thread, gboolean broadcast)
{
  auto void ags_thread_signal_children_recursive(AgsThread *thread, gboolean broadcast);

  void ags_thread_signal_children_recursive(AgsThread *thread, gboolean broadcast){
    AgsThread *current;

    if(thread == NULL){
      return;
    }

    current = thread;

    while(current != NULL){
      if((AGS_THREAD_WAIT_FOR_PARENT & (current->flags)) != 0){
	if(!broadcast){
	  pthread_cond_signal(&(current->cond));
	}else{
	  pthread_cond_broadcast(&(current->cond));
	}
      }
      
      ags_thread_signal_children_recursive(current, broadcast);

      current = current->next;
    }
  }

  ags_thread_signal_children(thread->children, broadcast);
}

void
ags_thread_real_start(AgsThread *thread)
{
  pthread_create(&(thread->thread), NULL,
		 &ags_thread_loop, thread);
}

void
ags_thread_start(AgsThread *thread)
{
  g_return_if_fail(AGS_IS_THREAD(thread));

  g_object_ref(G_OBJECT(thread));
  g_signal_emit(G_OBJECT(thread),
		thread_signals[START], 0);
  g_object_unref(G_OBJECT(thread));
}

void*
ags_thread_loop(void *ptr)
{
  AgsThread *thread;
  pthread_mutex_t mutex;
  int wait_count;
  gboolean initial_run;

  thread = AGS_THREAD(ptr);

  initial_run = TRUE;

  while((AGS_THREAD_RUNNING & (thread->flags)) != 0){
    mutex = ags_thread_get_toplevel(thread)->mutex;

    /*  */
    pthread_mutex_lock(&mutex);

    /* parent */
    if((AGS_THREAD_WAIT_FOR_PARENT & (thread->flags)) != 0 &&
       ags_thread_parental_is_locked(thread)){

      thread->flags |= AGS_THREAD_WAITING_FOR_PARENT; 

      while(ags_thread_parental_is_locked(thread)){
	pthread_cond_wait(&(thread->cond),
			  &mutex);
      }

      thread->flags &= (~AGS_THREAD_WAITING_FOR_PARENT);

      if((AGS_THREAD_BROADCAST_PARENT & (thread->flags)) == 0){
	pthread_cond_signal(&(thread->cond));
      }else{
	pthread_cond_broadcast(&(thread->cond));
      }
    }

    /* sibling */
    if((AGS_THREAD_WAIT_FOR_SIBLING & (thread->flags)) != 0 &&
       ags_thread_sibling_is_locked(thread)){

      thread->flags |= AGS_THREAD_WAITING_FOR_SIBLING;

      while(ags_thread_sibling_is_locked(thread)){
	pthread_cond_wait(&(thread->cond),
			  &mutex);
      }

      thread->flags &= (~AGS_THREAD_WAITING_FOR_SIBLING);

      if((AGS_THREAD_BROADCAST_SIBLING & (thread->flags)) == 0){
	pthread_cond_signal(&(thread->cond));
      }else{
	pthread_cond_broadcast(&(thread->cond));
      }
    }

    /* children */
    if((AGS_THREAD_WAIT_FOR_CHILDREN & (thread->flags)) != 0 &&
       ags_thread_children_is_locked(thread)){

      thread->flags |= AGS_THREAD_WAITING_FOR_CHILDREN;

      while(ags_thread_children_is_locked(thread)){
	pthread_cond_wait(&(thread->cond),
			  &mutex);
      }

      thread->flags &= (~AGS_THREAD_WAITING_FOR_CHILDREN);

      if((AGS_THREAD_BROADCAST_CHILDREN & (thread->flags)) == 0){
	pthread_cond_signal(&(thread->cond));
      }else{
	pthread_cond_broadcast(&(thread->cond));
      }
    }

    pthread_mutex_unlock(&mutex);

    /* barrier */
    if((AGS_THREAD_WAITING_FOR_BARRIER & (thread->flags)) != 0){
      if(thread->first_barrier){
	/* retrieve wait count */
	pthread_mutex_lock(&mutex);

	wait_count = thread->wait_count[0];

	pthread_mutex_unlock(&mutex);

	/* init and wait */
	pthread_barrier_init(&(thread->barrier[0]), NULL, wait_count);
	pthread_barrier_wait(&(thread->barrier[0]));
      }else{
	/* retrieve wait count */
	pthread_mutex_lock(&mutex);

	wait_count = thread->wait_count[1];

	pthread_mutex_unlock(&mutex);

	/* init and wait */
	pthread_barrier_init(&(thread->barrier[1]), NULL, wait_count);
	pthread_barrier_wait(&(thread->barrier[1]));
      }
    }

    /* set idle flag */
    pthread_mutex_lock(&mutex);

    thread->flags |= AGS_THREAD_IDLE;

    pthread_mutex_unlock(&mutex);

    /* run */
    ags_thread_run(thread);

    /* unset idle flag */
    pthread_mutex_lock(&mutex);

    thread->flags &= (~AGS_THREAD_IDLE);

    pthread_mutex_unlock(&mutex);

    /* unset initial run */
    if(initial_run){
      initial_run = FALSE;
    }
  }

  pthread_exit(NULL);
}

void
ags_thread_run(AgsThread *thread)
{
  g_return_if_fail(AGS_IS_THREAD(thread));

  g_object_ref(G_OBJECT(thread));
  g_signal_emit(G_OBJECT(thread),
		thread_signals[RUN], 0);
  g_object_unref(G_OBJECT(thread));
}

void
ags_thread_real_stop(AgsThread *thread)
{
  //TODO:JK: implement me
}

void
ags_thread_stop(AgsThread *thread)
{
  g_return_if_fail(AGS_IS_THREAD(thread));

  g_object_ref(G_OBJECT(thread));
  g_signal_emit(G_OBJECT(thread),
		thread_signals[STOP], 0);
  g_object_unref(G_OBJECT(thread));
}

AgsThread*
ags_thread_new(GObject *data)
{
  AgsThread *thread;

  thread = (AgsThread *) g_object_new(AGS_TYPE_THREAD,
				      NULL);

  thread->data = data;

  return(thread);
}
