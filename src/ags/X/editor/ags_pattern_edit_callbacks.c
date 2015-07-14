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

#include <ags/X/editor/ags_pattern_edit_callbacks.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_output.h>
#include <ags/audio/ags_input.h>

#include <ags/X/ags_editor.h>

#include <ags/X/machine/ags_panel.h>
#include <ags/X/machine/ags_mixer.h>
#include <ags/X/machine/ags_drum.h>
#include <ags/X/machine/ags_matrix.h>
#include <ags/X/machine/ags_synth.h>
#include <ags/X/machine/ags_ffplayer.h>

#include <gdk/gdkkeysyms.h>

extern pthread_mutex_t ags_application_mutex;

void
ags_pattern_edit_set_audio_channels_callback(AgsAudio *audio,
					     guint audio_channels, guint audio_channels_old,
					     AgsPatternEdit *pattern_edit)
{
  AgsEditor *editor;
  AgsEditorChild *editor_child;

  GList *list;
  GList *tabs;
  GList *notation;
  guint i;

  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);

  editor_child = NULL;
  list = editor->editor_child;
  
  while(list != NULL){
    if(AGS_EDITOR_CHILD(list->data)->edit_widget == pattern_edit){
      editor_child = AGS_EDITOR_CHILD(list->data);
      break;
    }
    
    list = list->next;
  }
  
  if(audio_channels_old < audio_channels){
    notation = g_list_nth(audio->notation,
			  audio_channels_old - 1);

    for(i = audio_channels_old; i < audio_channels; i++){
      ags_notebook_insert_tab(editor_child->notebook,
			      i);
      tabs = editor_child->notebook->tabs;
      notation = notation->next;
      AGS_NOTEBOOK_TAB(tabs->data)->notation = notation->data;
      gtk_toggle_button_set_active(AGS_NOTEBOOK_TAB(tabs->data)->toggle,
				   TRUE);
    }

    gtk_widget_show_all(editor_child->notebook);
  }else{
    for(i = audio_channels; i < audio_channels_old; i++){
      ags_notebook_remove_tab(editor_child->notebook,
			      i);
    }
  }
}

void
ags_pattern_edit_set_pads_callback(AgsAudio *audio,
				   GType channel_type,
				   guint pads, guint pads_old,
				   AgsPatternEdit *pattern_edit)
{
  AgsEditor *editor;

  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);

  if((AGS_AUDIO_NOTATION_DEFAULT & (audio->flags)) != 0){
    if(!g_type_is_a(channel_type, AGS_TYPE_INPUT)){
      return;
    }
  }else{
    if(!g_type_is_a(channel_type, AGS_TYPE_OUTPUT)){
      return;
    }
  }

  if(AGS_IS_PATTERN_EDIT(pattern_edit)){
    ags_pattern_edit_set_map_height(pattern_edit,
				    pads * pattern_edit->control_height);
  }else if(AGS_IS_PATTERN_EDIT(pattern_edit)){
    ags_pattern_edit_set_map_height(AGS_PATTERN_EDIT(pattern_edit),
				    pads * AGS_PATTERN_EDIT(pattern_edit)->control_height);
  }
  
  gtk_widget_queue_draw(editor->current_meter);
}

gboolean
ags_pattern_edit_drawing_area_expose_event(GtkWidget *widget, GdkEventExpose *event, AgsPatternEdit *pattern_edit)
{
  AgsEditor *editor;
  guint width;
  double zoom, zoom_old;
  double tact_factor, zoom_factor;
  double tact;
  gdouble old_upper, new_upper;
  gdouble position;
  guint history;
  
  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);

  /* calculate zoom */
  history = gtk_combo_box_get_active(editor->toolbar->zoom);

  zoom = exp2((double) history - 2.0);
  zoom_old = exp2((double) editor->toolbar->zoom_history - 2.0);

  zoom_factor = 0.25;

  tact_factor = exp2(6.0 - (double) gtk_combo_box_get_active((GtkComboBox *) editor->toolbar->zoom));
  tact = exp2((double) gtk_combo_box_get_active((GtkComboBox *) editor->toolbar->zoom) - 2.0);

  editor->toolbar->zoom_history = history;

  position = GTK_RANGE(pattern_edit->hscrollbar)->adjustment->value;
  old_upper = GTK_RANGE(pattern_edit->hscrollbar)->adjustment->upper;
  
  pattern_edit->flags |= AGS_PATTERN_EDIT_RESETING_HORIZONTALLY;
  ags_pattern_edit_reset_horizontally(pattern_edit, AGS_PATTERN_EDIT_RESET_HSCROLLBAR |
				      AGS_PATTERN_EDIT_RESET_WIDTH);
  pattern_edit->flags &= (~AGS_PATTERN_EDIT_RESETING_HORIZONTALLY);

  new_upper = GTK_RANGE(pattern_edit->hscrollbar)->adjustment->upper;
  
  gtk_adjustment_set_value(GTK_RANGE(pattern_edit->hscrollbar)->adjustment,
			   position / old_upper * new_upper);

  /* reset adjsutments */
  width = widget->allocation.width;
  gtk_adjustment_set_upper(GTK_RANGE(pattern_edit->hscrollbar)->adjustment,
			   (gdouble) (pattern_edit->map_width - width));
  gtk_adjustment_set_upper(pattern_edit->ruler->adjustment,
			   (gdouble) (pattern_edit->map_width - width) / pattern_edit->control_current.control_width);

  if(editor->selected_machine != NULL){
    AgsMachine *machine;

    machine = editor->selected_machine;

    if(machine != NULL){
      cairo_t *cr;

      cr = gdk_cairo_create(widget->window);
      cairo_push_group(cr);

      if(AGS_IS_PANEL(machine)){
      }else if(AGS_IS_MIXER(machine)){
      }else if(AGS_IS_DRUM(machine)){
	ags_pattern_edit_draw_segment(pattern_edit, cr);
	ags_pattern_edit_draw_notation(pattern_edit, cr);
      }else if(AGS_IS_MATRIX(machine)){
	ags_pattern_edit_draw_segment(pattern_edit, cr);
	ags_pattern_edit_draw_notation(pattern_edit, cr);
      }else if(AGS_IS_SYNTH(machine)){
	ags_pattern_edit_draw_segment(pattern_edit, cr);
	ags_pattern_edit_draw_notation(pattern_edit, cr);
      }else if(AGS_IS_FFPLAYER(machine)){
	ags_pattern_edit_draw_segment(pattern_edit, cr);
	ags_pattern_edit_draw_notation(pattern_edit, cr);
      }

      if(editor->toolbar->selected_edit_mode == editor->toolbar->position){
	ags_pattern_edit_draw_position(pattern_edit, cr);
      }

      cairo_pop_group_to_source(cr);
      cairo_paint(cr);
    }

    ags_meter_paint(editor->current_meter);
  }

  return(TRUE);
}

gboolean
ags_pattern_edit_drawing_area_configure_event(GtkWidget *widget, GdkEventConfigure *event, AgsPatternEdit *pattern_edit)
{
  pattern_edit->flags |= AGS_PATTERN_EDIT_RESETING_VERTICALLY;
  ags_pattern_edit_reset_vertically(pattern_edit, AGS_PATTERN_EDIT_RESET_VSCROLLBAR);
  pattern_edit->flags &= (~AGS_PATTERN_EDIT_RESETING_VERTICALLY);

  pattern_edit->flags |= AGS_PATTERN_EDIT_RESETING_HORIZONTALLY;
  ags_pattern_edit_reset_horizontally(pattern_edit, AGS_PATTERN_EDIT_RESET_HSCROLLBAR);
  pattern_edit->flags &= (~AGS_PATTERN_EDIT_RESETING_HORIZONTALLY);  

  return(FALSE);
}

gboolean
ags_pattern_edit_drawing_area_button_press_event (GtkWidget *widget, GdkEventButton *event, AgsPatternEdit *pattern_edit)
{
  AgsMachine *machine;
  AgsEditor *editor;
  double tact, zoom;

  auto void ags_pattern_edit_drawing_area_button_press_event_set_control();

  void ags_pattern_edit_drawing_area_button_press_event_set_control(){
    AgsNote *note;
    guint note_offset_x0, note_offset_y0;
    guint note_x, note_y;

    if(pattern_edit->control.y0 >= pattern_edit->map_height || pattern_edit->control.x0 >= pattern_edit->map_width)
      return;

    note_offset_x0 = (guint) (ceil((double) (pattern_edit->control.x0_offset) / (double) (pattern_edit->control_current.control_width)));

    if(pattern_edit->control.x0 >= pattern_edit->control_current.x0)
      note_x = (guint) (floor((double) (pattern_edit->control.x0 - pattern_edit->control_current.x0) / (double) (pattern_edit->control_current.control_width)));
    else{
      note_offset_x0 -= 1;
      note_x = 0;
    }

    note_offset_y0 = (guint) ceil((double) (pattern_edit->control.y0_offset) / (double) (pattern_edit->control_height));

    if(pattern_edit->control.y0 >= pattern_edit->y0)
      note_y = (guint) floor((double) (pattern_edit->control.y0 - pattern_edit->y0) / (double) (pattern_edit->control_height));
    else{
      note_offset_y0 -= 1;
      note_y = 0;
    }

    note = pattern_edit->control.note;
    note->flags = AGS_NOTE_GUI;
    note->x[0] = (note_x * tact) + (note_offset_x0 * tact);
    note->x[1] = note->x[0] + 1;
    note->y = note_y + note_offset_y0;
  }

  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);
  gtk_widget_grab_focus(pattern_edit->drawing_area);

  if(editor->selected_machine != NULL &&
     event->button == 1 &&
     (machine = editor->selected_machine) != NULL){
    AgsToolbar *toolbar;

    toolbar = editor->toolbar;

    if(toolbar->selected_edit_mode == toolbar->position){
      pattern_edit->flags |= AGS_PATTERN_EDIT_POSITION_CURSOR;
    }else if(toolbar->selected_edit_mode == toolbar->edit){
      pattern_edit->flags |= AGS_PATTERN_EDIT_ADDING_NOTE;
    }else if(toolbar->selected_edit_mode == toolbar->clear){
      pattern_edit->flags |= AGS_PATTERN_EDIT_DELETING_NOTE;
    }else if(toolbar->selected_edit_mode == toolbar->select){
      pattern_edit->flags |= AGS_PATTERN_EDIT_SELECTING_NOTES;
    }

    /* store the events position */
    pattern_edit->control.x0_offset = (guint) round((double) GTK_RANGE(pattern_edit->hscrollbar)->adjustment->value);
    pattern_edit->control.y0_offset = (guint) round((double) GTK_RANGE(pattern_edit->vscrollbar)->adjustment->value);

    pattern_edit->control.x0 = (guint) event->x;
    pattern_edit->control.y0 = (guint) event->y;

    if((AGS_PATTERN_EDIT_ADDING_NOTE & (pattern_edit->flags)) != 0 ||
       (AGS_PATTERN_EDIT_POSITION_CURSOR & (pattern_edit->flags)) != 0){
      tact = exp2(6.0 - (double) gtk_combo_box_get_active(editor->toolbar->zoom));
      
      if(AGS_IS_PANEL(machine)){
      }else if(AGS_IS_MIXER(machine)){
      }else if(AGS_IS_DRUM(machine)){
	ags_pattern_edit_drawing_area_button_press_event_set_control();
      }else if(AGS_IS_MATRIX(machine)){
	ags_pattern_edit_drawing_area_button_press_event_set_control();
      }else if(AGS_IS_SYNTH(machine)){
	ags_pattern_edit_drawing_area_button_press_event_set_control();
      }else if(AGS_IS_FFPLAYER(machine)){
	ags_pattern_edit_drawing_area_button_press_event_set_control();
      }
    }
  }

  return(TRUE);
}

gboolean
ags_pattern_edit_drawing_area_button_release_event(GtkWidget *widget, GdkEventButton *event, AgsPatternEdit *pattern_edit)
{
  AgsMachine *machine;
  AgsEditor *editor;
  AgsNote *note, *note0;
  double tact;
  
  auto void ags_pattern_edit_drawing_area_button_release_event_set_control();
  auto void ags_pattern_edit_drawing_area_button_release_event_draw_control(cairo_t *cr);
  auto void ags_pattern_edit_drawing_area_button_release_event_delete_point();
  auto void ags_pattern_edit_drawing_area_button_release_event_select_region();

  void ags_pattern_edit_drawing_area_button_release_event_set_control(){
    GList *list_notation;
    gint i;
    guint note_x, note_y;
    guint note_offset_x1;
    gint history;

    if(pattern_edit->control.x0 >= pattern_edit->map_width)
      pattern_edit->control.x0 = pattern_edit->map_width - 1;

    note_offset_x1 = (guint) (ceil((double) (pattern_edit->control.x1_offset)  / (double) (pattern_edit->control_current.control_width)));

    if(pattern_edit->control.x1 >= pattern_edit->control_current.x0)
      note_x = (guint) (ceil((double) (pattern_edit->control.x1 - pattern_edit->control_current.x0) / (double) (pattern_edit->control_current.control_width)));
    else{
      note_offset_x1 -= 1;
      note_x = 0;
    }

    list_notation = machine->audio->notation;

    i = 0;

    while((i = ags_notebook_next_active_tab(editor->current_notebook,
							   i)) != -1){
      list_notation = g_list_nth(machine->audio->notation,
				 i);

      if(list_notation == NULL){
	i++;
	
	continue;
      }
      
      note0 = ags_note_duplicate(note);

      ags_notation_add_note(AGS_NOTATION(list_notation->data), note0, FALSE);

      i++;
    }

    fprintf(stdout, "x0 = %llu\nx1 = %llu\ny  = %llu\n\n\0", (long long unsigned int) note->x[0], (long long unsigned int) note->x[1], (long long unsigned int) note->y);
  }
  void ags_pattern_edit_drawing_area_button_release_event_draw_control(cairo_t *cr){
    guint x, y, width, height;

    widget = (GtkWidget *) pattern_edit->drawing_area;
    //    cr = gdk_cairo_create(widget->window);

    x = note->x[0] * pattern_edit->control_unit.control_width;
    width = note->x[1] * pattern_edit->control_unit.control_width;

    if(x < pattern_edit->control.x1_offset){
      if(width > pattern_edit->control.x1_offset){
	width -= (guint) x;
	x = 0;
      }else{
	return;
      }
    }else if(x < pattern_edit->control.x1_offset + widget->allocation.width){
      width -= x;
      x -= pattern_edit->control.x1_offset;
    }else{
      return;
    }

    if(x + width > widget->allocation.width)
      width = widget->allocation.width - x;

    y = note->y * pattern_edit->control_height;

    if(y < pattern_edit->control.y1_offset){
      if(y + pattern_edit->control_height - pattern_edit->control_margin_y < pattern_edit->control.y1_offset){
	return;
      }else{
	if(y + pattern_edit->control_margin_y < pattern_edit->control.y1_offset){
	  height = pattern_edit->control_height;
	  y = y + pattern_edit->control_margin_y - pattern_edit->control.y1_offset;
	}else{
	  height = pattern_edit->y0;
	  y -= pattern_edit->control.y1_offset;
	}
      }
    }else if(y < pattern_edit->control.y1_offset + widget->allocation.height - pattern_edit->control_height){
      height = pattern_edit->control_height - 2 * pattern_edit->control_margin_y;
      y = y - pattern_edit->control.y1_offset + pattern_edit->control_margin_y;
    }else{
      if(y > pattern_edit->control.y1_offset + widget->allocation.height - pattern_edit->y1 + pattern_edit->control_margin_y){
	return;
      }else{
	height = pattern_edit->y0;
	y = y - pattern_edit->control.y1_offset + pattern_edit->control_margin_y;
      }
    }

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
    cairo_fill(cr);
  }
  void ags_pattern_edit_drawing_area_button_release_event_delete_point(){
    GList *list_notation;
    guint x, y;
    gint history;
    gint i;

    x = pattern_edit->control.x0_offset + pattern_edit->control.x0 - 1;
    y = pattern_edit->control.y0_offset + pattern_edit->control.y0;

    x = (guint) ceil((double) x / (double) (pattern_edit->control_unit.control_width));
    y = (guint) floor((double) y / (double) (pattern_edit->control_height));

    g_message("%d, %d\0", x, y);

    /* select notes */
    list_notation = machine->audio->notation;
    i = 0;

    while((i = ags_notebook_next_active_tab(editor->current_notebook,
					    i)) != -1){
      list_notation = g_list_nth(machine->audio->notation,
				 i);
      
      if(list_notation == NULL){
	i++;
	
	continue;
      }

      ags_notation_remove_note_at_position(AGS_NOTATION(list_notation->data),
					   x, y);

      i++;
    }
  }
  void ags_pattern_edit_drawing_area_button_release_event_select_region(){
    GList *list_notation;
    guint x0, x1, y0, y1;
    gint i;
    
    /* get real size and offset */
    x0 = pattern_edit->control.x0_offset + pattern_edit->control.x0;
    x1 = pattern_edit->control.x1_offset + pattern_edit->control.x1;

    if(x0 > x1){
      guint tmp;

      tmp = x1;
      x1 = x0;
      x0 = tmp;
    }

    /* convert to region */
    x0 = (guint) (floor((double) x0 / (double) (pattern_edit->control_current.control_width)) * tact);
    x1 = (guint) (ceil((double) x1 / (double) (pattern_edit->control_current.control_width)) * tact);

    /* get real size and offset */
    y0 = pattern_edit->control.y0_offset + pattern_edit->control.y0;
    y1 = pattern_edit->control.y1_offset + pattern_edit->control.y1;

    if(y0 > y1){
      guint tmp;

      tmp = y1;
      y1 = y0;
      y0 = tmp;
    }

    /* convert to region */
    y0 = (guint) floor((double) y0 / (double) (pattern_edit->control_height));
    y1 = (guint) ceil((double) y1 / (double) (pattern_edit->control_height));

    /* select notes */
    list_notation = machine->audio->notation;

    while((i = ags_notebook_next_active_tab(editor->current_notebook,
					    i)) != -1){
      list_notation = g_list_nth(machine->audio->notation,
				 i);

      if(list_notation == NULL){
	i++;
	
	continue;
      }

      ags_notation_add_region_to_selection(AGS_NOTATION(list_notation->data),
					   x0, y0,
					   x1, y1,
					   TRUE);

      i++;
    }
  }

  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);

  if(editor->selected_machine != NULL && event->button == 1){
    AgsMutexManager *mutex_manager;

    cairo_t *cr;

    pthread_mutex_t *audio_mutex;

    pthread_mutex_lock(&(ags_application_mutex));
  
    mutex_manager = ags_mutex_manager_get_instance();

    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) editor->selected_machine->audio);
  
    pthread_mutex_unlock(&(ags_application_mutex));

    if(event->x >= 0.0){
      pattern_edit->control.x1 = (guint) event->x;
    }else{
      pattern_edit->control.x1 = 0;
    }

    if(event->y >= 0.0){
      pattern_edit->control.y1 = (guint) event->y;
    }else{
      pattern_edit->control.y1 = 0;
    }
    
    machine = editor->selected_machine;
    note = pattern_edit->control.note;

    /* store the events position */
    pattern_edit->control.x1_offset = (guint) round((double) pattern_edit->hscrollbar->scrollbar.range.adjustment->value);
    pattern_edit->control.y1_offset = (guint) round((double) pattern_edit->vscrollbar->scrollbar.range.adjustment->value);

    tact = exp2(6.0 - (double) gtk_combo_box_get_active(editor->toolbar->zoom));

    cr = gdk_cairo_create(widget->window);
    cairo_push_group(cr);

    if((AGS_PATTERN_EDIT_POSITION_CURSOR & (pattern_edit->flags)) != 0){
      pattern_edit->flags &= (~AGS_PATTERN_EDIT_POSITION_CURSOR);

      ags_pattern_edit_draw_segment(pattern_edit, cr);
      ags_pattern_edit_draw_notation(pattern_edit, cr);

      pattern_edit->selected_x = pattern_edit->control.note->x[0];
      pattern_edit->selected_y = pattern_edit->control.note->y;

      if(AGS_IS_PANEL(machine)){
      }else if(AGS_IS_MIXER(machine)){
      }else if(AGS_IS_DRUM(machine)){
	ags_pattern_edit_draw_position(pattern_edit, cr);
      }else if(AGS_IS_MATRIX(machine)){
	ags_pattern_edit_draw_position(pattern_edit, cr);
      }else if(AGS_IS_FFPLAYER(machine)){
	ags_pattern_edit_draw_position(pattern_edit, cr);
      }else if(AGS_IS_SYNTH(machine)){
	ags_pattern_edit_draw_position(pattern_edit, cr);
      }
    }else if((AGS_PATTERN_EDIT_ADDING_NOTE & (pattern_edit->flags)) != 0){
      pattern_edit->flags &= (~AGS_PATTERN_EDIT_ADDING_NOTE);

      ags_pattern_edit_draw_segment(pattern_edit, cr);
      ags_pattern_edit_draw_notation(pattern_edit, cr);

      pthread_mutex_lock(audio_mutex);

      if(AGS_IS_PANEL(machine)){
      }else if(AGS_IS_MIXER(machine)){
      }else if(AGS_IS_DRUM(machine)){
	ags_pattern_edit_drawing_area_button_release_event_set_control();
      }else if(AGS_IS_MATRIX(machine)){
	ags_pattern_edit_drawing_area_button_release_event_set_control();
      }else if(AGS_IS_FFPLAYER(machine)){
	ags_pattern_edit_drawing_area_button_release_event_set_control();
      }else if(AGS_IS_SYNTH(machine)){
	ags_pattern_edit_drawing_area_button_release_event_set_control();
      }

      pthread_mutex_unlock(audio_mutex);

      if(AGS_IS_PANEL(machine)){
      }else if(AGS_IS_MIXER(machine)){
      }else if(AGS_IS_DRUM(machine)){
	ags_pattern_edit_drawing_area_button_release_event_draw_control(cr);
      }else if(AGS_IS_MATRIX(machine)){
	ags_pattern_edit_drawing_area_button_release_event_draw_control(cr);
      }else if(AGS_IS_FFPLAYER(machine)){
	ags_pattern_edit_drawing_area_button_release_event_draw_control(cr);
      }else if(AGS_IS_SYNTH(machine)){
	ags_pattern_edit_drawing_area_button_release_event_draw_control(cr);
      }
    }else if((AGS_PATTERN_EDIT_DELETING_NOTE & (pattern_edit->flags)) != 0){
      pattern_edit->flags &= (~AGS_PATTERN_EDIT_DELETING_NOTE);

      pthread_mutex_lock(audio_mutex);

      ags_pattern_edit_drawing_area_button_release_event_delete_point();

      pthread_mutex_unlock(audio_mutex);

      ags_pattern_edit_draw_segment(pattern_edit, cr);
      ags_pattern_edit_draw_notation(pattern_edit, cr);
    }else if((AGS_PATTERN_EDIT_SELECTING_NOTES & (pattern_edit->flags)) != 0){
      pattern_edit->flags &= (~AGS_PATTERN_EDIT_SELECTING_NOTES);

      //      pthread_mutex_lock(audio_mutex);
      
      ags_pattern_edit_drawing_area_button_release_event_select_region();

      //      pthread_mutex_unlock(audio_mutex);

      ags_pattern_edit_draw_segment(pattern_edit, cr);
      ags_pattern_edit_draw_notation(pattern_edit, cr);
    }

    cairo_pop_group_to_source(cr);
    cairo_paint(cr);
  }

  return(FALSE);
}

gboolean
ags_pattern_edit_drawing_area_motion_notify_event (GtkWidget *widget, GdkEventMotion *event, AgsPatternEdit *pattern_edit)
{
  AgsMachine *machine;
  AgsEditor *editor;
  AgsNote *note, *note0;
  double value[2];
  double tact;
  guint note_x1;
  guint prev_x1;
  void ags_pattern_edit_drawing_area_motion_notify_event_set_control(){
    GList *list_notation;
    guint note_x, note_y;
    guint note_offset_x1;

    if(pattern_edit->control.x0 >= pattern_edit->map_width)
      pattern_edit->control.x0 = pattern_edit->map_width - 1;

    note_offset_x1 = (guint) (ceil(pattern_edit->control.x1_offset / (double) (pattern_edit->control_current.control_width)));

    if(pattern_edit->control.x1 >= pattern_edit->control_current.x0)
      note_x = (guint) (ceil((double) (pattern_edit->control.x1 - pattern_edit->control_current.x0) / (double) (pattern_edit->control_current.control_width)));
    else{
      note_offset_x1 -= 1;
      note_x = 0;
    }

    note_x1 = (note_x * tact) + (note_offset_x1 * tact);

    list_notation = machine->audio->notation;

    fprintf(stdout, "x0 = %llu\nx1 = %llu\ny  = %llu\n\n\0", (long long unsigned int) note->x[0], (long long unsigned int) note->x[1], (long long unsigned int) note->y);
  }
  void ags_pattern_edit_drawing_area_motion_notify_event_draw_control(cairo_t *cr){
    guint x, y, width, height;

    widget = (GtkWidget *) pattern_edit->drawing_area;

    x = note->x[0] * pattern_edit->control_unit.control_width;
    width = note_x1 * pattern_edit->control_unit.control_width;

    if(x < pattern_edit->control.x1_offset){
      if(width > pattern_edit->control.x1_offset){
	width -= x;
	x = 0;
      }else{
	return;
      }
    }else if(x < pattern_edit->control.x1_offset + widget->allocation.width){
      width -= x;
      x -= pattern_edit->control.x1_offset;
    }else{
      return;
    }

    width = pattern_edit->control_unit.control_width;

    if(x + width > widget->allocation.width)
      width = widget->allocation.width - x;

    y = note->y * pattern_edit->control_height;

    if(y < pattern_edit->control.y1_offset){
      if(y + pattern_edit->control_height - pattern_edit->control_margin_y < pattern_edit->control.y1_offset){
	return;
      }else{
	if(y + pattern_edit->control_margin_y < pattern_edit->control.y1_offset){
	  height = pattern_edit->control_height;
	  y = y + pattern_edit->control_margin_y - pattern_edit->control.y1_offset;
	}else{
	  height = pattern_edit->y0;
	  y -= pattern_edit->control.y1_offset;
	}
      }
    }else if(y < pattern_edit->control.y1_offset + widget->allocation.height - pattern_edit->control_height){
      height = pattern_edit->control_height - 2 * pattern_edit->control_margin_y;
      y = y - pattern_edit->control.y1_offset + pattern_edit->control_margin_y;
    }else{
      if(y > pattern_edit->control.y1_offset + widget->allocation.height - pattern_edit->y1 + pattern_edit->control_margin_y){
	return;
      }else{
	height = pattern_edit->y0;
	y = y - pattern_edit->control.y1_offset + pattern_edit->control_margin_y;
      }
    }

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_rectangle(cr, (double) x, (double) y, (double) width, (double) height);
    cairo_fill(cr);
  }
  void ags_pattern_edit_drawing_area_motion_notify_event_draw_selection(cairo_t *cr){
    GtkAllocation allocation;
    guint x0_offset, x1_offset, y0_offset, y1_offset;
    guint x0, x1, y0, y1, width, height;
    guint x0_viewport, x1_viewport, y0_viewport, y1_viewport;

    /* get viewport */
    gtk_widget_get_allocation(widget, &allocation);

    x0_viewport = pattern_edit->control.x1_offset;
    x1_viewport = pattern_edit->control.x1_offset + allocation.width;

    y0_viewport = pattern_edit->control.y1_offset;
    y1_viewport = pattern_edit->control.y1_offset + allocation.height;

    /* get real size and offset */
    x0 = pattern_edit->control.x0_offset + pattern_edit->control.x0;
    x1 = pattern_edit->control.x1_offset + pattern_edit->control.x1;

    if(x0 > x1){
      x0_offset = x1;
      x1_offset = x0;

      x1 = x0_offset;
      x0 = x1_offset;
    }else{
      x0_offset = x0;
      x1_offset = x1;
    }

    /* get drawable size and offset */
    if(x0 < x0_viewport){
      //      x0 = 0;
      //      width = x1_offset - x0_viewport;
      x0 -= x0_viewport;
      width = x1 - x0;
    }else{
      x0 -= x0_viewport;
      width = x1 - x0;
    }

    if(x1 > x1_viewport){
      width -= (x1 - x1_viewport);
    }else{
      width -= x0_viewport;
    }

    /* get real size and offset */
    y0 = pattern_edit->control.y0_offset + pattern_edit->control.y0;
    y1 = pattern_edit->control.y1_offset + pattern_edit->control.y1;

    if(y0 > y1){
      y0_offset = y1;
      y1_offset = y0;

      y1 = y0_offset;
      y0 = y1_offset;
    }else{
      y0_offset = y0;
      y1_offset = y1;
    }

    /* get drawable size and offset */
    if(y0 < y0_viewport){
      y0 = 0;
      height = y1_offset - y0_viewport;
    }else{
      y0 -= y0_viewport;
      height = y1 - y0;
    }

    if(y1 > y1_viewport){
      height -= (y1 - y1_viewport);
    }else{
      height -= y0_viewport;
    }

    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.3);
    cairo_rectangle(cr, (double) x0, (double) y0, (double) width, (double) height);
    cairo_fill(cr);
  }

  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);

  if(editor->selected_machine != NULL){
    cairo_t *cr;

    prev_x1 = pattern_edit->control.x1;

    if(event->x >= 0.0){
      pattern_edit->control.x1 = (guint) event->x;
    }else{
      pattern_edit->control.x1 = 0;
    }

    if(event->y >= 0.0){
      pattern_edit->control.y1 = (guint) event->y;
    }else{
      pattern_edit->control.y1 = 0;
    }

    machine = editor->selected_machine;
    note = pattern_edit->control.note;

    pattern_edit->control.x1_offset = (guint) round((double) pattern_edit->hscrollbar->scrollbar.range.adjustment->value);
    pattern_edit->control.y1_offset = (guint) round((double) pattern_edit->vscrollbar->scrollbar.range.adjustment->value);

    tact = exp2(6.0 - (double) gtk_combo_box_get_active(editor->toolbar->zoom));

    cr = gdk_cairo_create(widget->window);
    cairo_push_group(cr);

    if((AGS_PATTERN_EDIT_ADDING_NOTE & (pattern_edit->flags)) != 0){
      if(prev_x1 > pattern_edit->control.x1){
	ags_pattern_edit_draw_segment(pattern_edit, cr);
	ags_pattern_edit_draw_notation(pattern_edit, cr);
      }

      if(AGS_IS_PANEL(machine)){
      }else if(AGS_IS_MIXER(machine)){
      }else if(AGS_IS_DRUM(machine)){
	ags_pattern_edit_drawing_area_motion_notify_event_set_control();
	ags_pattern_edit_drawing_area_motion_notify_event_draw_control(cr);
      }else if(AGS_IS_MATRIX(machine)){
	ags_pattern_edit_drawing_area_motion_notify_event_set_control();
	ags_pattern_edit_drawing_area_motion_notify_event_draw_control(cr);
      }else if(AGS_IS_FFPLAYER(machine)){
	ags_pattern_edit_drawing_area_motion_notify_event_set_control();
	ags_pattern_edit_drawing_area_motion_notify_event_draw_control(cr);
      }
    }else if((AGS_PATTERN_EDIT_SELECTING_NOTES & (pattern_edit->flags)) != 0){
      ags_pattern_edit_draw_segment(pattern_edit, cr);
      ags_pattern_edit_draw_notation(pattern_edit, cr);

      ags_pattern_edit_drawing_area_motion_notify_event_draw_selection(cr);
    }
    
    cairo_pop_group_to_source(cr);
    cairo_paint(cr);
  }

  return(FALSE);
}

gboolean
ags_pattern_edit_drawing_area_key_press_event(GtkWidget *widget, GdkEventKey *event, AgsPatternEdit *pattern_edit)
{
  AgsEditor *editor;

  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);

  if(editor->selected_machine != NULL){

    switch(event->keyval){
    case GDK_KEY_Control_L:
      {
	pattern_edit->key_mask |= AGS_PATTERN_EDIT_KEY_L_CONTROL;
      }
      break;
    case GDK_KEY_Control_R:
      {
	pattern_edit->key_mask |= AGS_PATTERN_EDIT_KEY_R_CONTROL;
      }
      break;
    case GDK_KEY_a:
      {
	/* select all patterns */
	if((AGS_PATTERN_EDIT_KEY_L_CONTROL & (pattern_edit->key_mask)) != 0 || (AGS_PATTERN_EDIT_KEY_R_CONTROL & (pattern_edit->key_mask)) != 0){
	  ags_editor_select_all(editor);
	}
      }
      break;
    case GDK_KEY_c:
      {
	/* copy patterns */
	if((AGS_PATTERN_EDIT_KEY_L_CONTROL & (pattern_edit->key_mask)) != 0 || (AGS_PATTERN_EDIT_KEY_R_CONTROL & (pattern_edit->key_mask)) != 0){
	  ags_editor_copy(editor);
	}
      }
      break;
    case GDK_KEY_v:
      {
	/* paste patterns */
	if((AGS_PATTERN_EDIT_KEY_L_CONTROL & (pattern_edit->key_mask)) != 0 || (AGS_PATTERN_EDIT_KEY_R_CONTROL & (pattern_edit->key_mask)) != 0){
	  ags_editor_paste(editor);
	}
      }
      break;
    case GDK_KEY_x:
      {
	/* cut patterns */
	if((AGS_PATTERN_EDIT_KEY_L_CONTROL & (pattern_edit->key_mask)) != 0 || (AGS_PATTERN_EDIT_KEY_R_CONTROL & (pattern_edit->key_mask)) != 0){
	  ags_editor_cut(editor);
	}
      }
      break;
    case GDK_KEY_i:
      {
	/* cut patterns */
	if((AGS_PATTERN_EDIT_KEY_L_CONTROL & (pattern_edit->key_mask)) != 0 || (AGS_PATTERN_EDIT_KEY_R_CONTROL & (pattern_edit->key_mask)) != 0){
	  ags_editor_invert(editor);
	}
      }
      break;
    }
  }
}

gboolean
ags_pattern_edit_drawing_area_key_release_event(GtkWidget *widget, GdkEventKey *event, AgsPatternEdit *pattern_edit)
{
  switch(event->keyval){
  case GDK_KEY_Control_L:
    {
      pattern_edit->key_mask &= (~AGS_PATTERN_EDIT_KEY_L_CONTROL);
    }
    break;
  case GDK_KEY_Control_R:
    {
      pattern_edit->key_mask &= (~AGS_PATTERN_EDIT_KEY_R_CONTROL);
    }
    break;
  }
}

void
ags_pattern_edit_vscrollbar_value_changed(GtkRange *range, AgsPatternEdit *pattern_edit)
{
  AgsEditor *editor;

  if((AGS_PATTERN_EDIT_RESETING_VERTICALLY & pattern_edit->flags) != 0){
    return;
  }

  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);

  ags_meter_paint(editor->current_meter);
  
  pattern_edit->flags |= AGS_PATTERN_EDIT_RESETING_VERTICALLY;
  ags_pattern_edit_reset_vertically(pattern_edit, 0);
  pattern_edit->flags &= (~AGS_PATTERN_EDIT_RESETING_VERTICALLY);
}

void
ags_pattern_edit_hscrollbar_value_changed(GtkRange *range, AgsPatternEdit *pattern_edit)
{
  if((AGS_PATTERN_EDIT_RESETING_HORIZONTALLY & pattern_edit->flags) != 0){
    return;
  }

  /* reset ruler */
  gtk_adjustment_set_value(pattern_edit->ruler->adjustment,
			   GTK_RANGE(pattern_edit->hscrollbar)->adjustment->value / (double) pattern_edit->control_current.control_width);
  gtk_widget_queue_draw(pattern_edit->ruler);

  /* update pattern edit */
  pattern_edit->flags |= AGS_PATTERN_EDIT_RESETING_HORIZONTALLY;
  ags_pattern_edit_reset_horizontally(pattern_edit, 0);
  pattern_edit->flags &= (~AGS_PATTERN_EDIT_RESETING_HORIZONTALLY);
}