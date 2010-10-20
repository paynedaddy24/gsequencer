#ifndef __AGS_APPLY_SYNTH_H__
#define __AGS_APPLY_SYNTH_H__

#include <glib.h>
#include <glib-object.h>

#include "../ags_task.h"

#include "../ags_channel.h"

#define AGS_TYPE_APPLY_SYNTH                (ags_apply_synth_get_type())
#define AGS_APPLY_SYNTH(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_APPLY_SYNTH, AgsApplySynth))
#define AGS_APPLY_SYNTH_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_APPLY_SYNTH, AgsApplySynthClass))
#define AGS_IS_APPLY_SYNTH(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_APPLY_SYNTH))
#define AGS_IS_APPLY_SYNTH_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_APPLY_SYNTH))
#define AGS_APPLY_SYNTH_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_APPLY_SYNTH, AgsApplySynthClass))

typedef struct _AgsApplySynth AgsApplySynth;
typedef struct _AgsApplySynthClass AgsApplySynthClass;

typedef enum{
  AGS_APPLY_SYNTH_INVALID,
  AGS_APPLY_SYNTH_SIN,
  AGS_APPLY_SYNTH_SAW,
  AGS_APPLY_SYNTH_SQUARE,
  AGS_APPLY_SYNTH_TRIANGLE,
}AgsApplySynthWave;

struct _AgsApplySynth
{
  AgsTask task;

  AgsChannel *start_channel;
  guint count;

  guint wave;

  guint attack;
  guint frame_count;
  guint frequency;
  guint phase;
  guint start;

  gdouble volume;
};

struct _AgsApplySynthClass
{
  AgsTaskClass task;
};

GType ags_apply_synth_get_type();

AgsApplySynth* ags_apply_synth_new(AgsChannel *start_channel, guint count,
				   guint wave,
				   guint attack, guint frame_count,
				   guint frequency, guint phase, guint start,
				   gdouble volume);

#endif /*__AGS_APPLY_SYNTH_H__*/