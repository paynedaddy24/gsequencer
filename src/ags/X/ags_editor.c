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

#include <ags/X/ags_editor.h>
#include <ags/X/ags_editor_callbacks.h>

#include <ags-lib/object/ags_connectable.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_notation.h>

#include <math.h>
#include <cairo.h>

void ags_editor_class_init(AgsEditorClass *editor);
void ags_editor_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_editor_init(AgsEditor *editor);
void ags_editor_set_property(GObject *gobject,
			     guint prop_id,
			     const GValue *value,
			     GParamSpec *param_spec);
void ags_editor_get_property(GObject *gobject,
			     guint prop_id,
			     GValue *value,
			     GParamSpec *param_spec);
void ags_editor_finalize(GObject *gobject);
void ags_editor_connect(AgsConnectable *connectable);
void ags_editor_disconnect(AgsConnectable *connectable);
void ags_editor_destroy(GtkObject *object);
void ags_editor_show(GtkWidget *widget);

void ags_editor_real_change_machine(AgsEditor *editor, AgsMachine *machine);

GtkMenu* ags_editor_popup_new(AgsEditor *editor);

enum{
  CHANGE_MACHINE,
  LAST_SIGNAL,
};

enum{
  PROP_0,
  PROP_DEVOUT,
};

static gpointer ags_editor_parent_class = NULL;
static guint editor_signals[LAST_SIGNAL];

GtkStyle *editor_style;

GType
ags_editor_get_type(void)
{
  static GType ags_type_editor = 0;

  if(!ags_type_editor){
    static const GTypeInfo ags_editor_info = {
      sizeof (AgsEditorClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_editor_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsEditor),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_editor_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_editor_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_editor = g_type_register_static(GTK_TYPE_VBOX,
					     "AgsEditor\0", &ags_editor_info,
					     0);
    
    g_type_add_interface_static(ags_type_editor,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_editor);
}

void
ags_editor_class_init(AgsEditorClass *editor)
{
  GObjectClass *gobject;
  GtkWidgetClass *widget;
  GParamSpec *param_spec;

  ags_editor_parent_class = g_type_class_peek_parent(editor);

  /* GObjectClass */
  gobject = (GObjectClass *) editor;

  gobject->set_property = ags_editor_set_property;
  gobject->get_property = ags_editor_get_property;

  gobject->finalize = ags_editor_finalize;

  /* properties */
  param_spec = g_param_spec_object("devout\0",
				   "assigned devout\0",
				   "The devout it is assigned with\0",
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DEVOUT,
				  param_spec);

  /*  */
  editor->change_machine = ags_editor_real_change_machine;

  editor_signals[CHANGE_MACHINE] =
    g_signal_new("change_machine\0",
                 G_TYPE_FROM_CLASS (editor),
                 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsEditorClass, change_machine),
                 NULL, NULL,
                 g_cclosure_marshal_VOID__OBJECT,
                 G_TYPE_NONE, 1,
                 G_TYPE_OBJECT);
}

void
ags_editor_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_editor_connect;
  connectable->disconnect = ags_editor_disconnect;
}

void
ags_editor_init(AgsEditor *editor)
{
  GtkHPaned *paned;
  GtkScrolledWindow *scrolled_window;
  GtkTable *table;
  GtkLabel *label;
  GtkHBox *hbox;
  GtkMenuToolButton *menu_button;

  g_signal_connect_after((GObject *) editor, "parent-set\0",
			 G_CALLBACK(ags_editor_parent_set_callback), editor);

  editor->flags = 0;

  editor->version = AGS_EDITOR_DEFAULT_VERSION;
  editor->build_id = AGS_EDITOR_DEFAULT_BUILD_ID;

  editor->devout = NULL;

  editor->popup = ags_editor_popup_new(editor);

  editor->toolbar = ags_toolbar_new();
  gtk_box_pack_start((GtkBox *) editor,
		     (GtkWidget *) editor->toolbar,
		     FALSE, FALSE, 0);

  paned = (GtkHPaned *) gtk_hpaned_new();
  gtk_box_pack_start((GtkBox *) editor,
		     (GtkWidget *) paned,
		     TRUE, TRUE, 0);

  scrolled_window = (GtkScrolledWindow *) gtk_scrolled_window_new(NULL, NULL);
  gtk_paned_pack1((GtkPaned *) paned, (GtkWidget *) scrolled_window, FALSE, TRUE);
  //  gtk_widget_set_size_request((GtkWidget *) scrolled_window, 180, -1);

  editor->index_radio = (GtkVBox *) gtk_vbox_new(FALSE, 0);
  gtk_scrolled_window_add_with_viewport(scrolled_window, (GtkWidget *) editor->index_radio);

  hbox = (GtkHBox *) gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(editor->index_radio),
		     GTK_WIDGET(hbox),
		     FALSE, FALSE,
		     0);

  label = (GtkLabel *) gtk_label_new("Notation\0");
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(label),
		     FALSE, FALSE,
		     0);

  menu_button = g_object_new(GTK_TYPE_MENU_TOOL_BUTTON,
			     "stock-id\0", GTK_STOCK_EXECUTE,
			     "menu\0", editor->popup,
			     NULL);
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(menu_button),
		     FALSE, FALSE,
		     0);

  editor->selected = NULL;

  table = (GtkTable *) gtk_table_new(4, 3, FALSE);
  gtk_paned_pack2((GtkPaned *) paned, (GtkWidget *) table, TRUE, FALSE);
  
  editor->notebook = ags_notebook_new();
  gtk_table_attach(table, (GtkWidget *) editor->notebook,
		   0, 3, 0, 1,
		   GTK_FILL|GTK_EXPAND, GTK_FILL,
		   0, 0);

  editor->meter = ags_meter_new();
  gtk_table_attach(table, (GtkWidget *) editor->meter,
		   0, 1, 1, 2,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  editor->note_edit = ags_note_edit_new();
  gtk_table_attach(table, (GtkWidget *) editor->note_edit,
		   0, 1, 1, 2,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  editor->tact_counter = 0;
}

void
ags_editor_set_property(GObject *gobject,
			guint prop_id,
			const GValue *value,
			GParamSpec *param_spec)
{
  AgsEditor *editor;

  editor = AGS_EDITOR(gobject);

  switch(prop_id){
  case PROP_DEVOUT:
    {
      AgsDevout *devout;

      devout = (AgsDevout *) g_value_get_object(value);

      if(editor->devout == devout)
	return;

      if(devout != NULL)
	g_object_ref(devout);

      editor->devout = devout;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_editor_get_property(GObject *gobject,
			guint prop_id,
			GValue *value,
			GParamSpec *param_spec)
{
  AgsEditor *editor;

  editor = AGS_EDITOR(gobject);

  switch(prop_id){
  case PROP_DEVOUT:
    g_value_set_object(value, editor->devout);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_editor_finalize(GObject *gobject)
{
  //TODO:JK: implement me
}

void
ags_editor_connect(AgsConnectable *connectable)
{
  GtkHPaned *hpaned;
  AgsEditor *editor;

  editor = AGS_EDITOR(connectable);

  g_signal_connect((GObject *) editor, "destroy\0",
		   G_CALLBACK(ags_editor_destroy_callback), (gpointer) editor);

  g_signal_connect((GObject *) editor, "show\0",
		   G_CALLBACK(ags_editor_show_callback), (gpointer) editor);

  g_signal_connect ((GObject *) editor->index_radio, "button_press_event\0",
                    G_CALLBACK (ags_editor_button_press_callback), (gpointer) editor);

  /*  */
  //TODO:JK: uncomment me
  //  g_signal_connect((GObject *) editor->devout, "tic\0",
  //		   G_CALLBACK(ags_editor_tic_callback), (gpointer) editor);

  /*  */
  ags_toolbar_connect(AGS_CONNECTABLE(editor->toolbar));
  ags_notebook_connect(AGS_CONNECTABLE(editor->notebook));
  ags_meter_connect(AGS_CONNECTABLE(editor->meter));
  ags_note_edit_connect(AGS_CONNECTABLE(editor->meter));
}

void
ags_editor_disconnect(AgsConnectable *connectable)
{
}

void
ags_editor_destroy(GtkObject *object)
{
}

void
ags_editor_show(GtkWidget *widget)
{
  GList *list;
  /*
  list = gtk_container_get_children((GtkContainer *) widget);

  while(list != NULL){
    gtk_widget_show_all((GtkWidget *) list->data);

    list = list->next;
  }
  */
}

void
ags_editor_real_change_machine(AgsEditor *editor, AgsMachine *machine)
{
}

void
ags_editor_change_machine(AgsEditor *editor, AgsMachine *machine)
{
  g_return_if_fail(AGS_IS_EDITOR(editor));

  g_object_ref((GObject *) editor);
  g_signal_emit((GObject *) editor,
		editor_signals[CHANGE_MACHINE], 0,
		machine);
  g_object_unref((GObject *) editor);
}

AgsEditor*
ags_editor_new()
{
  AgsEditor *editor;

  editor = (AgsEditor *) g_object_new(AGS_TYPE_EDITOR, NULL);

  return(editor);
}

GtkMenu*
ags_editor_popup_new(AgsEditor *editor)
{
  GtkMenu *popup;
  GtkMenuItem *item;
  GList *list;

  popup = (GtkMenu *) gtk_menu_new();
  g_object_set_data((GObject *) popup, g_type_name(AGS_TYPE_EDITOR), editor);

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("add tab\0"));
  gtk_menu_shell_append((GtkMenuShell*) popup, (GtkWidget*) item);

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("remove tab\0"));
  gtk_menu_shell_append((GtkMenuShell*) popup, (GtkWidget*) item);

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("add index\0"));
  gtk_menu_shell_append((GtkMenuShell*) popup, (GtkWidget*) item);

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("remove index\0"));
  gtk_menu_shell_append((GtkMenuShell*) popup, (GtkWidget*) item);

  item = (GtkMenuItem *) gtk_menu_item_new_with_label(g_strdup("link index\0"));
  gtk_menu_shell_append((GtkMenuShell*) popup, (GtkWidget*) item);

  list = gtk_container_get_children((GtkContainer *) popup);

  g_signal_connect(G_OBJECT(list->data), "activate\0",
		   G_CALLBACK(ags_editor_popup_add_tab_callback), (gpointer) popup);

  list = list->next;
  g_signal_connect(G_OBJECT(list->data), "activate\0",
		   G_CALLBACK(ags_editor_popup_remove_tab_callback), (gpointer) popup);

  list = list->next;
  g_signal_connect(G_OBJECT(list->data), "activate\0",
		   G_CALLBACK(ags_editor_popup_add_index_callback), (gpointer) popup);

  list = list->next;
  g_signal_connect(G_OBJECT(list->data), "activate\0",
		   G_CALLBACK(ags_editor_popup_remove_index_callback), (gpointer) popup);

  list = list->next;
  g_signal_connect(G_OBJECT(list->data), "activate\0",
		   G_CALLBACK(ags_editor_popup_link_index_callback), (gpointer) popup);

  gtk_widget_show_all((GtkWidget *) popup);

  return(popup);
}
