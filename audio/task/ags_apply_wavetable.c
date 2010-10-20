#include "ags_apply_wavetable.h"

#include "../../object/ags_connectable.h"

void ags_apply_wavetable_class_init(AgsApplyWavetableClass *apply_wavetable);
void ags_apply_wavetable_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_apply_wavetable_init(AgsApplyWavetable *apply_wavetable);
void ags_apply_wavetable_connect(AgsConnectable *connectable);
void ags_apply_wavetable_disconnect(AgsConnectable *connectable);
void ags_apply_wavetable_finalize(GObject *gobject);

static gpointer ags_apply_wavetable_parent_class = NULL;
static AgsConnectableInterface *ags_apply_wavetable_parent_connectable_interface;

GType
ags_apply_wavetable_get_type()
{
  static GType ags_type_apply_wavetable = 0;

  if(!ags_type_apply_wavetable){
    static const GTypeInfo ags_apply_wavetable_info = {
      sizeof (AgsApplyWavetableClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_apply_wavetable_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsApplyWavetable),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_apply_wavetable_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_apply_wavetable_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_apply_wavetable = g_type_register_static(AGS_TYPE_TASK,
						      "AgsApplyWavetable\0",
						      &ags_apply_wavetable_info,
						      0);

    g_type_add_interface_static(ags_type_apply_wavetable,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_apply_wavetable);
}

void
ags_apply_wavetable_class_init(AgsApplyWavetableClass *apply_wavetable)
{
  GObjectClass *gobject;

  ags_apply_wavetable_parent_class = g_type_class_peek_parent(apply_wavetable);

  gobject = (GObjectClass *) apply_wavetable;

  gobject->finalize = ags_apply_wavetable_finalize;
}

void
ags_apply_wavetable_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_apply_wavetable_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_apply_wavetable_connect;
  connectable->disconnect = ags_apply_wavetable_disconnect;
}

void
ags_apply_wavetable_init(AgsApplyWavetable *apply_wavetable)
{
}

void
ags_apply_wavetable_connect(AgsConnectable *connectable)
{
  ags_apply_wavetable_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_apply_wavetable_disconnect(AgsConnectable *connectable)
{
  ags_apply_wavetable_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_apply_wavetable_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_apply_wavetable_parent_class)->finalize(gobject);

  /* empty */
}

AgsApplyWavetable*
ags_apply_wavetable_new()
{
  AgsApplyWavetable *apply_wavetable;

  apply_wavetable = (AgsApplyWavetable *) g_object_new(AGS_TYPE_APPLY_WAVETABLE,
						       NULL);
  
  return(apply_wavetable);
}