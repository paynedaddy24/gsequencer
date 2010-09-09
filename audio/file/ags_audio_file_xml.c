#include "ags_audio_file_xml.h"

GType ags_audio_file_xml_get_type();
void ags_audio_file_xml_class_init(AgsAudioFileXmlClass *file_xml);
void ags_audio_file_xml_init(AgsAudioFileXml *file_xml);
void ags_audio_file_xml_destroy(GObject *object);

GType
ags_audio_file_xml_get_type()
{
  static GType ags_type_audio_file_xml = 0;

  if(!ags_type_audio_file_xml){
    static const GTypeInfo ags_audio_file_xml_info = {
      sizeof (AgsAudioFileXmlClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_audio_file_xml_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAudioFile),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_audio_file_xml_init,
    };
    ags_type_audio_file_xml = g_type_register_static(G_TYPE_OBJECT, "AgsAudioFileXml\0", &ags_audio_file_xml_info, 0);
  }
  return (ags_type_audio_file_xml);
}

void
ags_audio_file_xml_class_init(AgsAudioFileXmlClass *file_xml)
{
}

void
ags_audio_file_xml_init(AgsAudioFileXml *file_xml)
{
}

void
ags_audio_file_xml_destroy(GObject *object)
{
}

AgsAudioFileXml*
ags_audio_file_xml_new(AgsAudioFile *audio_file)
{
  AgsAudioFileXml *file_xml;

  file_xml = (AgsAudioFileXml *) g_object_new(AGS_TYPE_AUDIO_FILE_XML, NULL);

  return(file_xml);
}
