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

#ifndef __AGS_DEVOUT_H__
#define __AGS_DEVOUT_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#define __USE_GNU
#define __USE_UNIX98
#include <pthread.h>

#include <ao/ao.h>

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_task.h>

#define AGS_TYPE_DEVOUT                (ags_devout_get_type())
#define AGS_DEVOUT(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_DEVOUT, AgsDevout))
#define AGS_DEVOUT_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_DEVOUT, AgsDevout))
#define AGS_IS_DEVOUT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_DEVOUT))
#define AGS_IS_DEVOUT_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_DEVOUT))
#define AGS_DEVOUT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_DEVOUT, AgsDevoutClass))

#define AGS_DEVOUT_PLAY(ptr)           ((AgsDevoutPlay *)(ptr))

typedef struct _AgsDevout AgsDevout;
typedef struct _AgsDevoutClass AgsDevoutClass;
typedef struct _AgsDevoutPlay AgsDevoutPlay;
typedef struct _AgsDevoutAppend AgsDevoutAppend;

typedef enum
{
  AGS_DEVOUT_BUFFER0                        = 1,
  AGS_DEVOUT_BUFFER1                        = 1 << 1,
  AGS_DEVOUT_BUFFER2                        = 1 << 2,
  AGS_DEVOUT_BUFFER3                        = 1 << 3,

  AGS_DEVOUT_ATTACK_FIRST                   = 1 << 4,

  AGS_DEVOUT_PLAY                           = 1 << 5,

  AGS_DEVOUT_WAIT_DEVICE                    = 1 << 6,
  AGS_DEVOUT_WAIT_RECALL                    = 1 << 7,
  AGS_DEVOUT_WAIT_PLAY_FUNCTIONS            = 1 << 8,
  AGS_DEVOUT_WAIT_TASK                      = 1 << 9,
  AGS_DEVOUT_WAIT_APPEND_TASK               = 1 << 10,

  AGS_DEVOUT_LIBAO                          = 1 << 11,
  AGS_DEVOUT_OSS                            = 1 << 12,
  AGS_DEVOUT_ALSA                           = 1 << 13,

  AGS_DEVOUT_PLAY_RECALL                    = 1 << 14,
  AGS_DEVOUT_PLAYING_RECALL                 = 1 << 15,
  AGS_DEVOUT_PLAY_RECALL_TERMINATING        = 1 << 16,
  AGS_DEVOUT_PLAY_CHANNEL                   = 1 << 17,
  AGS_DEVOUT_PLAYING_CHANNEL                = 1 << 18,
  AGS_DEVOUT_PLAY_CHANNEL_TERMINATING       = 1 << 19,
  AGS_DEVOUT_PLAY_AUDIO                     = 1 << 20,
  AGS_DEVOUT_PLAYING_AUDIO                  = 1 << 21,
  AGS_DEVOUT_PLAY_AUDIO_TERMINATING         = 1 << 22,
  AGS_DEVOUT_PLAY_NOTE                      = 1 << 23,

  AGS_DEVOUT_SHUTDOWN                       = 1 << 24,
}AgsDevoutFlags;

typedef enum
{
  AGS_DEVOUT_PLAY_DONE              = 1,
  AGS_DEVOUT_PLAY_REMOVE            = 1 <<  1,
  AGS_DEVOUT_PLAY_PAD               = 1 <<  2,
  AGS_DEVOUT_PLAY_PLAYBACK          = 1 <<  3,
  AGS_DEVOUT_PLAY_SEQUENCER         = 1 <<  4,
  AGS_DEVOUT_PLAY_NOTATION          = 1 <<  5,
}AgsDevoutPlayFlags;

struct _AgsDevout
{
  GObject object;

  guint32 flags;

  guint dsp_channels;
  guint pcm_channels;
  guint bits;
  guint buffer_size;
  guint frequency; // sample_rate

  //  guint64 offset; // for timed tasks in AgsChannel

  //  guint note_delay;
  //  guint note_counter;
  //  guint note_offset; // corresponds to AgsNote->x

  short** buffer;

  double bpm; // beats per minute
  guint delay; // delay between tic change
  guint delay_counter; // next time attack changeing when delay_counter == delay
  AgsAttack *attack; // where currently tic resides in the stream's offset, measured in 1/64 of bpm

  union{
    struct _AgsAO{
      ao_device *device;
      ao_sample_format format;
      int driver_ao;
    }ao;
    struct _AgsOss{
      int device_fd;
      char *device;
    }oss;
    struct _AgsAlsa{
      int rc;
      snd_pcm_t *handle;
      snd_pcm_hw_params_t *params;
    }alsa;
  }out;

  pthread_t supervisor_thread;
  pthread_attr_t supervisor_thread_attr;
  pthread_mutex_t supervisor_mutex;
  pthread_mutexattr_t supervisor_mutex_attr;
  pthread_cond_t supervisor_wait_cond;
  guint wait_sync;

  pthread_t play_interceptor;
  gboolean play_suspend;
  pthread_cond_t play_interceptor_cond;
  pthread_t play_thread;
  pthread_attr_t play_thread_attr;
  pthread_mutex_t play_mutex;
  pthread_mutexattr_t play_mutex_attr;
  pthread_cond_t play_wait_cond;

  pthread_t play_functions_interceptor;
  gboolean play_functions_suspend;
  pthread_cond_t play_functions_interceptor_cond;
  pthread_t play_functions_thread;
  pthread_attr_t play_functions_thread_attr;
  pthread_mutex_t play_functions_mutex;
  pthread_mutexattr_t play_functions_mutex_attr;
  pthread_cond_t play_functions_wait_cond;

  pthread_t task_interceptor;
  gboolean task_suspend;
  pthread_cond_t task_interceptor_cond;
  pthread_t task_thread;
  pthread_attr_t task_thread_attr;
  pthread_mutex_t task_mutex;
  pthread_mutexattr_t task_mutex_attr;
  pthread_cond_t task_wait_cond;

  pthread_t append_task_interceptor;
  gboolean append_task_suspend;
  pthread_cond_t append_task_interceptor_cond;
  pthread_mutex_t append_task_mutex;
  pthread_cond_t append_task_wait_cond;

  guint tasks_queued;
  guint tasks_pending;
  GList *task;
  GList *audio;

  guint play_recall_ref;
  GList *play_recall; // play AgsRecall

  guint play_channel_ref;
  GList *play_channel; // play AgsChannel

  guint play_audio_ref;
  GList *play_audio; // play AgsAudio
};

struct _AgsDevoutClass
{
  GObjectClass object;

  void (*run)(AgsDevout *devout);
  void (*stop)(AgsDevout *devout);
  void (*tic)(AgsDevout *devout);

  void (*note_offset_changed)(AgsDevout *devout, guint note_offset);
};

struct _AgsDevoutPlay
{
  guint flags;

  GObject *source;
  guint audio_channel;

  AgsGroupId group_id; // if source is an AgsChannel or an AgsAudio
  AgsRecallID *recall_id; // if source is an AgsRecall
};

struct _AgsDevoutAppend
{
  AgsDevout *devout;
  gpointer data;
};

GType ags_devout_get_type();

AgsDevoutPlay* ags_devout_play_alloc();

void ags_devout_add_audio(AgsDevout *devout, GObject *audio);
void ags_devout_remove_audio(AgsDevout *devout, GObject *audio);

void* ags_devout_supervisor_thread(void *ptr);
void* ags_devout_task_thread(void *devout);

void ags_devout_append_task(AgsDevout *devout, AgsTask *task);
void ags_devout_append_tasks(AgsDevout *devout, GList *list);

void ags_devout_run(AgsDevout *devout);
void ags_devout_stop(AgsDevout *devout);
void ags_devout_tic(AgsDevout *devout);

void ags_devout_note_offset_changed(AgsDevout *devout, guint note_offset);

AgsDevout* ags_devout_new();

#endif /*__AGS_DEVOUT_H__*/
