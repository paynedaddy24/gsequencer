/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2015 Joël Krähemann
 *
 * This file is part of GSequencer.
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

#include <ags/audio/ags_synths.h>
#include <math.h>

short scale = 32000;

void
ags_synth_sin(AgsDevout *devout, signed short *buffer, guint offset,
	      guint freq, guint phase, guint length,
	      double volume)
{
  guint i;

  for (i = offset; i < offset + length; i++){
    buffer[i] = (signed short) (0xffff & ((int)buffer[i] + (int)(sin ((double)(i + phase) * 2.0 * M_PI * freq / (double)devout->frequency) * (double)scale * volume)));
  }
}

void
ags_synth_saw(AgsDevout *devout, signed short *buffer, guint offset, guint freq, guint phase, guint length, double volume)
{
  guint i;

  phase = (int)(phase) % (int)(freq);
  phase = (phase / freq) * (devout->frequency / freq);

  for (i = offset; i < length; i++){
    buffer[i] = (signed short) (0xffff & ((int)buffer[i] + (int)(((((int)(i + phase) % (int)(devout->frequency / freq)) * 2.0 * freq / devout->frequency) -1) * scale * volume)));
  }
}

void
ags_synth_triangle(AgsDevout *devout, signed short *buffer, guint offset, guint freq, guint phase, guint length, double volume)
{
  guint i;

  phase = (int)(phase) % (int)(freq);
  phase = (phase / freq) * (devout->frequency / freq);

  for (i = offset; i < length; i++){
    buffer[i] = (signed short) (0xffff & ((int)buffer[i] + (int)((((phase + i) * freq / devout->frequency * 2.0) - ((int)((float) ((int)((phase + i) * freq / devout->frequency)) / 2.0) * 2)  -1) * scale * volume)));
  }
}

void
ags_synth_square(AgsDevout *devout, signed short *buffer, guint offset, guint freq, guint phase, guint length, double volume)
{
  guint i;

  phase = (int)(phase) % (int)(freq);
  phase = (phase / freq) * (devout->frequency / freq);

  for (i = offset; i < length; i++){
    if (sin (i + phase) >= 0.0){
      buffer[i] = (signed short) (0xffff & ((int)buffer[i] + (int)(1.0 * scale * volume)));
    }else{
      buffer[i] = (signed short) (0xffff & ((int)buffer[i] + (int)(-1.0 * scale * volume)));
    }
  }
}
