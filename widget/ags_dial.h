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

#ifndef __AGS_DIAL_H__
#define __AGS_DIAL_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#define AGS_TYPE_DIAL                (ags_dial_get_type())
#define AGS_DIAL(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_DIAL, AgsDial))
#define AGS_DIAL_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_DIAL, AgsDialClass))
#define AGS_IS_DIAL(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_DIAL))
#define AGS_IS_DIAL_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_DIAL))
#define AGS_DIAL_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_DIAL, AgsDialClass))

typedef struct _AgsDial AgsDial;
typedef struct _AgsDialClass AgsDialClass;

struct _AgsDial
{
  GtkRange range;
};

struct _AgsDialClass
{
  GtkRangeClass range;
};

GType ags_dial_get_type(void);

AgsDial* ags_dial_new();

#endif /*__AGS_DIAL_H__*/