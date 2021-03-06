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

#include <ags/X/editor/ags_pattern_edit_callbacks.h>

#include <ags/X/ags_editor.h>

#include <ags/X/machine/ags_panel.h>
#include <ags/X/machine/ags_mixer.h>
#include <ags/X/machine/ags_drum.h>
#include <ags/X/machine/ags_matrix.h>
#include <ags/X/machine/ags_synth.h>
#include <ags/X/machine/ags_ffplayer.h>

gboolean
ags_pattern_edit_drawing_area_expose_event(GtkWidget *widget, GdkEventExpose *event, AgsPatternEdit *pattern_edit)
{
  AgsEditor *editor;

  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);

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
      tact = exp2(8.0 - (double) gtk_combo_box_get_active(editor->toolbar->zoom));
      
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
    guint note_x, note_y;
    guint note_offset_x1;
    gint history;
    gint selected_channel;

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
    history = gtk_combo_box_get_active(editor->toolbar->mode);

    switch(history){
    case 0:
      {
	if(editor->notebook->tabs != NULL){
	  list_notation = g_list_nth(list_notation,
				     ags_notebook_next_active_tab(editor->notebook,
								  0));

	  note0 = ags_note_duplicate(note);

	  ags_notation_add_note(AGS_NOTATION(list_notation->data), note0, FALSE);
	}
      }
      break;
    case 1:
      {
	gint i;

	i = 0;

	while((selected_channel = ags_notebook_next_active_tab(editor->notebook,
							       i)) != -1){
	  list_notation = g_list_nth(machine->audio->notation,
				     selected_channel);

	  note0 = ags_note_duplicate(note);

	  ags_notation_add_note(AGS_NOTATION(list_notation->data), note0, FALSE);

	  i++;
	}
      }
      break;
    case 2:
      {
	while(list_notation != NULL){
	  note0 = ags_note_duplicate(note);

	  ags_notation_add_note(AGS_NOTATION(list_notation->data), note0, FALSE);

	  list_notation = list_notation->next;
	}
      }
      break;
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
    gint selected_channel;

    x = pattern_edit->control.x0_offset + pattern_edit->control.x0 - 1;
    y = pattern_edit->control.y0_offset + pattern_edit->control.y0;

    x = (guint) ceil((double) x / (double) (pattern_edit->control_unit.control_width));
    y = (guint) floor((double) y / (double) (pattern_edit->control_height));

    g_message("%d, %d\0", x, y);

    /* select notes */
    list_notation = machine->audio->notation;

    history = gtk_combo_box_get_active(editor->toolbar->mode);

    if(history == 0){
      if(editor->notebook->tabs != NULL){
	list_notation = g_list_nth(list_notation,
				   ags_notebook_next_active_tab(editor->notebook,
								0));

	ags_notation_remove_note_at_position(AGS_NOTATION(list_notation->data),
					     x, y);
      }
    }else if(history == 1){
      gint i;

      i = 0;

      while((selected_channel = ags_notebook_next_active_tab(editor->notebook,
							     i)) != -1){
	list_notation = g_list_nth(machine->audio->notation,
				   selected_channel);

	ags_notation_remove_note_at_position(AGS_NOTATION(list_notation->data),
					     x, y);

	list_notation = list_notation->next;
	i++;
      }
    }else if(history == 2){
      while(list_notation != NULL){
	ags_notation_remove_note_at_position(AGS_NOTATION(list_notation->data),
					     x, y);

	list_notation = list_notation->next;
      }
    }
  }
  void ags_pattern_edit_drawing_area_button_release_event_select_region(){
    GList *list_notation;
    guint x0, x1, y0, y1;

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

    if(gtk_combo_box_get_active(editor->toolbar->mode) == 0){
      if(editor->notebook->tabs != NULL){
	list_notation = g_list_nth(list_notation,
				   ags_notebook_next_active_tab(editor->notebook,
				   0));

	ags_notation_add_region_to_selection(AGS_NOTATION(list_notation->data),
					     x0, y0,
					     x1, y1,
					     TRUE);
      }
    }else{
      while(list_notation != NULL ){
	ags_notation_add_region_to_selection(AGS_NOTATION(list_notation->data),
					     x0, y0,
					     x1, y1,
					     TRUE);

	list_notation = list_notation->next;
      }
    }

  }

  editor = (AgsEditor *) gtk_widget_get_ancestor(GTK_WIDGET(pattern_edit),
						 AGS_TYPE_EDITOR);

  if(editor->selected_machine != NULL && event->button == 1){
    cairo_t *cr;

    pattern_edit->control.x1 = (guint) event->x;
    pattern_edit->control.y1 = (guint) event->y;

    machine = editor->selected_machine;
    note = pattern_edit->control.note;

    /* store the events position */
    pattern_edit->control.x1_offset = (guint) round((double) pattern_edit->hscrollbar->scrollbar.range.adjustment->value);
    pattern_edit->control.y1_offset = (guint) round((double) pattern_edit->vscrollbar->scrollbar.range.adjustment->value);

    tact = exp2(8.0 - (double) gtk_combo_box_get_active(editor->toolbar->zoom));

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

      if(AGS_IS_PANEL(machine)){
      }else if(AGS_IS_MIXER(machine)){
      }else if(AGS_IS_DRUM(machine)){
	ags_pattern_edit_drawing_area_button_release_event_set_control();
	ags_pattern_edit_drawing_area_button_release_event_draw_control(cr);
      }else if(AGS_IS_MATRIX(machine)){
	ags_pattern_edit_drawing_area_button_release_event_set_control();
	ags_pattern_edit_drawing_area_button_release_event_draw_control(cr);
      }else if(AGS_IS_FFPLAYER(machine)){
	ags_pattern_edit_drawing_area_button_release_event_set_control();
	ags_pattern_edit_drawing_area_button_release_event_draw_control(cr);
      }else if(AGS_IS_SYNTH(machine)){
	ags_pattern_edit_drawing_area_button_release_event_set_control();
	ags_pattern_edit_drawing_area_button_release_event_draw_control(cr);
      }
    }else if((AGS_PATTERN_EDIT_DELETING_NOTE & (pattern_edit->flags)) != 0){
      pattern_edit->flags &= (~AGS_PATTERN_EDIT_DELETING_NOTE);

      ags_pattern_edit_drawing_area_button_release_event_delete_point();

      ags_pattern_edit_draw_segment(pattern_edit, cr);
      ags_pattern_edit_draw_notation(pattern_edit, cr);
    }else if((AGS_PATTERN_EDIT_SELECTING_NOTES & (pattern_edit->flags)) != 0){
      pattern_edit->flags &= (~AGS_PATTERN_EDIT_SELECTING_NOTES);

      ags_pattern_edit_drawing_area_button_release_event_select_region();

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
    pattern_edit->control.x1 = (guint) event->x;
    pattern_edit->control.y1 = (guint) event->y;

    machine = editor->selected_machine;
    note = pattern_edit->control.note;

    pattern_edit->control.x1_offset = (guint) round((double) pattern_edit->hscrollbar->scrollbar.range.adjustment->value);
    pattern_edit->control.y1_offset = (guint) round((double) pattern_edit->vscrollbar->scrollbar.range.adjustment->value);

    tact = exp2(8.0 - (double) gtk_combo_box_get_active(editor->toolbar->zoom));

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

void
ags_pattern_edit_vscrollbar_value_changed(GtkRange *range, AgsPatternEdit *pattern_edit)
{
  if((AGS_PATTERN_EDIT_RESETING_VERTICALLY & pattern_edit->flags) != 0){
    return;
  }

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

  /* update note edit */
  pattern_edit->flags |= AGS_PATTERN_EDIT_RESETING_HORIZONTALLY;
  ags_pattern_edit_reset_horizontally(pattern_edit, 0);
  pattern_edit->flags &= (~AGS_PATTERN_EDIT_RESETING_HORIZONTALLY);
}
