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

#include <ags/X/import/ags_midi_import_wizard.h>
#include <ags/X/import/ags_midi_import_wizard_callbacks.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_applicable.h>

#include <ags/audio/ags_output.h>
#include <ags/audio/ags_input.h>

#include <ags/X/ags_window.h>

#include <ags/X/import/ags_track_collection.h>
#include <ags/X/import/ags_track_collection_mapper.h>

void ags_midi_import_wizard_class_init(AgsMidiImportWizardClass *midi_import_wizard);
void ags_midi_import_wizard_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_midi_import_wizard_applicable_interface_init(AgsApplicableInterface *applicable);
void ags_midi_import_wizard_init(AgsMidiImportWizard *midi_import_wizard);
void ags_midi_import_wizard_connect(AgsConnectable *connectable);
void ags_midi_import_wizard_disconnect(AgsConnectable *connectable);
void ags_midi_import_wizard_set_update(AgsApplicable *applicable, gboolean update);
void ags_midi_import_wizard_apply(AgsApplicable *applicable);
void ags_midi_import_wizard_reset(AgsApplicable *applicable);
gboolean ags_midi_import_wizard_delete_event(GtkWidget *widget, GdkEventAny *event);
void ags_midi_import_wizard_show(GtkWidget *widget);

/**
 * SECTION:ags_midi_import_wizard
 * @short_description: pack pad editors.
 * @title: AgsMidiImportWizard
 * @section_id:
 * @include: ags/X/ags_midi_import_wizard.h
 *
 * #AgsMidiImportWizard is a wizard to import midi files and do track mapping.
 */

static gpointer ags_midi_import_wizard_parent_class = NULL;

GType
ags_midi_import_wizard_get_type(void)
{
  static GType ags_type_midi_import_wizard = 0;

  if(!ags_type_midi_import_wizard){
    static const GTypeInfo ags_midi_import_wizard_info = {
      sizeof (AgsMidiImportWizardClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_midi_import_wizard_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsMidiImportWizard),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_midi_import_wizard_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_midi_import_wizard_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_applicable_interface_info = {
      (GInterfaceInitFunc) ags_midi_import_wizard_applicable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_midi_import_wizard = g_type_register_static(GTK_TYPE_DIALOG,
							 "AgsMidiImportWizard\0", &ags_midi_import_wizard_info,
							 0);

    g_type_add_interface_static(ags_type_midi_import_wizard,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_midi_import_wizard,
				AGS_TYPE_APPLICABLE,
				&ags_applicable_interface_info);
  }
  
  return(ags_type_midi_import_wizard);
}

void
ags_midi_import_wizard_class_init(AgsMidiImportWizardClass *midi_import_wizard)
{
  GObjectClass *gobject;
  GtkWidgetClass *widget;
  GParamSpec *param_spec;

  ags_midi_import_wizard_parent_class = g_type_class_peek_parent(midi_import_wizard);

  /* GObjectClass */
  gobject = (GObjectClass *) midi_import_wizard;

  /* GtkWidgetClass */
  widget = (GtkWidgetClass *) midi_import_wizard;

  widget->delete_event = ags_midi_import_wizard_delete_event;
  widget->show = ags_midi_import_wizard_show;
}

void
ags_midi_import_wizard_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_midi_import_wizard_connect;
  connectable->disconnect = ags_midi_import_wizard_disconnect;
}

void
ags_midi_import_wizard_applicable_interface_init(AgsApplicableInterface *applicable)
{
  applicable->set_update = ags_midi_import_wizard_set_update;
  applicable->apply = ags_midi_import_wizard_apply;
  applicable->reset = ags_midi_import_wizard_reset;
}

void
ags_midi_import_wizard_init(AgsMidiImportWizard *midi_import_wizard)
{
  GtkAlignment *alignment;

  midi_import_wizard->flags = AGS_MIDI_IMPORT_WIZARD_SHOW_FILE_CHOOSER;
  
  alignment = g_object_new(GTK_TYPE_ALIGNMENT,
			   NULL);
  gtk_widget_set_no_show_all(alignment,
			     TRUE);
  gtk_box_pack_start((GtkBox *) midi_import_wizard->dialog.vbox,
		     (GtkWidget*) alignment,
		     TRUE, TRUE,
		     0);
  
  midi_import_wizard->file_chooser = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_container_add(alignment,
		    midi_import_wizard->file_chooser);

  /**/
  alignment = gtk_alignment_new(0.0, 0.0,
				1.0, 1.0);
  gtk_widget_set_no_show_all(alignment,
			     TRUE);
  gtk_box_pack_start((GtkBox *) midi_import_wizard->dialog.vbox,
		     (GtkWidget*) alignment,
		     TRUE, TRUE,
		     0);

  midi_import_wizard->track_collection = ags_track_collection_new(AGS_TYPE_TRACK_COLLECTION_MAPPER,
								  0,
								  NULL);
  gtk_container_add(alignment,
		    midi_import_wizard->track_collection);
  
  gtk_dialog_add_buttons(midi_import_wizard,
			 GTK_STOCK_GO_BACK, GTK_RESPONSE_REJECT,
			 GTK_STOCK_GO_FORWARD, GTK_RESPONSE_ACCEPT,
			 GTK_STOCK_OK, GTK_RESPONSE_OK,
			 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			 NULL);
}

void
ags_midi_import_wizard_connect(AgsConnectable *connectable)
{
  AgsMidiImportWizard *midi_import_wizard;

  midi_import_wizard = AGS_MIDI_IMPORT_WIZARD(connectable);

  g_signal_connect(midi_import_wizard, "response\0",
		   G_CALLBACK(ags_midi_import_wizard_response_callback), NULL);

  ags_connectable_connect(AGS_CONNECTABLE(midi_import_wizard->track_collection));
}

void
ags_midi_import_wizard_disconnect(AgsConnectable *connectable)
{
  AgsMidiImportWizard *midi_import_wizard;

  midi_import_wizard = AGS_MIDI_IMPORT_WIZARD(connectable);

  ags_connectable_disconnect(AGS_CONNECTABLE(midi_import_wizard->track_collection));
}

void
ags_midi_import_wizard_set_update(AgsApplicable *applicable, gboolean update)
{
  AgsMidiImportWizard *midi_import_wizard;

  midi_import_wizard = AGS_MIDI_IMPORT_WIZARD(applicable);

  ags_applicable_set_update(AGS_APPLICABLE(midi_import_wizard->track_collection), update);
}

void
ags_midi_import_wizard_apply(AgsApplicable *applicable)
{
  AgsMidiImportWizard *midi_import_wizard;

  midi_import_wizard = AGS_MIDI_IMPORT_WIZARD(applicable);

  ags_applicable_apply(AGS_APPLICABLE(midi_import_wizard->track_collection));
}

void
ags_midi_import_wizard_reset(AgsApplicable *applicable)
{
  AgsMidiImportWizard *midi_import_wizard;

  midi_import_wizard = AGS_MIDI_IMPORT_WIZARD(applicable);

  ags_applicable_reset(AGS_APPLICABLE(midi_import_wizard->track_collection));
}

gboolean
ags_midi_import_wizard_delete_event(GtkWidget *widget, GdkEventAny *event)
{
  gtk_widget_hide(widget);

  return(TRUE);
}

void
ags_midi_import_wizard_show(GtkWidget *widget)
{
  AgsMidiImportWizard *midi_import_wizard;

  midi_import_wizard = AGS_MIDI_IMPORT_WIZARD(widget);

  GTK_WIDGET_CLASS(ags_midi_import_wizard_parent_class)->show(widget);
  
  if((AGS_MIDI_IMPORT_WIZARD_SHOW_FILE_CHOOSER & (midi_import_wizard->flags)) != 0){
    gtk_widget_show(midi_import_wizard->file_chooser->parent);
    gtk_widget_show_all(midi_import_wizard->file_chooser);
  }

  if((AGS_MIDI_IMPORT_WIZARD_SHOW_TRACK_COLLECTION & (midi_import_wizard->flags)) != 0){
    gtk_widget_show(midi_import_wizard->track_collection->parent);
    gtk_widget_show_all(midi_import_wizard->track_collection);
  }
}

/**
 * ags_midi_import_wizard_new:
 *
 * Creates an #AgsMidiImportWizard
 *
 * Returns: a new #AgsMidiImportWizard
 *
 * Since: 0.4.3
 */
AgsMidiImportWizard*
ags_midi_import_wizard_new()
{
  AgsMidiImportWizard *midi_import_wizard;

  midi_import_wizard = (AgsMidiImportWizard *) g_object_new(AGS_TYPE_MIDI_IMPORT_WIZARD,
							    NULL);
  
  return(midi_import_wizard);
}
