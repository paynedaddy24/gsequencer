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

#include <ags/X/editor/ags_automation_area.h>
#include <ags/X/editor/ags_automation_area_callbacks.h>

#include <ags/object/ags_connectable.h>

#include <ags/X/editor/ags_automation_edit.h>

void ags_automation_area_class_init(AgsAutomationAreaClass *automation_area);
void ags_automation_area_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_automation_area_init(AgsAutomationArea *automation_area);
void ags_automation_area_set_property(GObject *gobject,
				      guint prop_id,
				      const GValue *value,
				      GParamSpec *param_spec);
void ags_automation_area_get_property(GObject *gobject,
				      guint prop_id,
				      GValue *value,
				      GParamSpec *param_spec);
void ags_automation_area_connect(AgsConnectable *connectable);
void ags_automation_area_disconnect(AgsConnectable *connectable);

void ags_automation_area_paint(AgsAutomationArea *automation_area);

/**
 * SECTION:ags_automation_area
 * @short_description: area automations
 * @title: AgsAutomationArea
 * @section_id:
 * @include: ags/X/areaor/ags_automation_area.h
 *
 * The #AgsAutomationArea lets you area automations.
 */

enum{
  PROP_0,
  PROP_AUTOMATION,
};

static gpointer ags_automation_area_parent_class = NULL;
GtkStyle *automation_area_style;

GType
ags_automation_area_get_type(void)
{
  static GType ags_type_automation_area = 0;

  if(!ags_type_automation_area){
    static const GTypeInfo ags_automation_area_info = {
      sizeof (AgsAutomationAreaClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_automation_area_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAutomationArea),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_automation_area_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_automation_area_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_automation_area = g_type_register_static(GTK_TYPE_DRAWING_AREA,
						      "AgsAutomationArea\0", &ags_automation_area_info,
						      0);
    
    g_type_add_interface_static(ags_type_automation_area,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_automation_area);
}

void
ags_automation_area_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_automation_area_connect;
  connectable->disconnect = ags_automation_area_disconnect;
}

void
ags_automation_area_class_init(AgsAutomationAreaClass *automation_area)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_automation_area_parent_class = g_type_class_peek_parent(automation_area);

  gobject = (GObjectClass *) automation_area;

  gobject->set_property = ags_automation_area_set_property;
  gobject->get_property = ags_automation_area_get_property;

  /* properties */
  /**
   * AgsAutomationArea:automation:
   *
   * The assigned #AgsAutomation
   * 
   * Since: 0.4.0
   */
  param_spec = g_param_spec_object("automation\0",
				   "automation of automation area\0",
				   "The automation of automation area\0",
				   AGS_TYPE_AUTOMATION,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUTOMATION,
				  param_spec);
}

void
ags_automation_area_init(AgsAutomationArea *automation_area)
{
  automation_area->automation = NULL;

  gtk_widget_set_style((GtkWidget *) automation_area, automation_area_style);
  gtk_widget_set_events(GTK_WIDGET (automation_area), GDK_EXPOSURE_MASK
			| GDK_LEAVE_NOTIFY_MASK
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_POINTER_MOTION_MASK
			| GDK_POINTER_MOTION_HINT_MASK
			);
}

void
ags_automation_area_set_property(GObject *gobject,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *param_spec)
{
  AgsAutomationArea *automation_area;

  automation_area = AGS_AUTOMATION_AREA(gobject);

  switch(prop_id){
  case PROP_AUTOMATION:
    {
      AgsAutomation *automation;

      automation = g_value_get_object(value);

      if(automation == automation_area->automation){
	return;
      }

      if(automation_area->automation != NULL){
	g_object_unref(automation_area->automation);
      }

      if(automation != NULL){
	g_object_ref(automation);
      }

      automation_area->automation = automation;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_automation_area_get_property(GObject *gobject,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *param_spec)
{
  AgsAutomationArea *automation_area;

  automation_area = AGS_AUTOMATION_AREA(gobject);

  switch(prop_id){
  case PROP_AUTOMATION:
    {
      g_value_set_object(value, automation_area->automation);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_automation_area_connect(AgsConnectable *connectable)
{
  AgsAutomationArea *automation_area;

  automation_area = AGS_AUTOMATION_AREA(connectable);

  g_signal_connect_after((GObject *) automation_area, "expose_event\0",
			 G_CALLBACK (ags_automation_area_expose_event), (gpointer) automation_area);

  g_signal_connect_after((GObject *) automation_area, "configure_event\0",
			 G_CALLBACK (ags_automation_area_configure_event), (gpointer) automation_area);
}

void
ags_automation_area_disconnect(AgsConnectable *connectable)
{
  //TODO:JK: implement me
}

/**
 * ags_automation_area_draw_strip:
 * @automation_area: the #AgsAutomationArea
 * @cr: the #cairo_t surface
 * @channel: the #AgsChannel
 *
 * Plot data.
 *
 * Since: 0.4
 */
void
ags_automation_area_draw_strip(AgsAutomationArea *automation_area, cairo_t *cr)
{
  AgsAutomationEdit *automation_edit;
  gchar *control_name;
  gdouble width, height;

  automation_edit = gtk_widget_get_ancestor(automation_area,
					    AGS_TYPE_AUTOMATION_EDIT);

  control_name = automation_area->automation->control_name;

  width = (gdouble) GTK_WIDGET(automation_area)->allocation.width;
  height = (gdouble) GTK_WIDGET(automation_area)->allocation.height;

  /* background */
  cairo_set_source_rgb(cr, 0.5, 0.4, 0.0);
  cairo_rectangle(cr, 0.0, 0.0, width, height);
  cairo_fill(cr);

  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
  cairo_set_line_width(cr, 1.0);
  cairo_rectangle(cr, 0.0, 0.0, width, height);
  cairo_stroke(cr);

  /*  */
  //TODO:JK: implement me
}

/**
 * ags_automation_area_draw_scale:
 * @automation_area: the #AgsAutomationArea
 * @cr: the #cairo_t surface
 *
 * Draw a scale and its boundaries.
 *
 * Since: 0.4
 */
void
ags_automation_area_draw_scale(AgsAutomationArea *automation_area, cairo_t *cr)
{
  AgsAutomationEdit *automation_edit;
  gdouble lower, upper;
  gdouble width, height;
  gdouble translated_ground;
  
  const static double dashes = {
    0.25,
  };

  automation_edit = gtk_widget_get_ancestor(automation_area,
					    AGS_TYPE_AUTOMATION_EDIT);

  lower = automation_area->automation->lower;
  upper = automation_area->automation->upper;

  width = (gdouble) GTK_WIDGET(automation_area)->allocation.width;
  height = (gdouble) GTK_WIDGET(automation_area)->allocation.height;

  if(lower < 0.0){
    if(upper < 0.0){
      translated_ground = (-1.0 * (lower - upper)) / 2.0;
    }else{
      translated_ground = (upper - lower) / 2.0;
    }
  }else{
    if(upper > 0.0){
      translated_ground = (upper - lower) / 2.0;
    }else{
      g_warning("invalid boundaries for scale\0");
    }
  }

  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);

  cairo_move_to(cr,
		0.0, translated_ground);
  cairo_line_to(cr,
		width, translated_ground);

  cairo_set_dash(cr,
		 &dashes,
		 1,
		 0.0);
  cairo_stroke(cr);
}

/**
 * ags_automation_area_draw_automation:
 * @automation_area: a #AgsAutomationArea
 * @cr: the #cairo_t surface
 *
 * Draw the #AgsAutomation of selected #AgsMachine on @automation_edit.
 *
 * Since: 0.4
 */
void
ags_automation_area_draw_automation(AgsAutomationArea *automation_area, cairo_t *cr)
{
  AgsAutomationEdit *automation_edit;
  AgsAcceleration *current, *prev;
  guint width, height;
  gdouble x0, x1;
  GList *list;

  automation_edit = gtk_widget_get_ancestor(automation_area,
					    AGS_TYPE_AUTOMATION_EDIT);


  width = GTK_WIDGET(automation_area)->allocation.width;
  height = GTK_WIDGET(automation_area)->allocation.height;

  x0 = gtk_scrolled_window_get_hadjustment(automation_edit->scrolled_window)->value;
  x1 = x0 + width;

  /*  */	
  list = AGS_ACCELERATION(automation_area->automation->acceleration);
  prev = NULL;

  while(list != NULL){
    current = AGS_ACCELERATION(list->data);

    if(prev != NULL){
      ags_automation_area_draw_surface(automation_area, cr,
				       prev->x, prev->y,
				       current->x, current->y);
    }

    prev = current;

    list = list->next;
  }
}

/**
 * ags_automation_area_draw_surface:
 * @automation_area: the #AgsAutomationArea
 * @cr: the #cairo_t surface
 * @x0: x offset
 * @y0: y offset
 * @x1: x offset
 * @y1: y offset
 *
 * Draw a portion of data.
 *
 * Since: 0.4
 */
void
ags_automation_area_draw_surface(AgsAutomationArea *automation_area, cairo_t *cr,
				 gdouble x0, gdouble y0,
				 gdouble x1, gdouble y1)
{
  AgsAutomationEdit *automation_edit;
  gdouble width, height;

  automation_edit = gtk_widget_get_ancestor(automation_area,
					    AGS_TYPE_AUTOMATION_EDIT);

  width = (gdouble) GTK_WIDGET(automation_area)->allocation.width;
  height = (gdouble) GTK_WIDGET(automation_area)->allocation.height;

  cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);

  /* area */
  cairo_rectangle(cr, x0, 0.0, x1 - x0, ((y0 < y1) ? y0: y1));
  cairo_fill(cr);

  /* acceleration */
  cairo_move_to(cr,
		x0, y0);
  cairo_line_to(cr,
		x1, y1);

  if(y0 > y1){
    cairo_line_to(cr,
		  x0, y0);
  }else{
    cairo_line_to(cr,
		  x1, y1);
  }

  cairo_close_path(cr);
  cairo_fill(cr);
}

void
ags_automation_area_paint(AgsAutomationArea *automation_area)
{
  if(GTK_WIDGET_VISIBLE(automation_area)){
   cairo_t *cr;

   cr = gdk_cairo_create(GTK_WIDGET(automation_area)->window);

   cairo_push_group(cr);
    
    ags_automation_area_draw_strip(automation_area,
				   cr);
    ags_automation_area_draw_scale(automation_area,
				   cr);
    ags_automation_area_draw_automation(automation_area,
					cr);

    cairo_pop_group_to_source(cr);
    cairo_paint(cr);
  }
}

/**
 * ags_automation_area_new:
 *
 * Create a new #AgsAutomationArea.
 *
 * Since: 0.4
 */
AgsAutomationArea*
ags_automation_area_new()
{
  AgsAutomationArea *automation_area;

  automation_area = (AgsAutomationArea *) g_object_new(AGS_TYPE_AUTOMATION_AREA, NULL);

  return(automation_area);
}
