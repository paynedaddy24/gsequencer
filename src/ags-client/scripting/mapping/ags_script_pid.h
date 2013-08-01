/* AGS Client - Advanced GTK Sequencer Client
 * Copyright (C) 2013 Joël Krähemann
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

#ifndef __AGS_SCRIPT_PID_H__
#define __AGS_SCRIPT_PID_H__

#include <glib.h>
#include <glib-object.h>

#define AGS_TYPE_SCRIPT_PID                (ags_script_pid_get_type())
#define AGS_SCRIPT_PID(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SCRIPT_PID, AgsScriptPid))
#define AGS_SCRIPT_PID_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_SCRIPT_PID, AgsScriptPidClass))
#define AGS_IS_SCRIPT_PID(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_SCRIPT_PID))
#define AGS_IS_SCRIPT_PID_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_SCRIPT_PID))
#define AGS_SCRIPT_PID_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_SCRIPT_PID, AgsScriptPidClass))

typedef struct _AgsScriptPid AgsScriptPid;
typedef struct _AgsScriptPidClass AgsScriptPidClass;

struct _AgsScriptPid
{
  GObject object;
};

struct _AgsScriptPidClass
{
  GObjectClass object;
};

GType ags_script_pid_get_type();

AgsScriptPid* ags_script_pid_new();

#endif /*__AGS_SCRIPT_PID_H__*/
