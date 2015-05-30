/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2014 Joël Krähemann
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

#ifndef __AGS_AUTOMATION_EDIT_H__
#define __AGS_AUTOMATION_EDIT_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <ags/widget/ags_ruler.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_automation.h>

#define AGS_TYPE_AUTOMATION_EDIT                (ags_automation_edit_get_type())
#define AGS_AUTOMATION_EDIT(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_AUTOMATION_EDIT, AgsAutomationEdit))
#define AGS_AUTOMATION_EDIT_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_AUTOMATION_EDIT, AgsAutomationEditClass))
#define AGS_IS_AUTOMATION_EDIT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_AUTOMATION_EDIT))
#define AGS_IS_AUTOMATION_EDIT_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_AUTOMATION_EDIT))
#define AGS_AUTOMATION_EDIT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS (obj, AGS_TYPE_AUTOMATION_EDIT, AgsAutomationEditClass))

#define AGS_AUTOMATION_EDIT_MAX_CONTROLS 1200

typedef struct _AgsAutomationEdit AgsAutomationEdit;
typedef struct _AgsAutomationEditClass AgsAutomationEditClass;

typedef enum{
  AGS_AUTOMATION_EDIT_RESETING_VERTICALLY    = 1,
  AGS_AUTOMATION_EDIT_RESETING_HORIZONTALLY  = 1 <<  1,
  AGS_AUTOMATION_EDIT_POSITION_CURSOR        = 1 <<  2,
  AGS_AUTOMATION_EDIT_ADDING_AUTOMATION         = 1 <<  3,
  AGS_AUTOMATION_EDIT_DELETING_AUTOMATION       = 1 <<  4,
  AGS_AUTOMATION_EDIT_SELECTING_AUTOMATIONS     = 1 <<  5,
}AgsAutomationEditFlags;

typedef enum{
  AGS_AUTOMATION_EDIT_RESET_VSCROLLBAR   = 1,
  AGS_AUTOMATION_EDIT_RESET_HSCROLLBAR   = 1 <<  1,
  AGS_AUTOMATION_EDIT_RESET_WIDTH        = 1 <<  2,
  AGS_AUTOMATION_EDIT_RESET_HEIGHT       = 1 <<  3, // reserved
}AgsAutomationEditResetFlags;

struct _AgsAutomationEdit
{
  GtkTable table;

  guint flags;

  AgsRuler *ruler;

  guint map_width;
  guint map_height;

  GtkScrolledWindow *scrolled_window;
  GtkVBox *automation_area;
};

struct _AgsAutomationEditClass
{
  GtkTableClass table;
};

GType ags_automation_edit_get_type(void);

void ags_automation_edit_reset_vertically(AgsAutomationEdit *automation_edit, guint flags);
void ags_automation_edit_reset_horizontally(AgsAutomationEdit *automation_edit, guint flags);

void ags_automation_edit_draw_position(GtkVBox *drawing_area);
void ags_automation_edit_draw_scroll(GtkVBox *drawing_area,
				     gdouble position);

GtkDrawingArea* ags_automation_edit_add_area(AgsAutomationEdit *automation_edit,
					     AgsAutomation *automation);

AgsAutomationEdit* ags_automation_edit_new();

#endif /*__AGS_AUTOMATION_EDIT_H__*/
