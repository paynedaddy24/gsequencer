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

#include <ags/plugin/ags_turtle.h>

void ags_turtle_class_init(AgsTurtleClass *turtle);
void ags_turtle_init (AgsTurtle *turtle);
void ags_turtle_finalize(GObject *gobject);

GType
ags_turtle_get_type(void)
{
  static GType ags_type_turtle = 0;

  if(!ags_type_turtle){
    static const GTypeInfo ags_turtle_info = {
      sizeof (AgsTurtleClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_turtle_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsTurtle),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_turtle_init,
    };

    ags_type_turtle = g_type_register_static(G_TYPE_OBJECT,
					     "AgsTurtle\0",
					     &ags_turtle_info,
					     0);
  }

  return (ags_type_turtle);
}

void
ags_turtle_class_init(AgsTurtleClass *turtle)
{
  GObjectClass *gobject;

  ags_turtle_parent_class = g_type_class_peek_parent(turtle);

  /* GObjectClass */
  gobject = (GObjectClass *) turtle;

  gobject->finalize = ags_turtle_finalize;
}

void
ags_turtle_init(AgsTurtle *turtle)
{
  turtle->locale = NULL;
  turtle->hashmap = g_hash_table_new_full(g_str_hash, (GEqualFunc) g_ascii_strcasecmp,
					  (GDestroyNotify) g_free,
					  (GDestroyNotify) g_free);
}

void
ags_turtle_finalize(GObject *gobject)
{
  AgsTurtle *turtle;

  turtle = AGS_TURTLE(gobject);

  g_free(turtle->locale);
  g_hash_table_destroy(turtle->hash_table);
}

gboolean
ags_turtle_insert(AgsTurtle *turtle,
		  gchar *key, gchar *value)
{
  return(g_hash_table_insert(turtle->hash_table,
			     key, value));
}

gboolean
ags_turtle_remove(AgsTurtle *turtle,
		  gchar *key)
{
  g_hash_table_remove(turtle->hash_table,
		      key);
  
  return(TRUE);
}

gchar*
ags_turtle_lookup(AgsTurtle *turtle,
		  gchar *key)
{
  gchar *value;

  value = (gchar *) g_hash_table_lookup(turtle->hash_table,
					key);

  return(value);
}

gchar**
ags_turtle_value_as_array(AgsTurtle,
			  gchar *value)
{
  //TODO:JK: implement me

  return(NULL);
}

void
ags_turtle_load(AgsTurtle *turtle)
{
  //TODO:JK: implement me
}

AgsTurtle*
ags_turtle_new(gchar *filename,
	       gchar locale)
{
  AgsTurtle *turtle;

  turtle = g_object_new(AGS_TYPE_TURTLE,
			"filename\0", filename,
			"locale\0", locale,
			NULL);

  return(turtle);
}
