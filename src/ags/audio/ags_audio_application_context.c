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

#include <ags/audio/ags_audio_application_context.h>

#include <ags/object/ags_config.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/file/ags_file.h>
#include <ags/file/ags_file_stock.h>
#include <ags/file/ags_file_id_ref.h>

#include <ags/audio/ags_devout.h>

#include <ags/audio/file/ags_audio_file_xml.h>

void ags_audio_application_context_class_init(AgsAudioApplicationContextClass *audio_application_context);
void ags_audio_application_context_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_audio_application_context_init(AgsAudioApplicationContext *audio_application_context);
void ags_audio_application_context_set_property(GObject *gobject,
						guint prop_id,
						const GValue *value,
						GParamSpec *param_spec);
void ags_audio_application_context_get_property(GObject *gobject,
						guint prop_id,
						GValue *value,
						GParamSpec *param_spec);
void ags_audio_application_context_connect(AgsConnectable *connectable);
void ags_audio_application_context_disconnect(AgsConnectable *connectable);
void ags_audio_application_context_finalize(GObject *gobject);

void ags_audio_application_context_load_config(AgsApplicationContext *application_context);
void ags_audio_application_context_register_types(AgsApplicationContext *application_context);
void ags_audio_application_context_read(AgsFile *file, xmlNode *node, GObject **application_context);
xmlNode* ags_audio_application_context_write(AgsFile *file, xmlNode *parent, GObject *application_context);

void ags_audio_application_context_set_value_callback(AgsConfig *config, gchar *group, gchar *key, gchar *value,
						      AgsAudioApplicationContext *audio_application_context);

static gpointer ags_audio_application_context_parent_class = NULL;
static AgsConnectableInterface* ags_audio_application_context_parent_connectable_interface;

enum{
  PROP_0,
  PROP_SOUNDCARD,
};

GType
ags_audio_application_context_get_type()
{
  static GType ags_type_audio_application_context = 0;

  if(!ags_type_audio_application_context){
    static const GTypeInfo ags_audio_application_context_info = {
      sizeof (AgsAudioApplicationContextClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_audio_application_context_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAudioApplicationContext),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_audio_application_context_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_audio_application_context_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_audio_application_context = g_type_register_static(AGS_TYPE_APPLICATION_CONTEXT,
								"AgsAudioApplicationContext\0",
								&ags_audio_application_context_info,
								0);

    g_type_add_interface_static(ags_type_audio_application_context,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return (ags_type_audio_application_context);
}

void
ags_audio_application_context_class_init(AgsAudioApplicationContextClass *audio_application_context)
{
  GObjectClass *gobject;
  AgsApplicationContextClass *application_context;
  GParamSpec *param_spec;
  
  ags_audio_application_context_parent_class = g_type_class_peek_parent(audio_application_context);

  /* GObjectClass */
  gobject = (GObjectClass *) audio_application_context;

  gobject->set_property = ags_audio_application_context_set_property;
  gobject->get_property = ags_audio_application_context_get_property;

  gobject->finalize = ags_audio_application_context_finalize;
  
  /**
   * AgsAudioApplicationContext:soundcard:
   *
   * The assigned soundcard.
   * 
   * Since: 0.4
   */
  param_spec = g_param_spec_object("soundcard\0",
				   "soundcard of audio application context\0",
				   "The soundcard which this audio application context assigned to\0",
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SOUNDCARD,
				  param_spec);

  /* AgsAudioApplicationContextClass */
  application_context = (AgsApplicationContextClass *) audio_application_context;
  
  application_context->load_config = ags_audio_application_context_load_config;
  application_context->register_types = ags_audio_application_context_register_types;
  application_context->read = ags_audio_application_context_read;
  application_context->write = ags_audio_application_context_write;
}

void
ags_audio_application_context_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_audio_application_context_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_audio_application_context_connect;
  connectable->disconnect = ags_audio_application_context_disconnect;
}

void
ags_audio_application_context_init(AgsAudioApplicationContext *audio_application_context)
{
  audio_application_context->flags = 0;

  audio_application_context->soundcard = NULL;
}

void
ags_audio_application_context_set_property(GObject *gobject,
					   guint prop_id,
					   const GValue *value,
					   GParamSpec *param_spec)
{
  AgsAudioApplicationContext *audio_application_context;

  audio_application_context = AGS_AUDIO_APPLICATION_CONTEXT(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    {
      GObject *soundcard;
      
      soundcard = (GObject *) g_value_get_object(value);

      if(soundcard == NULL){
	return;
      }
      
      if(g_list_find(audio_application_context->soundcard, soundcard) == NULL){
	g_object_ref(G_OBJECT(soundcard));

	audio_application_context->soundcard = g_list_prepend(audio_application_context->soundcard,
							       soundcard);
      }
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_audio_application_context_get_property(GObject *gobject,
					   guint prop_id,
					   GValue *value,
					   GParamSpec *param_spec)
{
  AgsAudioApplicationContext *audio_application_context;

  audio_application_context = AGS_AUDIO_APPLICATION_CONTEXT(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    {
      g_value_set_pointer(value, audio_application_context->soundcard);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_audio_application_context_connect(AgsConnectable *connectable)
{
  AgsAudioApplicationContext *audio_application_context;
  GList *list;
  
  audio_application_context = AGS_AUDIO_APPLICATION_CONTEXT(connectable);

  if((AGS_APPLICATION_CONTEXT_CONNECTED & (audio_application_context->flags)) != 0){
    return;
  }

  ags_audio_application_context_parent_connectable_interface->connect(connectable);

  g_message("connecting audio\0");
  
  list = audio_application_context->soundcard;

  while(list != NULL){
    ags_connectable_connect(AGS_CONNECTABLE(list->data));

    list = list->next;
  }
}

void
ags_audio_application_context_disconnect(AgsConnectable *connectable)
{
  AgsAudioApplicationContext *audio_application_context;

  audio_application_context = AGS_AUDIO_APPLICATION_CONTEXT(connectable);

  if((AGS_APPLICATION_CONTEXT_CONNECTED & (audio_application_context->flags)) == 0){
    return;
  }

  ags_audio_application_context_parent_connectable_interface->disconnect(connectable);
}

void
ags_audio_application_context_finalize(GObject *gobject)
{
  AgsAudioApplicationContext *audio_application_context;

  G_OBJECT_CLASS(ags_audio_application_context_parent_class)->finalize(gobject);

  audio_application_context = AGS_AUDIO_APPLICATION_CONTEXT(gobject);
}

void
ags_audio_application_context_load_config(AgsApplicationContext *application_context)
{
  AgsConfig *config;
  AgsSoundcard *soundcard;
  GList *list;
  
  gchar *alsa_handle;
  guint samplerate;
  guint buffer_size;
  guint pcm_channels, dsp_channels;

  config = application_context->config;
  
  list = AGS_AUDIO_APPLICATION_CONTEXT(application_context)->soundcard;

  while(soundcard != NULL){
    soundcard = AGS_SOUNDCARD(list->data);
    
    alsa_handle = ags_config_get_value(config,
				 AGS_CONFIG_SOUNDCARD,
				 "alsa-handle\0");

    dsp_channels = strtoul(ags_config_get_value(config,
					  AGS_CONFIG_SOUNDCARD,
					  "dsp-channels\0"),
			   NULL,
			   10);
    
    pcm_channels = strtoul(ags_config_get_value(config,
					  AGS_CONFIG_SOUNDCARD,
					  "pcm-channels\0"),
			   NULL,
			   10);

    samplerate = strtoul(ags_config_get_value(config,
					AGS_CONFIG_SOUNDCARD,
					"samplerate\0"),
			 NULL,
			 10);

    buffer_size = strtoul(ags_config_get_value(config,
					 AGS_CONFIG_SOUNDCARD,
					 "buffer-size\0"),
			  NULL,
			  10);
    
    g_object_set(G_OBJECT(soundcard),
		 "device\0", alsa_handle,
		 "dsp-channels\0", dsp_channels,
		 "pcm-channels\0", pcm_channels,
		 "frequency\0", samplerate,
		 "buffer-size\0", buffer_size,
		 NULL);

    list = list->next;
  }
}

void
ags_audio_application_context_set_value_callback(AgsConfig *config, gchar *group, gchar *key, gchar *value,
						 AgsAudioApplicationContext *audio_application_context)
{
  if(!strncmp(group,
	      AGS_CONFIG_SOUNDCARD,
	      7)){
    AgsSoundcard *soundcard;

    if(audio_application_context == NULL ||
       audio_application_context->soundcard == NULL){
      return;
    }

    soundcard = audio_application_context->soundcard->data;

    if(!strncmp(key,
		"samplerate\0",
		10)){    
      guint samplerate;

      samplerate = strtoul(value,
			   NULL,
			   10);

      g_object_set(G_OBJECT(soundcard),
		   "frequency\0", samplerate,
		   NULL);
    }else if(!strncmp(key,
		      "buffer-size\0",
		      11)){
      guint buffer_size;
    
      buffer_size = strtoul(value,
			    NULL,
			    10);

      g_object_set(G_OBJECT(soundcard),
		   "buffer-size\0", buffer_size,
		   NULL);
    }else if(!strncmp(key,
		      "pcm-channels\0",
		      12)){
      guint pcm_channels;

      pcm_channels = strtoul(value,
			     NULL,
			     10);
      
      g_object_set(G_OBJECT(soundcard),
		   "pcm-channels\0", pcm_channels,
		   NULL);
    }else if(!strncmp(key,
		      "dsp-channels\0",
		      12)){
      guint dsp_channels;

      dsp_channels = strtoul(value,
			     NULL,
			     10);
      
      g_object_set(G_OBJECT(soundcard),
		   "dsp-channels\0", dsp_channels,
		   NULL);
    }else if(!strncmp(key,
		      "alsa-handle\0",
		      11)){
      gchar *alsa_handle;
    
      alsa_handle = value;
      g_object_set(G_OBJECT(soundcard),
		   "device\0", alsa_handle,
		   NULL);
    }
  }
}

void
ags_audio_application_context_register_types(AgsApplicationContext *application_context)
{
  ags_audio_loop_get_type();
  ags_soundcard_thread_get_type();
  ags_export_thread_get_type();
  ags_record_thread_get_type();
  ags_iterator_thread_get_type();
  ags_recycling_thread_get_type();

  /* register recalls */
  ags_play_audio_get_type();
  ags_play_channel_get_type();
  ags_play_channel_run_get_type();
  ags_play_channel_run_master_get_type();

  ags_stream_channel_get_type();
  ags_stream_channel_run_get_type();

  ags_loop_channel_get_type();
  ags_loop_channel_run_get_type();

  ags_copy_channel_get_type();
  ags_copy_channel_run_get_type();

  ags_volume_channel_get_type();
  ags_volume_channel_run_get_type();

  ags_peak_channel_get_type();
  ags_peak_channel_run_get_type();

  ags_recall_ladspa_get_type();
  ags_recall_channel_run_dummy_get_type();
  ags_recall_ladspa_run_get_type();

  ags_delay_audio_get_type();
  ags_delay_audio_run_get_type();

  ags_count_beats_audio_get_type();
  ags_count_beats_audio_run_get_type();

  ags_copy_pattern_audio_get_type();
  ags_copy_pattern_audio_run_get_type();
  ags_copy_pattern_channel_get_type();
  ags_copy_pattern_channel_run_get_type();

  ags_buffer_channel_get_type();
  ags_buffer_channel_run_get_type();

  ags_play_notation_audio_get_type();
  ags_play_notation_audio_run_get_type();
}

void
ags_audio_application_context_read(AgsFile *file, xmlNode *node, GObject **application_context)
{
  AgsAudioApplicationContext *gobject;
  GList *list;
  xmlNode *child;

  if(*application_context == NULL){
    gobject = g_object_new(AGS_TYPE_AUDIO_APPLICATION_CONTEXT,
			   NULL);

    *application_context = (GObject *) gobject;
  }else{
    gobject = (AgsApplicationContext *) *application_context;
  }

  file->application_context = gobject;

  g_object_set(G_OBJECT(file),
	       "application-context\0", gobject,
	       NULL);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));
  
  /* properties */
  AGS_APPLICATION_CONTEXT(gobject)->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
								     NULL,
								     16);

  AGS_APPLICATION_CONTEXT(gobject)->version = xmlGetProp(node,
							 AGS_FILE_VERSION_PROP);

  AGS_APPLICATION_CONTEXT(gobject)->build_id = xmlGetProp(node,
							  AGS_FILE_BUILD_ID_PROP);

  //TODO:JK: check version compatibelity

  /* child elements */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp("ags-devout-list\0",
			   child->name,
			   16)){
	ags_file_read_devout_list(file,
				  child,
				  &(gobject->soundcard));
      }
    }

    child = child->next;
  }
}

xmlNode*
ags_audio_application_context_write(AgsFile *file, xmlNode *parent, GObject *application_context)
{
  xmlNode *node, *child;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-application-context\0");

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", application_context,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_CONTEXT_PROP,
	     "audio\0");

  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", ((~AGS_APPLICATION_CONTEXT_CONNECTED) & (AGS_APPLICATION_CONTEXT(application_context)->flags))));

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     AGS_APPLICATION_CONTEXT(application_context)->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     AGS_APPLICATION_CONTEXT(application_context)->build_id);

  /* add to parent */
  xmlAddChild(parent,
	      node);

  ags_file_write_devout_list(file,
			     node,
			     AGS_AUDIO_APPLICATION_CONTEXT(application_context)->soundcard);

  return(node);
}

AgsAudioApplicationContext*
ags_audio_application_context_new(GObject *main_loop,
				  AgsConfig *config)
{
  AgsAudioApplicationContext *audio_application_context;

  audio_application_context = (AgsAudioApplicationContext *) g_object_new(AGS_TYPE_AUDIO_APPLICATION_CONTEXT,
									  "main-loop\0", main_loop,
									  "config\0", config,
									  NULL);

  return(audio_application_context);
}


