#include "ags_pattern.h"
#include "ags_audio_signal.h"

#include <stdarg.h>
#include <math.h>
#include <string.h>

GType ags_pattern_get_type();
void ags_pattern_class_init(AgsPatternClass *pattern_class);
void ags_pattern_init(AgsPattern *pattern);
void ags_pattern_finalize(GObject *gobject);

static gpointer ags_pattern_parent_class = NULL;

GType
ags_pattern_get_type (void)
{
  static GType ags_type_pattern = 0;

  if(!ags_type_pattern){
    static const GTypeInfo ags_pattern_info = {
      sizeof (AgsPatternClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_pattern_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsPattern),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_pattern_init,
    };
    ags_type_pattern = g_type_register_static(G_TYPE_OBJECT, "AgsPattern\0", &ags_pattern_info, 0);
  }
  return (ags_type_pattern);
}

void
ags_pattern_class_init(AgsPatternClass *pattern)
{
  GObjectClass *gobject;

  ags_pattern_parent_class = g_type_class_peek_parent(pattern);

  gobject = (GObjectClass *) pattern;
  gobject->finalize = ags_pattern_finalize;
}

void
ags_pattern_init(AgsPattern *pattern)
{
  pattern->dim[0] = 0;
  pattern->dim[1] = 0;
  pattern->dim[2] = 0;

  pattern->pattern = NULL;

  pattern->offset = 0;
}

void
ags_pattern_finalize(GObject *gobject)
{
  AgsPattern *pattern;
  guint i, j;

  pattern = AGS_PATTERN(gobject);

  for(i = 0; i < pattern->dim[0]; i++){
    for(j = 0; i < pattern->dim[1]; i++){
      free(pattern->pattern[i][j]);
    }

    free(pattern->pattern[i]);
  }

  free(pattern->pattern);

  G_OBJECT_CLASS(ags_pattern_parent_class)->finalize(gobject);
}

void
ags_pattern_connect(AgsPattern *pattern)
{
}

AgsPattern*
ags_pattern_get_by_offset(GList *list, guint64 offset)
{
  if(list == NULL)
    return(NULL);

  while(AGS_PATTERN(list->data)->offset != offset){
    if(list->next == NULL)
      return(NULL);

    list = list->next;
  }

  return((AgsPattern *) list->data);
}

void 
ags_pattern_set_dim(AgsPattern *pattern, guint dim0, guint dim1, guint length)
{
  guint ***index0, **index1, *bitmap;
  guint i, j, k, j_set, k_set;
  guint bitmap_size;

  if(dim0 == 0 && pattern->pattern == NULL)
      return;

  // shrink
  if(pattern->dim[0] > dim0){
    for(i = dim0; i < pattern->dim[0]; i++){
      for(j = 0; j < pattern->dim[1]; j++)
	free(pattern->pattern[i][j]);

      free(pattern->pattern[i]);
    }

    if(dim0 == 0){
      free(pattern->pattern);
      ags_pattern_init(pattern);
      return;
    }else{
      index0 = (guint ***) malloc((int) dim0 * sizeof(guint**));

      for(i = 0; i < dim0; i++)
	index0[i] = pattern->pattern[i];

      free(pattern->pattern);
      pattern->pattern = index0;

      pattern->dim[0] = dim0;
    }
  }

  if(pattern->dim[1] > dim1){
    if(dim1 == 0){
      for(i = 0; i < pattern->dim[0]; i++){
	for(j = dim1; j < pattern->dim[1]; j++){
	  free(pattern->pattern[i][j]);
	}
	pattern->pattern[i] = NULL;
      }

    }else{
      for(i = 0; i < pattern->dim[0]; i++){
	for(j = dim1; j < pattern->dim[1]; j++){
	  free(pattern->pattern[i][j]);
	}
      }

      for(i = 0; pattern->dim[0]; i++){
	index1 = (guint **) malloc(dim1 * sizeof(guint*));

	for(j = 0; j < dim1; j++){
	  index1[j] = pattern->pattern[i][j];
	}

	free(pattern->pattern[i]);
	pattern->pattern[i] = index1;
      }
    }

    pattern->dim[1] = dim1;
  }

  if(pattern->dim[2] > length){
    for(i = 0; i < pattern->dim[0]; i++)
      for(j = 0; j < pattern->dim[1]; j++){
	bitmap = (guint *) malloc((int) ceil((double) length / (double) (sizeof(guint) * 8)) * sizeof(guint));

	for(k = 0; k < (guint) ceil((double) length / (double) (sizeof(guint) * 8)); k++)
	  bitmap[k] = pattern->pattern[i][j][k];

	free(pattern->pattern[i][j]);
	pattern->pattern[i][j] = bitmap;
      }
  }else if(pattern->dim[2] < length){ // grow
    for(i = 0; i < pattern->dim[0]; i++)
      for(j = 0; j < pattern->dim[1]; j++){
	bitmap = (guint *) malloc((int) ceil((double) length / (double) (sizeof(guint) * 8)) * sizeof(guint));

	for(k = 0; k < (guint) ceil((double) pattern->dim[2] / (double) (sizeof(guint) * 8)); k++)
	  bitmap[k] = pattern->pattern[i][j][k];

	free(pattern->pattern[i][j]);
	pattern->pattern[i][j] = bitmap;
      }
  }

  pattern->dim[2] = length;
  bitmap_size = (guint) ceil((double) pattern->dim[2] / (double) (sizeof(guint) * 8)) * sizeof(guint);

  if(pattern->dim[1] < dim1){
    for(i = 0; i < pattern->dim[0]; i++){
      index1 = (guint**) malloc(dim1 * sizeof(guint*));

      for(j = 0; j < pattern->dim[1]; j++)
	index1[j] = pattern->pattern[i][j];

      free(pattern->pattern[i]);
      pattern->pattern[i] = index1;

      for(j = pattern->dim[1]; j < dim1; j++){
	pattern->pattern[i][j] = (guint*) malloc(bitmap_size);
	memset(pattern->pattern[i][j], 0, bitmap_size);
      }
    }

    pattern->dim[1] = dim1;
  }

  if(pattern->dim[0] < dim0){
    index0 = (guint***) malloc(dim0 * sizeof(guint**));

    for(i = 0; i < pattern->dim[0]; i++)
      index0[i] = pattern->pattern[i];

    if(pattern->pattern != NULL)
      free(pattern->pattern);

    pattern->pattern = index0;

    for(i = pattern->dim[0]; i < dim0; i++){
      pattern->pattern[i] = (guint**) malloc(pattern->dim[1] * sizeof(guint*));

      for(j = 0; j < pattern->dim[1]; j++){
	pattern->pattern[i][j] = (guint*) malloc(bitmap_size);
	memset(pattern->pattern[i][j], 0, bitmap_size);
      }
    }

    pattern->dim[0] = dim0;
  }
}

gboolean
ags_pattern_get_bit(AgsPattern *pattern, guint i, guint j, guint bit)
{
  if(((1 << (bit % (sizeof(guint) *8))) & (pattern->pattern[i][j][(guint) floor((double) bit / (double) (sizeof(guint) * 8))])) != 0)
    return(TRUE);
  else
    return(FALSE);
}

void
ags_pattern_toggle_bit(AgsPattern *pattern, guint i, guint j, guint bit)
{
  guint k, value;

  k = (guint) floor((double) bit / (double) (sizeof(guint) * 8));
  value = 1 << (bit % (sizeof(guint) * 8));


  if(value & pattern->pattern[i][j][k])
    pattern->pattern[i][j][k] &= (~value);
  else
    pattern->pattern[i][j][k] |= value;
}

AgsPattern*
ags_pattern_new()
{
  AgsPattern *pattern;

  pattern = (AgsPattern *) g_object_new(AGS_TYPE_PATTERN, NULL);

  return(pattern);
}
