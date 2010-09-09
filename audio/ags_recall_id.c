#include "ags_recall_id.h"
#include <stdio.h>

GType ags_recall_id_get_type(void);
void ags_recall_id_class_init(AgsRecallIDClass *recall_id);
void ags_recall_id_init(AgsRecallID *recall_id);
void ags_recall_id_connect(AgsRecallID *recall_id);
void ags_recall_id_finalize(GObject *gobject);

static gpointer ags_recall_id_parent_class = NULL;

GType
ags_recall_id_get_type(void)
{
  static GType ags_type_recall_id = 0;

  if(!ags_type_recall_id){
    static const GTypeInfo ags_recall_id_info = {
      sizeof (AgsRecallIDClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_recall_id_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRecallID),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_recall_id_init,
    };
    ags_type_recall_id = g_type_register_static(G_TYPE_OBJECT, "AgsRecallID\0", &ags_recall_id_info, 0);
  }
  return (ags_type_recall_id);
}

void
ags_recall_id_class_init(AgsRecallIDClass *recall_id)
{
  GObjectClass *gobject;

  ags_recall_id_parent_class = g_type_class_peek_parent(recall_id);

  gobject = (GObjectClass *) recall_id;
  gobject->finalize = ags_recall_id_finalize;
}

void
ags_recall_id_init(AgsRecallID *recall_id)
{
  recall_id->flags = 0;

  recall_id->parent_group_id = 0;
  recall_id->group_id = 0;
  recall_id->child_group_id = 0;

  recall_id->first_recycling = NULL;
  recall_id->last_recycling = NULL;
}

void
ags_recall_id_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_recall_id_parent_class)->finalize(gobject);
}

void
ags_recall_id_connect(AgsRecallID *recall_id)
{
}

guint
ags_recall_id_generate_group_id()
{
  static guint counter = 1;
  guint group_id;

  group_id = counter;
  counter++;

  if(counter == ~0)
    fprintf(stderr, "WARNING: ags_recall_id_generate_group_id - counter expired\n\0");

  return(group_id);
}

/*
 * output - the AgsChannel to check
 * stage - the current stage to check against
 * returns TRUE if the stage isn't run yet
 */
gboolean
ags_recall_id_get_run_stage(AgsRecallID *id, gint stage)
{
  switch(stage){
  case 0:
    if((AGS_RECALL_ID_PRE_SYNC_ASYNC_DONE & (id->flags)) == 0)
      return(TRUE);

    break;
  case 1:
    if((AGS_RECALL_ID_INTER_SYNC_ASYNC_DONE & (id->flags)) == 0)
      return(TRUE);

    break;
  case 2:
    if((AGS_RECALL_ID_POST_SYNC_ASYNC_DONE & (id->flags)) == 0)
      return(TRUE);
  }

  return(FALSE);
}

void
ags_recall_id_set_stage(AgsRecallID *recall_id, gint stage)
{
  if(stage == 0)
    recall_id->flags |= AGS_RECALL_ID_PRE_SYNC_ASYNC_DONE;
  else if(stage == 1)
    recall_id->flags |= AGS_RECALL_ID_INTER_SYNC_ASYNC_DONE;
  else
    recall_id->flags |= AGS_RECALL_ID_POST_SYNC_ASYNC_DONE;
}

void
ags_recall_id_unset_stage(AgsRecallID *recall_id, gint stage)
{
  if(stage == 0)
    recall_id->flags &= (~AGS_RECALL_ID_PRE_SYNC_ASYNC_DONE);
  else if(stage == 1)
    recall_id->flags &= (~AGS_RECALL_ID_INTER_SYNC_ASYNC_DONE);
  else
    recall_id->flags &= (~AGS_RECALL_ID_POST_SYNC_ASYNC_DONE);
}

GList*
ags_recall_id_add(GList *recall_id_list,
		  guint parent_group_id, guint group_id, guint child_group_id,
		  AgsRecycling *first_recycling, AgsRecycling *last_recycling)
{
  AgsRecallID *recall_id;
  GList *list;

  recall_id = ags_recall_id_new();

  recall_id->parent_group_id = parent_group_id;
  recall_id->group_id = group_id;
  recall_id->child_group_id = child_group_id;

  recall_id->first_recycling = first_recycling;
  recall_id->last_recycling = last_recycling;

  list = g_list_prepend(recall_id_list,
			(gpointer) recall_id);

  return(list);
}

AgsRecallID*
ags_recall_id_find_group_id(GList *recall_id_list, guint group_id)
{
  while(recall_id_list != NULL){
    if(AGS_RECALL_ID(recall_id_list->data)->group_id == group_id)
      return((AgsRecallID *) recall_id_list->data);

    recall_id_list = recall_id_list->next;
  }

  return(NULL);
}

AgsRecallID*
ags_recall_id_find_parent_group_id(GList *recall_id_list, guint parent_group_id)
{
  while(recall_id_list != NULL){
    if(AGS_RECALL_ID(recall_id_list->data)->parent_group_id == parent_group_id)
      return((AgsRecallID *) recall_id_list->data);

    recall_id_list = recall_id_list->next;
  }

  return(NULL);
}

AgsRecallID*
ags_recall_id_new()
{
  AgsRecallID *recall_id;

  recall_id = (AgsRecallID *) g_object_new(AGS_TYPE_RECALL_ID, NULL);

  return(recall_id);
}
