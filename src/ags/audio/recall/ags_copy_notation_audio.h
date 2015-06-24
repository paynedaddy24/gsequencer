/* This file is part of GSequencer.
 * 
 * GSequencer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GSequencer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GSequencer.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __AGS_COPY_NOTATION_AUDIO_H__
#define __AGS_COPY_NOTATION_AUDIO_H__

#include <glib-object.h>

#include <ags/audio/ags_devout.h>
#include <ags/audio/ags_recall_audio.h>
#include <ags/audio/ags_notation.h>

#define AGS_TYPE_COPY_NOTATION_AUDIO                (ags_copy_notation_audio_get_type())
#define AGS_COPY_NOTATION_AUDIO(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_COPY_NOTATION_AUDIO, AgsCopyNotationAudio))
#define AGS_COPY_NOTATION_AUDIO_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_COPY_NOTATION_AUDIO, AgsCopyNotationAudio))
#define AGS_IS_COPY_NOTATION_AUDIO(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_COPY_NOTATION_AUDIO))
#define AGS_IS_COPY_NOTATION_AUDIO_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_COPY_NOTATION_AUDIO))
#define AGS_COPY_NOTATION_AUDIO_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_COPY_NOTATION_AUDIO, AgsCopyNotationAudioClass))

typedef struct _AgsCopyNotationAudio AgsCopyNotationAudio;
typedef struct _AgsCopyNotationAudioClass AgsCopyNotationAudioClass;

typedef enum{
  AGS_COPY_NOTATION_AUDIO_FIT_AUDIO_SIGNAL  = 1,
}AgsCopyNotationAudioFlags;

struct _AgsCopyNotationAudio
{
  AgsRecallAudio recall_audio;

  guint flags;

  AgsDevout *devout;

  AgsNotation *notation;
  guint audio_channel;
};

struct _AgsCopyNotationAudioClass
{
  AgsRecallAudioClass recall_audio;
};

GType ags_copy_notation_audio_get_type();

AgsCopyNotationAudio* ags_copy_notation_audio_new(AgsDevout *devout,
						  AgsNotation *notation,
						  guint audio_channel);

#endif /*__AGS_COPY_NOTATION_AUDIO_H__*/
