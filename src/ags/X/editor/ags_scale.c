/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2015 Joël Krähemann
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

#include <ags/X/editor/ags_scale.h>
#include <ags/X/editor/ags_scale_callbacks.h>

#include <ags/object/ags_connectable.h>

#include <ags/X/ags_editor.h>

#include <math.h>

void ags_scale_class_init(AgsScaleClass *scale);
void ags_scale_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_scale_init(AgsScale *scale);
void ags_scale_connect(AgsConnectable *connectable);
void ags_scale_disconnect(AgsConnectable *connectable);
void ags_scale_destroy(GtkObject *object);
void ags_scale_show(GtkWidget *widget);

/**
 * SECTION:ags_scale
 * @short_description: scale widget
 * @title: AgsScale
 * @section_id:
 * @include: ags/X/editor/ags_scale.h
 *
 * The #AgsScale draws you a scale.
 */

GtkStyle *scale_style;

GType
ags_scale_get_type(void)
{
  static GType ags_type_scale = 0;

  if (!ags_type_scale){
    static const GTypeInfo ags_scale_info = {
      sizeof (AgsScaleClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_scale_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsScale),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_scale_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_scale_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_scale = g_type_register_static(GTK_TYPE_DRAWING_AREA,
					    "AgsScale\0", &ags_scale_info,
					    0);
    
    g_type_add_interface_static(ags_type_scale,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return (ags_type_scale);
}

void
ags_scale_class_init(AgsScaleClass *scale)
{
}

void
ags_scale_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_scale_connect;
  connectable->disconnect = ags_scale_disconnect;
}

void
ags_scale_init(AgsScale *scale)
{
  GtkWidget *widget;

  widget = (GtkWidget *) scale;
  gtk_widget_set_style(widget, scale_style);
  gtk_widget_set_size_request(widget, 60, -1);
  gtk_widget_set_events (GTK_WIDGET (scale), GDK_EXPOSURE_MASK
                         | GDK_LEAVE_NOTIFY_MASK
                         | GDK_BUTTON_PRESS_MASK
			 | GDK_BUTTON_RELEASE_MASK
			 );

  scale->scale_area = NULL;
}

void
ags_scale_connect(AgsConnectable *connectable)
{
  AgsScale *scale;

  scale = AGS_SCALE(connectable);

  g_signal_connect((GObject *) scale, "expose_event\0",
  		   G_CALLBACK(ags_scale_expose_event), (gpointer) scale);

  g_signal_connect((GObject *) scale, "configure_event\0",
  		   G_CALLBACK(ags_scale_configure_event), (gpointer) scale);
}

void
ags_scale_disconnect(AgsConnectable *connectable)
{
  //TODO:JK: implement me
}

void
ags_scale_paint(AgsScale *scale)
{
}

/**
 * ags_scale_new:
 *
 * Create a new #AgsScale.
 *
 * Since: 0.4.3
 */
AgsScale*
ags_scale_new()
{
  AgsScale *scale;

  scale = (AgsScale *) g_object_new(AGS_TYPE_SCALE, NULL);

  return(scale);
}
