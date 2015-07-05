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

#include <ags/X/ags_editor_callbacks.h>

#include <ags/main.h>

#include <ags/thread/ags_audio_loop.h>
#include <ags/thread/ags_task_thread.h>

#include <ags/audio/ags_notation.h>
#include <ags/audio/task/ags_scroll_on_play.h>

#include <ags/X/ags_window.h>

#include <ags/X/editor/ags_toolbar.h>
#include <ags/X/editor/ags_notebook.h>
#include <ags/X/editor/ags_piano.h>
#include <ags/X/editor/ags_note_edit.h>
#include <ags/X/editor/ags_pattern_edit.h>

#include <math.h>
#include <string.h>
#include <cairo.h>

void
ags_editor_parent_set_callback(GtkWidget  *widget, GtkObject *old_parent, AgsEditor *editor)
{
  if(old_parent != NULL)
    return;

  if(AGS_IS_NOTE_EDIT(editor->current_edit_widget)){
    AGS_NOTE_EDIT(editor->current_edit_widget)->flags |= AGS_NOTE_EDIT_RESETING_HORIZONTALLY;
    ags_note_edit_reset_horizontally(AGS_NOTE_EDIT(editor->current_edit_widget), AGS_NOTE_EDIT_RESET_HSCROLLBAR);
    AGS_NOTE_EDIT(editor->current_edit_widget)->flags &= (~AGS_NOTE_EDIT_RESETING_HORIZONTALLY);
  }else if(AGS_IS_PATTERN_EDIT(editor->current_edit_widget)){
    AGS_PATTERN_EDIT(editor->current_edit_widget)->flags |= AGS_PATTERN_EDIT_RESETING_HORIZONTALLY;
    ags_pattern_edit_reset_horizontally(AGS_PATTERN_EDIT(editor->current_edit_widget), AGS_PATTERN_EDIT_RESET_HSCROLLBAR);
    AGS_PATTERN_EDIT(editor->current_edit_widget)->flags &= (~AGS_PATTERN_EDIT_RESETING_HORIZONTALLY);
  }
}

void
ags_editor_tic_callback(AgsDevout *devout,
			AgsEditor *editor)

{
  AgsWindow *window;

  window = AGS_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(editor)));

  if(gtk_toggle_button_get_active(window->navigation->scroll)){
    AgsScrollOnPlay *scroll_on_play;
    double tact_factor, zoom_factor;
    double tact;
    gdouble step;
    
    zoom_factor = 0.25;

    tact_factor = exp2(6.0 - (double) gtk_combo_box_get_active((GtkComboBox *) window->editor->toolbar->zoom));
    tact = exp2((double) gtk_combo_box_get_active((GtkComboBox *) window->editor->toolbar->zoom) - 2.0);

    if((guint) (32 * devout->tact_counter) % 16 == 0){
      scroll_on_play = ags_scroll_on_play_new(editor, 64.0);
      ags_task_thread_append_task(AGS_TASK_THREAD(AGS_AUDIO_LOOP(AGS_MAIN(window->ags_main)->main_loop)->task_thread),
				  AGS_TASK(scroll_on_play));
    }
  }
}

void
ags_editor_machine_changed_callback(AgsMachineSelector *machine_selector, AgsMachine *machine,
				    AgsEditor *editor)
{
  ags_editor_machine_changed(editor, machine);
}
