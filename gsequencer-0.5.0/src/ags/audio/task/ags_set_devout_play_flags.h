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

#ifndef __AGS_SET_DEVOUT_PLAY_FLAGS_H__
#define __AGS_SET_DEVOUT_PLAY_FLAGS_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/audio/ags_task.h>
#include <ags/audio/ags_devout.h>

#define AGS_TYPE_SET_DEVOUT_PLAY_FLAGS                (ags_set_devout_play_flags_get_type())
#define AGS_SET_DEVOUT_PLAY_FLAGS(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SET_DEVOUT_PLAY_FLAGS, AgsSetDevoutPlayFlags))
#define AGS_SET_DEVOUT_PLAY_FLAGS_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_SET_DEVOUT_PLAY_FLAGS, AgsSetDevoutPlayFlagsClass))
#define AGS_IS_SET_DEVOUT_PLAY_FLAGS(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_SET_DEVOUT_PLAY_FLAGS))
#define AGS_IS_SET_DEVOUT_PLAY_FLAGS_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_SET_DEVOUT_PLAY_FLAGS))
#define AGS_SET_DEVOUT_PLAY_FLAGS_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_SET_DEVOUT_PLAY_FLAGS, AgsSetDevoutPlayFlagsClass))

typedef struct _AgsSetDevoutPlayFlags AgsSetDevoutPlayFlags;
typedef struct _AgsSetDevoutPlayFlagsClass AgsSetDevoutPlayFlagsClass;

struct _AgsSetDevoutPlayFlags
{
  AgsTask task;

  AgsDevoutPlay *devout_play;
  guint devout_play_flags;
};

struct _AgsSetDevoutPlayFlagsClass
{
  AgsTaskClass task;
};

GType ags_set_devout_play_flags_get_type();

AgsSetDevoutPlayFlags* ags_set_devout_play_flags_new(AgsDevoutPlay *play,
						     guint devout_play_flags);

#endif /*__AGS_SET_DEVOUT_PLAY_FLAGS_H__*/
