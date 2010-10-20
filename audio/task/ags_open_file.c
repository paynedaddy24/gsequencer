#include "ags_open_file.h"

#include "../../object/ags_connectable.h"

void ags_open_file_class_init(AgsOpenFileClass *open_file);
void ags_open_file_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_open_file_init(AgsOpenFile *open_file);
void ags_open_file_connect(AgsConnectable *connectable);
void ags_open_file_disconnect(AgsConnectable *connectable);
void ags_open_file_finalize(GObject *gobject);

static gpointer ags_open_file_parent_class = NULL;
static AgsConnectableInterface *ags_open_file_parent_connectable_interface;

GType
ags_open_file_get_type()
{
  static GType ags_type_open_file = 0;

  if(!ags_type_open_file){
    static const GTypeInfo ags_open_file_info = {
      sizeof (AgsOpenFileClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_open_file_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsOpenFile),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_open_file_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_open_file_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_open_file = g_type_register_static(AGS_TYPE_TASK,
						"AgsOpenFile\0",
						&ags_open_file_info,
						0);

    g_type_add_interface_static(ags_type_open_file,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_open_file);
}

void
ags_open_file_class_init(AgsOpenFileClass *open_file)
{
  GObjectClass *gobject;

  ags_open_file_parent_class = g_type_class_peek_parent(open_file);

  gobject = (GObjectClass *) open_file;

  gobject->finalize = ags_open_file_finalize;
}

void
ags_open_file_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_open_file_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_open_file_connect;
  connectable->disconnect = ags_open_file_disconnect;
}

void
ags_open_file_init(AgsOpenFile *open_file)
{
}

void
ags_open_file_connect(AgsConnectable *connectable)
{
  ags_open_file_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_open_file_disconnect(AgsConnectable *connectable)
{
  ags_open_file_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_open_file_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_open_file_parent_class)->finalize(gobject);

  /* empty */
}

AgsOpenFile*
ags_open_file_new()
{
  AgsOpenFile *open_file;

  open_file = (AgsOpenFile *) g_object_new(AGS_TYPE_OPEN_FILE,
					   NULL);

  return(open_file);
}