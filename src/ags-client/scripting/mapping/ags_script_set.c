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

#include <ags-client/scripting/mapping/ags_script_set.h>

#include <ags-lib/object/ags_connectable.h>

#include <ags-client/scripting/ags_script.h>
#include <ags-client/scripting/ags_xml_script_factory.h>

#include <ags-client/scripting/mapping/ags_script_var.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

void ags_script_set_class_init(AgsScriptSetClass *script_set);
void ags_script_set_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_script_set_init(AgsScriptSet *script_set);
void ags_script_set_connect(AgsConnectable *connectable);
void ags_script_set_disconnect(AgsConnectable *connectable);
void ags_script_set_finalize(GObject *gobject);

AgsScriptObject* ags_script_set_launch(AgsScriptObject *script_object);

xmlNode* ags_script_set_matrix_find_index(AgsScriptSet *script_set,
					  AgsScriptArray *matrix,
					  guint y);

void ags_script_set_matrix_get(AgsScriptSet *script_set,
			       AgsScriptArray *matrix,
			       AgsScriptVar *lvalue,
			       guint offset,
			       guint *x, guint *y);
void ags_script_set_matrix_put(AgsScriptSet *script_set,
			       AgsScriptArray *matrix,
			       AgsScriptVar *value,
			       guint offset,
			       guint *x, guint *y);

void ags_script_set_prime(AgsScriptSet *script_set,
			  AgsScriptVar *a,
			  AgsScriptArray *prime);
void ags_script_set_ggt(AgsScriptSet *script_set,
			AgsScriptVar *a,
			AgsScriptVar *b,
			AgsScriptVar *ggt);
void ags_script_set_kgv(AgsScriptSet *script_set,
			AgsScriptVar *a,
			AgsScriptVar *b,
			AgsScriptVar *kgv);
void ags_script_set_value(AgsScriptSet *script_set,
			  AgsScriptVar *value,
			  AgsScriptArray *lvalue);

void ags_script_set_default_index(AgsScriptSet *script_set,
				  AgsScriptArray *index);
void ags_script_set_matrix_move_index(AgsScriptSet *script_set,
				      AgsScriptArray *matrix,
				      AgsScriptArray *index,
				      guint dest_x, guint dest_y,
				      guint src_x, guint src_y);

void ags_script_set_matrix_sort(AgsScriptSet *script_set,
				AgsScriptArray *matrix,
				gboolean sort_column, gboolean sort_row,
				AgsScriptArray *index, guint depth,
				guint *x, guint *y);
void ags_script_set_matrix_sort_by_operands(AgsScriptSet *script_set,
				       AgsScriptArray *matrix,
				       AgsScriptArray *operands,
				       AgsScriptArray *index, guint depth,
				       guint *x, guint *y);

void ags_script_set_matrix_vector_mirror(AgsScriptSet *script_set,
					 AgsScriptArray *matrix,
					 AgsScriptArray *vector);

void ags_script_set_equation(AgsScriptSet *script_set,
			     AgsScriptArray *factorized,
			     AgsScriptArray *operands);

#define AGS_SCRIPT_SET_BASE64_UNIT (4)
#define AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT (3)

static gpointer ags_script_set_parent_class = NULL;

GType
ags_script_set_get_type()
{
  static GType ags_type_script_set = 0;

  if(!ags_type_script_set){
    static const GTypeInfo ags_script_set_info = {
      sizeof (AgsScriptSetClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_script_set_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsScriptSet),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_script_set_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_script_set_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_script_set = g_type_register_static(AGS_TYPE_SCRIPT_CONTROLLER,
						 "AgsScriptSet\0",
						 &ags_script_set_info,
						 0);
    
    g_type_add_interface_static(ags_type_script_set,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return (ags_type_script_set);
}

void
ags_script_set_class_init(AgsScriptSetClass *script_set)
{
  AgsScriptObjectClass *script_object;
  GObjectClass *gobject;

  ags_script_set_parent_class = g_type_class_peek_parent(script_set);

  /* GObjectClass */
  gobject = (GObjectClass *) script_set;

  gobject->finalize = ags_script_set_finalize;

  /* AgsScriptObjectClass */
  script_object = (AgsScriptObjectClass *) script_set;

  script_object->launch = ags_script_set_launch;
}

void
ags_script_set_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->connect = ags_script_set_connect;
  connectable->disconnect = ags_script_set_disconnect;
}

void
ags_script_set_init(AgsScriptSet *script_set)
{
  script_set->flags = 0;

  script_set->start =
    script_set->current = NULL;
}

void
ags_script_set_connect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_script_set_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_script_set_finalize(GObject *gobject)
{
  AgsScriptSet *script_set;

  script_set = AGS_SCRIPT_SET(gobject);

  if(script_set->start != NULL){
    free(script_set->start);
  }

  G_OBJECT_CLASS(ags_script_set_parent_class)->finalize(gobject);
}

AgsScriptObject*
ags_script_set_launch(AgsScriptObject *script_object)
{
  //TODO:JK: implement me

  return(script_object);
}

void
ags_script_set_from_string(AgsScriptSet *script_set, gchar *string)
{
  //TODO:JK: implement me
}

gchar*
ags_script_set_matrix_to_string(AgsScriptSet *script_set,
				AgsScriptArray *script_array)
{
  gchar *string;

  auto gchar* ags_script_set_fill_column(AgsScriptArray *script_array, xmlNode *column);
  auto gchar* ags_script_set_fill_row(AgsScriptArray *script_array, xmlNode *row);

  gchar* ags_script_set_fill_column(AgsScriptArray *script_array, xmlNode *column){
    xmlChar *content, *tmp;
    guchar *value;
    union{
      gint16 *data_int16;
      guint16 *data_uint16;
      gint32 *data_int32;
      guint32 *data_uint32;
      gint64 *data_int64;
      guint64 *data_uint64;
      gdouble *data_double;
      gchar *data_char;
    }ptr;
    gchar *string;
    gsize retlength;
    guint i, i_stop;

    content = xmlNodeGetContent(column);

    value = g_base64_decode(content, &retlength);
    string = NULL;

    switch(script_array->mode){
    case AGS_SCRIPT_ARRAY_INT16:
      {
	ptr.data_int16 = (gint16 *) value;

	i_stop = retlength * AGS_SCRIPT_SET_BASE64_UNIT / sizeof(gint16);
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT16:
      {
	ptr.data_uint16 = (gint16 *) value;

	i_stop = retlength * AGS_SCRIPT_SET_BASE64_UNIT / sizeof(guint16);
      }
      break;
    case AGS_SCRIPT_ARRAY_INT32:
      {
	ptr.data_int32 = (gint32 *) value;

	i_stop = retlength * AGS_SCRIPT_SET_BASE64_UNIT / sizeof(gint32);
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT32:
      {
	ptr.data_uint32 = (guint32 *) value;

	i_stop = retlength * AGS_SCRIPT_SET_BASE64_UNIT / sizeof(guint32);
      }
      break;
    case AGS_SCRIPT_ARRAY_INT64:
      {
	ptr.data_int64 = (gint64 *) value;

	i_stop = retlength * AGS_SCRIPT_SET_BASE64_UNIT / sizeof(gint64);
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT64:
      {
	ptr.data_uint64 = (guint64 *) value;

	i_stop = retlength * AGS_SCRIPT_SET_BASE64_UNIT / sizeof(guint64);
      }
      break;
    case AGS_SCRIPT_ARRAY_DOUBLE:
      {
	ptr.data_double = (gdouble *) value;

	i_stop = retlength * AGS_SCRIPT_SET_BASE64_UNIT / sizeof(gdouble);
      }
      break;
    case AGS_SCRIPT_ARRAY_CHAR:
      {
	ptr.data_char = (gchar *) value;

	g_message("unsupported data type: gchar\0");
      }
      break;
    case AGS_SCRIPT_ARRAY_POINTER:
      {
	g_message("unsupported data type: gpointer\0");
      }
      break;
    }

    for(i = 0; i < i_stop; i++){
      switch(script_array->mode){
      case AGS_SCRIPT_ARRAY_INT16:
	{
	  gint16 k;

	  memcpy(&k, &(ptr.data_int16[i * sizeof(gint16)]), sizeof(gint16));

	  tmp = string;

	  string = g_strdup_printf("%s %d:\0", string, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT16:
	{
	  guint16 k;

	  memcpy(&k, &(ptr.data_uint16[i * sizeof(guint16)]), sizeof(guint16));

	  tmp = string;

	  string = g_strdup_printf("%s %d:\0", string, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_INT32:
	{
	  gint32 k;

	  memcpy(&k, &(ptr.data_int32[i * sizeof(gint32)]), sizeof(gint32));

	  tmp = string;

	  string = g_strdup_printf("%s %d:\0", string, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT32:
	{
	  guint32 k;

	  memcpy(&k, &(ptr.data_uint32[i * sizeof(guint32)]), sizeof(guint32));

	  tmp = string;

	  string = g_strdup_printf("%s %d:\0", string, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_INT64:
	{
	  gint64 k;

	  memcpy(&k, &(ptr.data_int64[i * sizeof(gint64)]), sizeof(gint64));

	  tmp = string;

	  string = g_strdup_printf("%s %d:\0", string, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT64:
	{
	  guint64 k;

	  memcpy(&k, &(ptr.data_uint64[i * sizeof(guint64)]), sizeof(guint64));

	  tmp = string;

	  string = g_strdup_printf("%s %d:\0", string, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_DOUBLE:
	{
	  gdouble k;

	  memcpy(&k, &(ptr.data_double[i * sizeof(gdouble)]), sizeof(gdouble));

	  tmp = string;

	  string = g_strdup_printf("%s %f:\0", string, k);
	  g_free(tmp);
	}
	break;
      case AGS_SCRIPT_ARRAY_CHAR:
	{
	  g_message("unsupported data type: gchar\0");
	}
	break;
      case AGS_SCRIPT_ARRAY_POINTER:
	{
	  g_message("unsupported data type: gpointer\0");
	}
	break;
      }
    }

    return(string);
  }
  gchar* ags_script_set_fill_row(AgsScriptArray *script_array, xmlNode *row){
    xmlNode *current;
    gchar *matrix, *column, *tmp;

    current = row;
    matrix = g_strdup("{\0");
    
    while(current != NULL){
      if(current->type == XML_ELEMENT_NODE){
	column = ags_script_set_fill_column(script_array, current);
	tmp = matrix;

	matrix = g_strconcat(matrix, column, ";\0");
	g_free(tmp);
      }

      current = current->next;
    }

    tmp = matrix;

    matrix = g_strconcat(matrix, "}\0");
    g_free(tmp);

    return(matrix);
  }

  string = ags_script_set_fill_row(script_array, AGS_SCRIPT_OBJECT(script_array)->node);

  return(string);
}

AgsScriptArray*
ags_script_set_matrix_from_string(AgsScriptSet *script_set,
				  gchar *string)
{
  AgsXmlScriptFactory *xml_script_factory;
  AgsScriptArray *script_array;
  xmlNode *matrix;
  GError *error;

  auto void ags_script_set_matrix_from_string_read_col(AgsScriptArray *script_array,
						       xmlNode *node,
						       gchar *string);
  auto void ags_script_set_matrix_from_string_read_row(AgsScriptArray *script_array,
						       xmlNode *node,
						       gchar *string);
  auto void ags_script_set_matrix_from_string_validate(AgsScriptArray *script_array,
						       xmlNode *node);

  void ags_script_set_matrix_from_string_read_col(AgsScriptArray *script_array,
						  xmlNode *node,
						  gchar *string){
    guchar *col, *data;
    gchar *current, *prev;
    guint n_cols;

    col = NULL;

    current =
      prev = string;
    n_cols = 0;

    while((current = strchr(current, ':')) != NULL){

      switch(script_array->mode){
      case AGS_SCRIPT_ARRAY_INT16:
	{
	  gint16 k;

	  sscanf(prev, "%d:\0", &k);

	  if(col == NULL){
	    col = (guchar *) (gint16 *) malloc(sizeof(gint16));
	  }else{
	    col = (guchar *) (gint16 *) realloc(col,
						(n_cols + 1) * sizeof(gint16));
	  }

	  memcpy(col, &k, sizeof(gint16));
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT16:
	{
	  guint16 k;

	  sscanf(prev, "%d:\0", &k);

	  if(col == NULL){
	    col = (guchar *) (guint16 *) malloc(sizeof(guint16));
	  }else{
	    col = (guchar *) (guint16 *) realloc(col,
						 (n_cols + 1) * sizeof(guint16));
	  }

	  memcpy(col, &k, sizeof(guint16));
	}
	break;
      case AGS_SCRIPT_ARRAY_INT32:
	{
	  gint32 k;

	  sscanf(prev, "%d:\0", &k);

	  if(col == NULL){
	    col = (guchar *) (gint32 *) malloc(sizeof(gint32));
	  }else{
	    col = (guchar *) (gint32 *) realloc(col,
						(n_cols + 1) * sizeof(gint32));
	  }

	  memcpy(col, &k, sizeof(gint32));
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT32:
	{
	  guint32 k;

	  sscanf(prev, "%d:\0", &k);

	  if(col == NULL){
	    col = (guchar *) (guint32 *) malloc(sizeof(guint32));
	  }else{
	    col = (guchar *) (guint32 *) realloc(col,
						 (n_cols + 1) * sizeof(guint32));
	  }

	  memcpy(col, &k, sizeof(guint32));
	}
	break;
      case AGS_SCRIPT_ARRAY_INT64:
	{
	  gint64 k;

	  sscanf(prev, "%d:\0", &k);

	  if(col == NULL){
	    col = (guchar *) (gint64 *) malloc(sizeof(gint64));
	  }else{
	    col = (guchar *) (gint64 *) realloc(col,
						(n_cols + 1) * sizeof(gint64));
	  }

	  memcpy(col, &k, sizeof(gint64));
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT64:
	{
	  guint64 k;

	  sscanf(prev, "%d:\0", &k);

	  if(col == NULL){
	    col = (guchar *) (guint64 *) malloc(sizeof(guint64));
	  }else{
	    col = (guchar *) (guint64 *) realloc(col,
						 (n_cols + 1) * sizeof(guint64));
	  }

	  memcpy(col, &k, sizeof(guint64));
	}
	break;
      case AGS_SCRIPT_ARRAY_DOUBLE:
	{
	  gdouble k;

	  sscanf(prev, "%f:\0", &k);

	  if(col == NULL){
	    col = (guchar *) (gdouble *) malloc(sizeof(gdouble));
	  }else{
	    col = (guchar *) (gdouble *) realloc(col,
						 (n_cols + 1) * sizeof(gdouble));
	  }

	  memcpy(col, &k, sizeof(gdouble));
	}
	break;
      case AGS_SCRIPT_ARRAY_CHAR:
	{
	  g_message("unsupported data type: gchar\0");
	}
	break;
      case AGS_SCRIPT_ARRAY_POINTER:
	{
	  g_message("unsupported data type: gpointer\0");
	}
	break;
      }

      current++;
      prev = current;
      n_cols++;
    }

    xmlNodeSetContent(node, (xmlChar *) g_base64_encode(col, n_cols));
    xmlSetProp(node, "length\0", (xmlChar *) g_strdup_printf("%d\0", n_cols));
  }
  void ags_script_set_matrix_from_string_read_row(AgsScriptArray *script_array,
						  xmlNode *node,
						  gchar *string){
    xmlNode *row;
    gchar *current, *prev;
    guint rows;

    current =
      prev = string;
    rows = 0;

    while((current = strchr(current, ';')) != NULL){
      row = ags_xml_script_factory_map(xml_script_factory,
				       "ags-array\0",
				       &error);
      ags_script_set_matrix_from_string_read_col(script_array,
						 row,
						 prev);
      xmlAddChild(node, row);

      current++;
      prev = current;
      rows++;
    }

    xmlSetProp(node, "length\0", (xmlChar *) g_strdup_printf("%d\0", rows));
  }
  void ags_script_set_matrix_from_string_validate(AgsScriptArray *script_array,
						  xmlNode *node){
    xmlNode *current;
    guchar *col;
    guint *cols;
    guint n_cols, n_rows;
    guint current_length;
    guint i, j, j_start;
    gsize retlength;

    n_rows = strtoul(xmlGetProp(node, "length\0"), NULL, 10);
    cols = (guint *) malloc(n_rows * sizeof(guint));

    current = node->children;
    n_cols = 0;

    /* read cols */
    for(i = 0; i < n_rows; i++){
      cols[i] = strtoul(xmlGetProp(current, "length\0"), NULL, 10);

      /* find longest row */
      if(n_cols < cols[i]){
	n_cols = cols[i];
      }

      current = current->next;
    }

    /* fill invalid rows */
    current = node->children;

    for(i = 0; i < n_rows; i++){
      col = g_base64_decode((guchar *) xmlNodeGetContent(node),
			    &retlength);
      j_start = strtoul(xmlGetProp(node, "length\0"), NULL, 10);

      switch(script_array->mode){
      case AGS_SCRIPT_ARRAY_INT16:
	{
	  gint16 k;

	  k = 0;
	    
	  if(col == NULL){
	    col = (guchar *) (gint16 *) malloc(n_cols * sizeof(gint16));
	  }else{
	    col = (guchar *) (gint16 *) realloc(col,
						n_cols * sizeof(gint16));
	  }

	  for(j = j_start; j < n_cols; j++){
	    memcpy(&(col[j * sizeof(gint16)]), &k, sizeof(gint16));
	  }
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT16:
	{
	  guint16 k;

	  k = 0;
	    
	  if(col == NULL){
	    col = (guchar *) (guint16 *) malloc(n_cols * sizeof(guint16));
	  }else{
	    col = (guchar *) (guint16 *) realloc(col,
						 n_cols * sizeof(guint16));
	  }

	  for(j = j_start; j < n_cols; j++){
	    memcpy(&(col[j * sizeof(guint16)]), &k, sizeof(guint16));
	  }
	}
	break;
      case AGS_SCRIPT_ARRAY_INT32:
	{
	  gint32 k;

	  k = 0;
	    
	  if(col == NULL){
	    col = (guchar *) (gint32 *) malloc(n_cols * sizeof(gint32));
	  }else{
	    col = (guchar *) (gint32 *) realloc(col,
						n_cols * sizeof(gint32));
	  }

	  for(j = j_start; j < n_cols; j++){
	    memcpy(&(col[j * sizeof(gint32)]), &k, sizeof(gint32));
	  }
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT32:
	{
	  guint32 k;

	  k = 0;
	    
	  if(col == NULL){
	    col = (guchar *) (guint32 *) malloc(n_cols * sizeof(guint32));
	  }else{
	    col = (guchar *) (guint32 *) realloc(col,
						 n_cols * sizeof(guint32));
	  }

	  for(j = j_start; j < n_cols; j++){
	    memcpy(&(col[j * sizeof(guint32)]), &k, sizeof(guint32));
	  }
	}
	break;
      case AGS_SCRIPT_ARRAY_INT64:
	{
	  gint64 k;

	  k = 0;
	    
	  if(col == NULL){
	    col = (guchar *) (gint64 *) malloc(n_cols * sizeof(gint64));
	  }else{
	    col = (guchar *) (gint64 *) realloc(col,
						n_cols * sizeof(gint64));
	  }

	  for(j = j_start; j < n_cols; j++){
	    memcpy(&(col[j * sizeof(gint64)]), &k, sizeof(gint64));
	  }
	}
	break;
      case AGS_SCRIPT_ARRAY_UINT64:
	{
	  guint64 k;

	  k = 0;
	    
	  if(col == NULL){
	    col = (guchar *) (guint64 *) malloc(n_cols * sizeof(guint64));
	  }else{
	    col = (guchar *) (guint64 *) realloc(col,
						 n_cols * sizeof(guint64));
	  }

	  for(j = j_start; j < n_cols; j++){
	    memcpy(&(col[j * sizeof(guint64)]), &k, sizeof(guint64));
	  }
	}
	break;
      case AGS_SCRIPT_ARRAY_DOUBLE:
	{
	  gdouble k;

	  k = 0.0;
	    
	  if(col == NULL){
	    col = (guchar *) (gdouble *) malloc(n_cols * sizeof(gdouble));
	  }else{
	    col = (guchar *) (gdouble *) realloc(col,
						 n_cols * sizeof(gdouble));
	  }

	  for(j = j_start; j < n_cols; j++){
	    memcpy(&(col[j * sizeof(gdouble)]), &k, sizeof(gdouble));
	  }
	}
	break;
      case AGS_SCRIPT_ARRAY_CHAR:
	{
	  g_message("unsupported data type: gchar\0");
	}
	break;
      case AGS_SCRIPT_ARRAY_POINTER:
	{
	  g_message("unsupported data type: gpointer\0");
	}
	break;
      }

      col = g_base64_encode(col,
			    n_cols);

      current = current->next;
    }
  }

  xml_script_factory = AGS_SCRIPT(AGS_SCRIPT_OBJECT(script_set)->script)->xml_script_factory;

  script_array = ags_script_array_new();
  script_array->flags &= (~AGS_SCRIPT_ARRAY_UTF8_ENCODED);
  script_array->flags |= AGS_SCRIPT_ARRAY_BASE64_ENCODED;
  script_array->mode = AGS_SCRIPT_ARRAY_DOUBLE;

  error = NULL;

  matrix = AGS_SCRIPT_OBJECT(script_array)->node;

  if(error != NULL){
    g_message("failed to instantiate xmlNode: ags-array\0");

    g_object_unref(script_array);

    return(NULL);
  }

  //TODO:JK: implement data type - see above
  ags_script_set_matrix_from_string_read_row(script_array,
					     matrix,
					     string);
  ags_script_set_matrix_from_string_validate(script_array,
					     matrix);

  return(script_array);
}

xmlNode*
ags_script_set_matrix_find_index(AgsScriptSet *script_set,
				 AgsScriptArray *matrix,
				 guint y)
{
  xmlNode *current;

  current = AGS_SCRIPT_OBJECT(matrix)->node->children;

  while(current != NULL){
    if(strtoul(xmlGetProp(current, "index\0"), NULL, 10) == y){
      return(current);
    }

    current = current->next;
  }

  return(NULL);
}

void
ags_script_set_matrix_get(AgsScriptSet *script_set,
			  AgsScriptArray *matrix,
			  AgsScriptVar *lvalue,
			  guint offset,
			  guint *x, guint *y)
{
  xmlNode *current, *row;
  guchar *data;
  guint n_cols;
  guint ret_x, ret_y;
  gsize retlength;
  
  row = AGS_SCRIPT_OBJECT(matrix)->node->children;

  if(xmlNodeGetContent(AGS_SCRIPT_OBJECT(lvalue)->node) != NULL){
    xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node,
		      NULL);
  }

  if(row != NULL){
    n_cols = strtoul(xmlGetProp(row, "length\0"), NULL, 10);
    
    ret_x = (guint) floor(offset / n_cols);
    ret_y = offset % n_cols;

    current = ags_script_set_matrix_find_index(script_set,
					       matrix,
					       ret_y);

    data = g_base64_decode(xmlNodeGetContent(current),
			   &retlength);
    
    switch(matrix->mode){
    case AGS_SCRIPT_ARRAY_INT16:
      {
	gint16 *k;
	
	k = (gint16 *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(gint16)));
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT16:
      {
	guint16 *k;

	k = (guint16 *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(guint16)));
      }
      break;
    case AGS_SCRIPT_ARRAY_INT32:
      {
	gint32 *k;

	k = (gint32 *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(gint32)));
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT32:
      {
	guint32 *k;

	k = (guint32 *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(guint32)));
      }
      break;
    case AGS_SCRIPT_ARRAY_INT64:
      {
	gint64 *k;

	k = (gint64 *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(gint64)));
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT64:
      {
	guint64 *k;

	k = (guint64 *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(guint64)));
      }
      break;
    case AGS_SCRIPT_ARRAY_DOUBLE:
      {
	gdouble *k;

	k = (gdouble *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(gdouble)));
      }
      break;
    case AGS_SCRIPT_ARRAY_CHAR:
      {
	char *k;

	k = (char *) data;
	xmlNodeSetContent(AGS_SCRIPT_OBJECT(lvalue)->node, g_base64_encode((guchar *) &(k[ret_x]),
									   sizeof(char)));
      }
      break;
    case AGS_SCRIPT_ARRAY_POINTER:
      {
	g_message("unsupported data type: gpointer\0");
      }
      break;
    }

    free(data);
  }else{
    ret_x = G_MAXUINT;
    ret_y = G_MAXUINT;
  }
  
  *x = ret_x;
  *y = ret_y;
}

void
ags_script_set_matrix_put(AgsScriptSet *script_set,
			  AgsScriptArray *matrix,
			  AgsScriptVar *value,
			  guint offset,
			  guint *x, guint *y)
{
  xmlNode *current, *row;
  xmlChar *data;
  guchar *tmp_col;
  guint offset;
  guint n_cols;
  guint ret_x, ret_y;
  gsize putlength;
  GError *error;
  
  row = AGS_SCRIPT_OBJECT(matrix)->node->children;

  if(row != NULL){
    n_cols = strtoul(xmlGetProp(row, "length\0"), NULL, 10);
    
    ret_x = (guint) floor(offset / n_cols);
    ret_y = offset % n_cols;

    current = ags_script_set_matrix_find_index(script_set,
					       matrix,
					       ret_y);

    data = xmlNodeGetContent(current);

    switch(matrix->mode){
    case AGS_SCRIPT_ARRAY_INT16:
      {
	guchar *value_data;
	gint16 *k_ptr;
	gint16 k;
	gint state;
	guint save;
	gsize retlength;

	putlength = (guint) round((gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
				  (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
				  sizeof(gint16)) *
	  AGS_SCRIPT_SET_BASE64_UNIT +
	  AGS_SCRIPT_SET_BASE64_UNIT;

	offset = (guint) floor((gdouble) x *
			       (gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
			       (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
			       (gdouble) sizeof(gint16));
	
	/*  */
	k_ptr = (gint16 *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(&(data[offset]),
			     putlength,
			     (guchar *) k_ptr,
			     &state,
			     &save);

	k_ptr[(x * AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT) %
	      AGS_SCRIPT_SET_BASE64_UNIT] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  &putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT16:
      {
	guchar *value_data;
	guint16 *k_ptr;
	guint16 k;
	gint state;
	guint save;
	gsize retlength;

	putlength = (guint) round((gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
				  (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
				  sizeof(guint16)) *
	  AGS_SCRIPT_SET_BASE64_UNIT +
	  AGS_SCRIPT_SET_BASE64_UNIT;

	offset = (guint) floor((gdouble) x *
			       (gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
			       (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
			       (gdouble) sizeof(guint16));
	
	/*  */
	k_ptr = (guint16 *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(&(data[offset]),
			     putlength,
			     (guchar *) k_ptr,
			     &state,
			     &save);

	k_ptr[(x * AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT) %
	      AGS_SCRIPT_SET_BASE64_UNIT] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  &putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_INT32:
      {
	guchar *value_data;
	gint32 *k_ptr;
	gint32 k;
	gint state;
	guint save;
	gsize retlength;

	putlength = (guint) round((gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
				  (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
				  sizeof(gint32)) *
	  AGS_SCRIPT_SET_BASE64_UNIT +
	  AGS_SCRIPT_SET_BASE64_UNIT;

	offset = (guint) floor((gdouble) x *
			       (gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
			       (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
			       (gdouble) sizeof(gint32));
	
	/*  */
	k_ptr = (gint32 *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(&(data[offset]),
			     putlength,
			     (guchar *) k_ptr,
			     &state,
			     &save);

	k_ptr[(x * AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT) %
	      AGS_SCRIPT_SET_BASE64_UNIT] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  &putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT32:
      {
	guchar *value_data;
	guint32 *k_ptr;
	guint32 k;
	gint state;
	guint save;
	gsize retlength;

	putlength = (guint) round((gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
				  (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
				  sizeof(guint32)) *
	  AGS_SCRIPT_SET_BASE64_UNIT +
	  AGS_SCRIPT_SET_BASE64_UNIT;

	offset = (guint) floor((gdouble) x *
			       (gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
			       (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
			       (gdouble) sizeof(guint32));
	
	/*  */
	k_ptr = (guint32 *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(&(data[offset]),
			     putlength,
			     (guchar *) k_ptr,
			     &state,
			     &save);

	k_ptr[(x * AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT) %
	      AGS_SCRIPT_SET_BASE64_UNIT] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  &putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_INT64:
      {
	guchar *value_data;
	gint64 *k_ptr;
	gint64 k;
	gint state;
	guint save;
	gsize retlength;

	putlength = (guint) round((gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
				  (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
				  sizeof(gint64)) *
	  AGS_SCRIPT_SET_BASE64_UNIT +
	  AGS_SCRIPT_SET_BASE64_UNIT;

	offset = (guint) floor((gdouble) x *
			       (gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
			       (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
			       (gdouble) sizeof(gint64));
	
	/*  */
	k_ptr = (gint64 *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(&(data[offset]),
			     putlength,
			     (guchar *) k_ptr,
			     &state,
			     &save);

	k_ptr[(x * AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT) %
	      AGS_SCRIPT_SET_BASE64_UNIT] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  &putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT64:
      {
	guchar *value_data;
	guint64 *k_ptr;
	guint64 k;
	gint state;
	guint save;
	gsize retlength;

	putlength = (guint) round((gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
				  (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
				  sizeof(guint64)) *
	  AGS_SCRIPT_SET_BASE64_UNIT +
	  AGS_SCRIPT_SET_BASE64_UNIT;

	offset = (guint) floor((gdouble) x *
			       (gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
			       (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
			       (gdouble) sizeof(guint64));
	
	/*  */
	k_ptr = (guint64 *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(&(data[offset]),
			     putlength,
			     (guchar *) k_ptr,
			     &state,
			     &save);

	k_ptr[(x * AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT) %
	      AGS_SCRIPT_SET_BASE64_UNIT] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  &putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_DOUBLE:
      {
	guchar *value_data;
	gdouble *k_ptr;
	gdouble k;
	gint state;
	guint save;
	gsize retlength;

	putlength = (guint) round((gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
				  (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
				  sizeof(gdouble)) *
	  AGS_SCRIPT_SET_BASE64_UNIT +
	  AGS_SCRIPT_SET_BASE64_UNIT;

	offset = (guint) floor((gdouble) x *
			       (gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
			       (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
			       (gdouble) sizeof(gdouble));
	
	/*  */
	k_ptr = (gdouble *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(&(data[offset]),
			     putlength,
			     (guchar *) k_ptr,
			     &state,
			     &save);

	k_ptr[(x * AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT) %
	      AGS_SCRIPT_SET_BASE64_UNIT] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  &putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_CHAR:
      {
	guchar *value_data;
	char *k_ptr;
	char k;
	gint state;
	guint save;
	gsize retlength;

	putlength = (guint) round((gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
				  (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
				  sizeof(char)) *
	  AGS_SCRIPT_SET_BASE64_UNIT +
	  AGS_SCRIPT_SET_BASE64_UNIT;

	offset = (guint) floor((gdouble) x *
			       (gdouble) AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT /
			       (gdouble) AGS_SCRIPT_SET_BASE64_UNIT *
			       (gdouble) sizeof(char));
	
	/*  */
	k_ptr = (char *) malloc(putlength);

	state = 0;
	save = 0;

	g_base64_decode_step(&(data[offset]),
			     putlength,
			     (guchar *) k_ptr,
			     &state,
			     &save);

	k_ptr[(x * AGS_SCRIPT_SET_BASE64_BYTES_PER_UNIT) %
	      AGS_SCRIPT_SET_BASE64_UNIT] = k;

	tmp_col = g_base64_encode((guchar *) k_ptr,
				  &putlength);
	free(k_ptr);
      }
      break;
    case AGS_SCRIPT_ARRAY_POINTER:
      {
	g_message("unsupported data type: gpointer\0");
      }
      break;
    }
  }else{
    ret_x = G_MAXUINT;
    ret_y = G_MAXUINT;
  }

  memcpy(&(data[offset]), tmp_col, putlength);
  g_free(tmp_col);

  *x = ret_x;
  *y = ret_y;
}


void
ags_script_set_prime(AgsScriptSet *script_set,
		     AgsScriptVar *a,
		     AgsScriptArray *prime)
{
  //TODO:JK: implement me
}

void
ags_script_set_ggt(AgsScriptSet *script_set,
		   AgsScriptVar *a,
		   AgsScriptVar *b,
		   AgsScriptVar *ggt)
{
  //TODO:JK: implement me
}

void
ags_script_set_kgv(AgsScriptSet *script_set,
		   AgsScriptVar *a,
		   AgsScriptVar *b,
		   AgsScriptVar *kgv)
{
  //TODO:JK: implement me
}

void
ags_script_set_value(AgsScriptSet *script_set,
		     AgsScriptVar *value,
		     AgsScriptArray *lvalue)
{
  xmlNode *current;
  guchar *data;
  guint n_cols;
  gsize retlength;

  current = AGS_SCRIPT_OBJECT(script_set)->node->children;

  while(current != NULL){
    n_cols = strtoul(xmlGetProp(current, "length\0"), NULL, 10);

    switch(script_var->mode){
    case AGS_SCRIPT_ARRAY_INT16:
      {
	guchar *value;
	gint16 k;
	int c, mask;
	guint i, shift;
	
	data = (guchar *) malloc(n_cols * sizeof(gint16));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(gint16));

	mask = (int) (gint16) 0xffff;

	for(i = 0; i < sizeof(int) / sizeof(gint16) && i < n_cols; i++){
	  shift = i * sizeof(gint16);
	  mask = mask << shift;
	  c = mask & (k << shift);
	}

	memset(data, c, (size_t) ceil((n_cols * sizeof(gint16)) / sizeof(int)));
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT16:
      {
	guchar *value;
	guint16 k;
	int c, mask;
	guint i, shift;

	data = (guchar *) malloc(n_cols * sizeof(guint16));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(guint16));

	mask = (int) (guint16) 0xffff;

	for(i = 0; i < sizeof(int) / sizeof(guint16) && i < n_cols; i++){
	  shift = i * sizeof(guint16);
	  mask = mask << shift;
	  c = mask & (k << shift);
	}

	memset(data, c, (size_t) ceil((n_cols * sizeof(guint16)) / sizeof(int)));
      }
      break;
    case AGS_SCRIPT_ARRAY_INT32:
      {
	guchar *value;
	gint32 k;
	int c, mask;
	guint i, shift;

	data = (guchar *) malloc(n_cols * sizeof(gint32));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(gint32));

	mask = (int) (gint32) 0xffff;

	for(i = 0; i < sizeof(int) / sizeof(gint32) && i < n_cols; i++){
	  shift = i * sizeof(gint32);
	  mask = mask << shift;
	  c = mask & (k << shift);
	}

	memset(data, c, (size_t) ceil((n_cols * sizeof(gint32)) / sizeof(int)));
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT32:
      {
	guchar *value;
	guint32 k;
	int c, mask;
	guint i, shift;

	data = (guchar *) malloc(n_cols * sizeof(guint32));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(guint32));

	mask = (int) (guint32) 0xffff;

	for(i = 0; i < sizeof(int) / sizeof(guint32) && i < n_cols; i++){
	  shift = i * sizeof(guint32);
	  mask = mask << shift;
	  c = mask & (k << shift);
	}

	memset(data, c, (size_t) ceil((n_cols * sizeof(guint32)) / sizeof(int)));
      }
      break;
    case AGS_SCRIPT_ARRAY_INT64:
      {
	char *ptr;
	gint64 k, mask;
	char c;
	guint i, j, j_stop, shift;

	ptr =
	  data = (char *) malloc(n_cols * sizeof(gint64) * sizeof(char));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(gint64));

	j_stop = sizeof(gint64) / sizeof(char);

	mask = (gint64) 0xff;
	
	for(i = 0; i < n_cols; i++){
	  mask = (gint64) 0xff;

	  for(j = 0; j < j_stop; j++){
	    c = (mask & k) >> j; 

	    ptr[i * j_stop + j] = c;

	    if(j + 1 < j_stop){
	      mask = mask << sizeof(char);
	    }
	  }
	}
      }
      break;
    case AGS_SCRIPT_ARRAY_UINT64:
      {
	char *ptr;
	guint offset;
	guint64 k, mask;
	char c;
	guint i, j, j_stop, shift;

	ptr =
	  data = (char *) malloc(n_cols * sizeof(guint64) * sizeof(char));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(guint64));

	j_stop = sizeof(guint64) / sizeof(char);

	mask = (guint64) 0xff;
	
	for(i = 0; i < n_cols; i++){
	  mask = (gint64) 0xff;

	  for(j = 0; j < j_stop; j++){
	    c = (mask & k) >> j; 

	    ptr[i * j_stop + j] = c;

	    if(j + 1 < j_stop){
	      mask = mask << sizeof(char);
	    }
	  }
	}
      }
      break;
    case AGS_SCRIPT_ARRAY_DOUBLE:
      {
	gdouble *ptr;
	gdouble k;
	guint i;

	data = (char *) malloc(n_cols * sizeof(gdouble) * sizeof(guchar));
	ptr = (gdouble *) data;
	
	for(i = 0; i < n_cols; i++){
	  ptr[i] = k;
	}
      }
      break;
    case AGS_SCRIPT_ARRAY_CHAR:
      {
	guchar *value;
	char k;
	int c, mask;
	guint i, shift;
	
	data = (guchar *) malloc(n_cols * sizeof(char));

	value = g_base64_decode(xmlNodeGetContent(current), &retlength);
	memcpy(&k, value, sizeof(char));

	mask = (int) (char) 0xffff;

	for(i = 0; i < sizeof(int) / sizeof(char) && i < n_cols; i++){
	  shift = i * sizeof(char);
	  mask = mask << shift;
	  c = mask & (k << shift);
	}

	memset(data, c, (size_t) ceil((n_cols * sizeof(char)) / sizeof(int)));	
      }
      break;
    case AGS_SCRIPT_ARRAY_POINTER:
      {
	g_message("unsupported data type: gpointer\0");
      }
      break;
    }

    xmlNodeSetContent(current, g_base64_encode(data));
    free(data);

    current = current->next;
  }
}

void
ags_script_set_default_index(AgsScriptSet *script_set,
			     AgsScriptArray *index)
{
  AgsXmlScriptFactory *xml_script_factory;
  AgsScriptVar *index_value;
  xmlNode *current;
  guint i, j;
  guint n_rows, n_cols;
  guint offset;
  guint x, y;
  GError *error;

  xml_script_factory = AGS_SCRIPT(AGS_SCRIPT_OBJECT(script_set)->script)->xml_script_factory;
  error = NULL;

  index_value = ags_script_var_new();
  index_value->node = ags_xml_script_factory_map(xml_script_factory,
						 "ags-var\0",
						 &error);

  if(error != NULL){
    g_message("can't instantiate ags-var: %s\0", error->message);

    return;
  }

  n_rows = strtoul(xmlGetProp(AGS_SCRIPT_OBJECT(index)->node, "length\0"), NULL, 10);

  offset = 0;

  for(i = 0; i < n_rows; i++){
    current = ags_script_set_matrix_find_index(script_set,
					       index,
					       i);

    n_cols = strtoul(xmlGetProp(, "length\0"), NULL, 10);

    for(j = 0; j < n_cols; j++){
      xmlNodeSetContent(AGS_SCRIPT_OBJECT(index_value)->node, g_base64_encode(i * n_cols + j, sizeof(guint)));

      //TODO:JK: verify
      ags_script_set_matrix_put(script_set,
				index,
				index_value,
				floor((double) offset / 2.0) + ((offset % 2 == 1) ? j: i * n_cols),
				&x, &y);

      offset++;
    }
  }
}

void
ags_script_set_matrix_move_index(AgsScriptSet *script_set,
				 AgsScriptArray *matrix,
				 AgsScriptArray *index,
				 guint dest_x, guint dest_y,
				 guint src_x, guint src_y)
{
  AgsScriptVar *src, *dest;
  AgsScriptVar *src_index, *dest_index;
  xmlNode *current;
  guint src_offset, dest_offset;
  guint ncols;
  guint x, y;
  
  current = ags_script_set_matrix_find_index(script_set,
					     matrix,
					     dest_y);
  n_cols = strtoul(xmlGetProp(current, "length\0"), NULL, 10);

  //TODO:JK: verify
  /* move values */
  /* collect old fields */
  src_offset = src_y * n_cols + src_x;
  ags_script_set_matrix_get(script_set,
			    matrix,
			    src,
			    src_offset + ((src_offset % 2 == 1) ?
					  floor(-1.0 * (double) j / 2.0):
					  floor(-1.0 * (double) i / 2.0 * (double) n_cols)),
			    &x, &y);

  dest_offset = dest_y * n_cols + dest_x;
  ags_script_set_matrix_get(script_set,
			    matrix,
			    dest,
			    dest_offset + ((dest_offset % 2 == 1) ?
					   floor(-1.0 * (double) j / 2.0):
					   floor(-1.0 * (double) i / 2.0 * (double) n_cols)),
			    &x, &y);

  /* move indices */
  src_offset = src_y * n_cols + src_x;
  ags_script_set_matrix_put(script_set,
			    matrix,
			    dest,
			    src_offset + ((src_offset % 2 == 1) ?
					  floor(-1.0 * (double) j / 2.0):
					  floor(-1.0 * (double) i / 2.0 * (double) n_cols)),
			    &x, &y);
			    &x, &y);

  dest_offset = dest_y * n_cols + dest_x;
  ags_script_set_matrix_put(script_set,
			    matrix,
			    src,
			    dest_offset + ((dest_offset % 2 == 1) ?
					   floor(-1.0 * (double) j / 2.0):
					   floor(-1.0 * (double) i / 2.0 * (double) n_cols)),
			    &x, &y);

  /* move indices */
  /* collect old fields */
  //TODO:JK: verify
  src_offset = src_y * n_cols + src_x;
  ags_script_set_matrix_get(script_set,
			    index,
			    src_index,
			    src_offset + ((src_offset % 2 == 1) ?
					  floor(-1.0 * (double) j / 2.0):
					  floor(-1.0 * (double) i / 2.0 * (double) n_cols)),
			    &x, &y);

  dest_offset = dest_y * n_cols + dest_x;
  ags_script_set_matrix_get(script_set,
			    index,
			    dest_index,
			    dest_offset + ((dest_offset % 2 == 1) ?
					   floor(-1.0 * (double) j / 2.0):
					   floor(-1.0 * (double) i / 2.0 * (double) n_cols)),
			    &x, &y);


  /* move indices */
  src_offset = src_y * n_cols + src_x;
  ags_script_set_matrix_put(script_set,
			    index,
			    dest_index,
			    src_offset + ((src_offset % 2 == 1) ?
					  floor(-1.0 * (double) j / 2.0):
					  floor(-1.0 * (double) i / 2.0 * (double) n_cols)),
			    &x, &y);

  dest_offset = dest_y * n_cols + dest_x;
  ags_script_set_matrix_put(script_set,
			    index,
			    src_index,
			    dest_offset + ((dest_offset % 2 == 1) ?
					   floor(-1.0 * (double) j / 2.0):
					   floor(-1.0 * (double) i / 2.0 * (double) n_cols)),
			    &x, &y);
}

void
ags_script_set_matrix_sort(AgsScriptSet *script_set,
			   AgsScriptArray *matrix,
			   gboolean sort_column, gboolean sort_row,
			   AgsScriptArray *index, guint depth,
			   guint *x, guint *y)
{
  xmlNode *current_matrix, *current_index;
  guchar *matrix_data, *index_data;
  guint *index_ptr;
  guint n_cols;
  guint offset, stop;
  guint i, j, k;

  current_matrix = ags_script_set_matrix_find_index(script_set,
						    matrix,
						    y);
  current_index = ags_script_set_matrix_find_index(script_set,
						   index,
						   y);

  offset = 0;

  /* depth^2 / 2.0 - 1 */
  //TODO:JK: verify
  stop = (guint) exp(log((double) depth) / log((double) 2));

  for(i = 0; i < depth && current != NULL;){
    matrix_data = g_base64_decode(xmlNodeGetContent(current));
    index_data = g_base64_decode(xmlNodeGetContent(current));

    index_ptr = (guint) index_data;

    /*  */
    //TODO:JK: verify
    n_cols = strtoul(xmlGetProp(current, "length\0"), NULL, 10);

    for(k = 0; k < depth - i && offset < stop; k++){
      for(j = 0; j < (guint) ceil((double) depth / (double) k) && offset < stop; j++, i++, offset++){
	if(j + x >= n_cols){
	  break;
	}

	switch(script_array->mode){
	case AGS_SCRIPT_ARRAY_INT16:
	  {
	    gint16 *matrix_ptr;

	    matrix_ptr = (gint16 *) matrix_data;

	    if(matrix_ptr > ){
	    }

	  }
	  break;
	case AGS_SCRIPT_ARRAY_UINT16:
	  {
	  }
	  break;
	case AGS_SCRIPT_ARRAY_INT32:
	  {
	  }
	  break;
	case AGS_SCRIPT_ARRAY_UINT32:
	  {
	  }
	  break;
	case AGS_SCRIPT_ARRAY_INT64:
	  {
	  }
	  break;
	case AGS_SCRIPT_ARRAY_UINT64:
	  {
	  }
	  break;
	case AGS_SCRIPT_ARRAY_DOUBLE:
	  {
	  }
	  break;
	case AGS_SCRIPT_ARRAY_CHAR:
	  {
	    g_message("unsupported data type: gchar\0");
	  }
	  break;
	case AGS_SCRIPT_ARRAY_POINTER:
	  {
	    g_message("unsupported data type: gpointer\0");
	  }
	  break;
	}
      }
    }
    
    xmlNodeSetContent(current_index, g_base64_encode(index_data));
    free(index_data);

    xmlNodeSetContent(current_matrix, g_base64_encode(matrix_data));
    free(matrix_data);

    current_index = current_index->next;
    current_matrix = current_matrix->next;
  }
}

void
ags_script_set_matrix_sort_by_operands(AgsScriptSet *script_set,
				       AgsScriptArray *matrix,
				       AgsScriptArray *operands,
				       AgsScriptArray *index, guint depth,
				       guint *x, guint *y)
{

}


void
ags_script_set_matrix_vector_mirror(AgsScriptSet *script_set,
				    AgsScriptArray *matrix,
				    AgsScriptArray *vector)
{

}

void
ags_script_set_equation(AgsScriptSet *script_set,
			AgsScriptArray *factorized,
			AgsScriptArray *operands)
{

}

gboolean
ags_script_set_boolean_term(AgsScriptSet *script_set)
{
  gboolean term;

  term = FALSE;

  //TODO:JK: implement me

  return(term);
}

gint
ags_script_set_int_term(AgsScriptSet *script_set)
{
  gint term;

  term = -1;

  //TODO:JK: implement me

  return(term);
}

guint
ags_script_set_uint_term(AgsScriptSet *script_set)
{
  guint term;

  term = 0;

  //TODO:JK: implement me

  return(term);
}

gdouble
ags_script_set_double_term(AgsScriptSet *script_set)
{
  gdouble term;

  term = 1.0;

  //TODO:JK: implement me

  return(term);
}


AgsScriptSet*
ags_script_set_new()
{
  AgsScriptSet *script_set;

  script_set = (AgsScriptSet *) g_object_new(AGS_TYPE_SCRIPT_SET,
					     NULL);

  return(script_set);
}
