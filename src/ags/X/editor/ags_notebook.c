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

#include <ags/X/editor/ags_notebook.h>
#include <ags/X/editor/ags_notebook_callbacks.h>

#include <ags/object/ags_connectable.h>

void ags_notebook_class_init(AgsNotebookClass *notebook);
void ags_notebook_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_notebook_init(AgsNotebook *notebook);
void ags_notebook_connect(AgsConnectable *connectable);
void ags_notebook_disconnect(AgsConnectable *connectable);

void ags_notebook_paint(AgsNotebook *notebook);

AgsNotebookTab* ags_notebook_tab_alloc();

/**
 * SECTION:ags_notebook
 * @short_description: select channel
 * @title: AgsNotebook
 * @section_id:
 * @include: ags/X/editor/ags_notebook.h
 *
 * The #AgsNotebook lets select/deselect channels to edit.
 */

GtkStyle *notebook_style;

GType
ags_notebook_get_type(void)
{
  static GType ags_type_notebook = 0;

  if(!ags_type_notebook){
    static const GTypeInfo ags_notebook_info = {
      sizeof (AgsNotebookClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_notebook_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsNotebook),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_notebook_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_notebook_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_notebook = g_type_register_static(GTK_TYPE_VBOX,
					       "AgsNotebook\0", &ags_notebook_info,
					       0);
    
    g_type_add_interface_static(ags_type_notebook,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_notebook);
}

void
ags_notebook_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_notebook_connect;
  connectable->disconnect = ags_notebook_disconnect;
}

void
ags_notebook_class_init(AgsNotebookClass *notebook)
{
}

void
ags_notebook_init(AgsNotebook *notebook)
{
  gtk_widget_set_style((GtkWidget *) notebook, notebook_style);

  notebook->flags = 0;

  notebook->hbox = (GtkHBox *) gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(notebook),
		     GTK_WIDGET(notebook->hbox),
		     FALSE, FALSE,
		     0);

  notebook->tabs = NULL;
  notebook->child = NULL;
}

void
ags_notebook_connect(AgsConnectable *connectable)
{
  AgsNotebook *notebook;

  notebook = AGS_NOTEBOOK(connectable);

  editor = (AgsEditor *) gtk_widget_get_ancestor((GtkWidget *) notebook, AGS_TYPE_EDITOR);
}

void
ags_notebook_disconnect(AgsConnectable *connectable)
{
  //TODO:JK: implement me
}

AgsNotebookTab*
ags_notebook_tab_alloc()
{
  AgsNotebookTab *notebook_tab;

  notebook_tab = (AgsNotebookTab *) malloc(sizeof(AgsNotebookTab));

  notebook_tab->flags = 0;

  notebook_tab->toggle = NULL;
  notebook_tab->data = NULL;

  return(notebook_tab);
}

gint
ags_notebook_tab_index(AgsNotebook *notebook,
		       gpointer data)
{
  GList *list;
  gint i;

  list = notebook->tabs;

  for(i = 0; list != NULL; i++){
    if(AGS_NOTEBOOK_TAB(list->data)->data == data){
      return(i);
    }

    list = list->next;
  }
  
  return(-1);
}

gint
ags_notebook_add_tab(AgsNotebook *notebook)
{
  AgsNotebookTab *tab;
  gint tab_index;

  tab = ags_notebook_tab_alloc();

  notebook->tabs = g_list_prepend(notebook->tabs,
				  tab);
  tab_index = g_list_length(notebook->tabs);

  if((AGS_NOTEBOOK_SHOW_AUDIO_CHANNEL & (notebook->flags)) != 0){
    tab->toggle = (GtkToggleButton *) gtk_toggle_button_new_with_label(g_strdup_printf("channel %d\0",
										       tab_index));
  }else if((AGS_NOTEBOOK_SHOW_PAD & (notebook->flags)) != 0){
    tab->toggle = (GtkToggleButton *) gtk_toggle_button_new_with_label(g_strdup_printf("pad %d\0",
										       tab_index));
  }else if((AGS_NOTEBOOK_SHOW_LINE & (notebook->flags)) != 0){
    tab->toggle = (GtkToggleButton *) gtk_toggle_button_new_with_label(g_strdup_printf("line %d\0",
										       tab_index));
  }else{
    tab->toggle = (GtkToggleButton *) gtk_toggle_button_new_with_label(g_strdup_printf("%d\0",
										       tab_index));
  }
  
  gtk_toggle_button_set_active(tab->toggle, TRUE);
  gtk_box_pack_start(GTK_BOX(notebook->hbox),
		     GTK_WIDGET(tab->toggle),
		     FALSE, FALSE,
		     0);

  return(tab_index);
}

gint
ags_notebook_next_active_tab(AgsNotebook *notebook,
			     gint position)
{
  GList *list;
  gint length;
  gint i;

  list = notebook->tabs;
  length = g_list_length(notebook->tabs);

  for(i = 0; i < length - position && list != NULL; i++){
    if(gtk_toggle_button_get_active(AGS_NOTEBOOK_TAB(list->data)->toggle)){
      return(position + i);
    }

    list = list->next;
  }

  return(-1);
}

void
ags_notebook_insert_tab(AgsNotebook *notebook,
			gint position)
{
  AgsNotebookTab *tab;
  GList *list;
  gchar *str;
  gint length;
  guint i;

  length = g_list_length(notebook->tabs);

  tab = ags_notebook_tab_alloc();
  notebook->tabs = g_list_insert(notebook->tabs,
				 tab,
				 length - position);

  tab->toggle = (GtkToggleButton *) gtk_toggle_button_new_with_label(g_strdup_printf("channel %d\0",
										     position + 1));
  gtk_box_pack_start(GTK_BOX(notebook->hbox),
		     GTK_WIDGET(tab->toggle),
		     FALSE, FALSE,
		     0);
  gtk_box_reorder_child(GTK_BOX(notebook->hbox),
			GTK_WIDGET(tab->toggle),
			position);

  list = g_list_nth(notebook->tabs,
		    length - position);
  list = list->prev;
  i = position + 2;
  
  while(list != NULL){
    if((AGS_NOTEBOOK_SHOW_AUDIO_CHANNEL & (notebook->flags)) != 0){
      g_strdup_printf("channel %d\0",
		      i);
    }else if((AGS_NOTEBOOK_SHOW_PAD & (notebook->flags)) != 0){
      g_strdup_printf("pad %d\0",
		      i);
    }else if((AGS_NOTEBOOK_SHOW_LINE & (notebook->flags)) != 0){
      g_strdup_printf("line %d\0",
		      i);
    }else{
      g_strdup_printf("%d\0",
		      i);
    }
    
    gtk_button_set_label(AGS_NOTEBOOK_TAB(list->data)->toggle,
			 str);
    list = list->prev;
    i++;
  }
}

void
ags_notebook_remove_tab(AgsNotebook *notebook,
			gint nth)
{
  AgsNotebookTab *tab;
  gint length;

  if(notebook->tabs == NULL){
    return;
  }
  
  length = g_list_length(notebook->tabs);

  tab = g_list_nth_data(notebook->tabs,
			length - nth - 1);

  notebook->tabs = g_list_remove(notebook->tabs,
				 tab);
  gtk_widget_destroy(GTK_WIDGET(tab->toggle));
  free(tab);
}

void
ags_notebook_add_child(AgsNotebook *notebook,
		       GtkWidget *child)
{
  gtk_box_pack_start(GTK_BOX(notebook),
		     child,
		     FALSE, FALSE,
		     0);
}

void
ags_notebook_remove_child(AgsNotebook *notebook,
			  GtkWidget *child)
{
  gtk_widget_destroy(child);
}

/**
 * ags_notebook_new:
 *
 * Create a new #AgsNotebook.
 *
 * Since: 0.4
 */
AgsNotebook*
ags_notebook_new()
{
  AgsNotebook *notebook;

  notebook = (AgsNotebook *) g_object_new(AGS_TYPE_NOTEBOOK, NULL);

  return(notebook);
}
