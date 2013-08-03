/* AGS Client - Advanced GTK Sequencer Client
 * Copyright (C) 2013 Joël Krähemann
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

#include <ags-client/scripting/mapping/ags_script_var.h>

#include <ags-lib/object/ags_connectable.h>

void ags_script_var_class_init(AgsScriptVarClass *script_var);
void ags_script_var_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_script_var_init(AgsScriptVar *script_var);
void ags_script_var_connect(AgsConnectable *connectable);
void ags_script_var_disconnect(AgsConnectable *connectable);
void ags_script_var_finalize(GObject *gobject);

static gpointer ags_script_var_parent_class = NULL;

GType
ags_script_var_get_type()
{
  static GType ags_type_script_var = 0;

  if(!ags_type_script_var){
    static const GTypeInfo ags_script_var_info = {
      sizeof (AgsScriptVarClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_script_var_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsScriptVar),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_script_var_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_script_var_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_script_var = g_type_register_static(AGS_TYPE_SCRIPT_OBJECT,
						 "AgsScriptVar\0",
						 &ags_script_var_info,
						 0);
    
    g_type_add_interface_static(ags_type_script_var,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return (ags_type_script_var);
}

void
ags_script_var_class_init(AgsScriptVarClass *script_var)
{
  GObjectClass *gobject;

  ags_script_var_parent_class = g_type_class_peek_parent(script_var);

  /* GObjectClass */
  gobject = (GObjectClass *) script_var;

  gobject->finalize = ags_script_var_finalize;
}

void
ags_script_var_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->connect = ags_script_var_connect;
  connectable->disconnect = ags_script_var_disconnect;
}

void
ags_script_var_init(AgsScriptVar *script_var)
{
  script_var->flags = 0;
}

void
ags_script_var_connect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_script_var_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_script_var_finalize(GObject *gobject)
{
  AgsScriptVar *script_var;

  script_var = AGS_SCRIPT_VAR(gobject);

  G_OBJECT_CLASS(ags_script_var_parent_class)->finalize(gobject);
}

AgsScriptVar*
ags_script_var_new()
{
  AgsScriptVar *script_var;

  script_var = (AgsScriptVar *) g_object_new(AGS_TYPE_SCRIPT_VAR,
					     NULL);

  return(script_var);
}
