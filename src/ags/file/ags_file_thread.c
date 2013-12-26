/* AGS - Advanced GTK Sequencer
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

#include <ags/file/ags_file_thread.h>

#include <ags/util/ags_id_generator.h>

#include <ags/file/ags_file_stock.h>
#include <ags/file/ags_file_id_ref.h>

void
ags_file_read_thread(AgsFile *file, xmlNode *node, AgsThread **thread)
{
  AgsThread *gobject;
  xmlNode *child;
  xmlChar *type_name;
  static gboolean thread_type_is_registered = FALSE;

  if(*thread == NULL){
    GType type;

    if(!thread_type_is_registered){
      ags_main_register_thread_type();

      thread_type_is_registered = TRUE;
    }

    type_name = xmlGetProp(node,
			   AGS_FILE_TYPE_PROP);
    g_message(type_name);

    type = g_type_from_name(type_name);

    gobject = g_object_new(type,
			   NULL);

    *thread = gobject;
  }else{
    gobject = *thread;
  }

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "main\0", file->ags_main,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node,
						       AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);

  /* read children */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-thread-list\0",
		     16)){
	GList *list;

	list = NULL;

	ags_file_read_thread_list(file,
				  child,
				  &list);

	while(list != NULL){
	  ags_thread_add_child(gobject,
			       list->data);

	  list = list->next;
	}
      }else if(!xmlStrncmp(child->name,
			   "ags-audio-loop\0",
			   15)){
	ags_file_read_audio_loop(file,
				 child,
				 AGS_AUDIO_LOOP(gobject));
      }
    }

    child = child->next;
  }

  /* start thread */
  if((AGS_THREAD_RUNNING & (gobject->flags)) != 0){
    gobject->flags &= (~AGS_THREAD_RUNNING);
    ags_thread_start(gobject);
  }
}

xmlNode*
ags_file_write_thread(AgsFile *file, xmlNode *parent, AgsThread *thread)
{
  AgsThread *current;
  xmlNode *node, *child;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-thread\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);
 
  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "main\0", file->ags_main,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//[@id='%s']\0", id),
				   "reference\0", thread,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_TYPE_PROP,
	     G_OBJECT_TYPE_NAME(thread));

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", thread->flags));

  xmlAddChild(parent,
	      node);

  /* child elements */
  if(AGS_IS_AUDIO_LOOP(thread)){
    ags_file_write_audio_loop(file,
			      node,
			      AGS_AUDIO_LOOP(thread));
  }

  current = thread->children;

  child = xmlNewNode(NULL,
		     "ags-thread-list\0");
  xmlAddChild(node,
	      child);

  while(current != NULL){
    ags_file_write_thread(file,
			  child,
			  current);
    current = current->next;
  }
}

void
ags_file_read_thread_list(AgsFile *file, xmlNode *node, GList **thread)
{
  AgsThread *current;
  GList *list;
  xmlNode *child;
  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-thread\0",
		     11)){
	current = NULL;
	ags_file_read_thread(file, child, &current);
	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *thread = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "main\0", file->ags_main,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_thread_list(AgsFile *file, xmlNode *parent, GList *thread)
{
  AgsThread *current;
  xmlNode *node;
  GList *list;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-thread-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "main\0", file->ags_main,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  //TODO:JK: generate id and add id ref

  list = thread;

  while(list != NULL){
    ags_file_write_thread(file, node, AGS_THREAD(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_thread_pool(AgsFile *file, xmlNode *node, AgsThreadPool **thread_pool)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_thread_pool(AgsFile *file, xmlNode *parent, AgsThreadPool *thread_pool)
{
  //TODO:JK: implement me
}

void
ags_file_read_audio_loop(AgsFile *file, xmlNode *node, AgsAudioLoop *audio_loop)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_audio_loop(AgsFile *file, xmlNode *parent, AgsAudioLoop *audio_loop)
{
  //TODO:JK: implement me
}

