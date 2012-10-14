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

#ifndef __AGS_DELAY_AUDIO_H__
#define __AGS_DELAY_AUDIO_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/audio/ags_recall_audio.h>

#include <ags/object/ags_tactable.h>

#define AGS_TYPE_DELAY_AUDIO                (ags_delay_audio_get_type())
#define AGS_DELAY_AUDIO(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_DELAY_AUDIO, AgsDelayAudio))
#define AGS_DELAY_AUDIO_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_DELAY_AUDIO, AgsDelayAudio))
#define AGS_IS_DELAY_AUDIO(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_DELAY_AUDIO))
#define AGS_IS_DELAY_AUDIO_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_DELAY_AUDIO))
#define AGS_DELAY_AUDIO_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_DELAY_AUDIO, AgsDelayAudioClass))

typedef struct _AgsDelayAudio AgsDelayAudio;
typedef struct _AgsDelayAudioClass AgsDelayAudioClass;

struct _AgsDelayAudio
{
  AgsRecallAudio recall_audio;

  gdouble bpm;
  gdouble tact;
  gdouble duration;

  guint frames;
  guint notation_delay;
  guint sequencer_delay;

  AgsTactable *tactable;
  
  gulong change_bpm_handle;
  gulong change_tact_handle;
  gulong change_duration_handle;
};

struct _AgsDelayAudioClass
{
  AgsRecallAudioClass recall_audio;
};

GType ags_delay_audio_get_type();

AgsDelayAudio* ags_delay_audio_new(AgsTactable *tactable);

#endif /*__AGS_DELAY_AUDIO_H__*/