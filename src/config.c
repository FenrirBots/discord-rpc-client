#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parson.h"
#include "config.h"


JSON_Value *g_root     = NULL;
char       *g_filename = NULL;

void config_shutdown();


void config_init(
  char *filename)
{
  g_filename = filename;

  atexit(config_shutdown);
}

void config_shutdown()
{
  if (NULL != g_root)
  {
    json_value_free(g_root);
  }
}

int config_load()
{
  int result = 0;

  if (NULL == g_filename)
  {
    goto end;
  }

  g_root = json_parse_file_with_comments(g_filename);
  
  if (NULL == g_root)
  {
    printf("No configuration file was found\n\n");
    config_save_defaults();
    goto end;
  }

  result = 1;
end:
  return result;
}

int config_save_defaults()
{
  JSON_Object *object;
  JSON_Value  *value;
  JSON_Array  *array;
  int          iter;

  json_value_free(g_root);
  
  g_root = json_value_init_object();
  object = json_value_get_object(g_root);

  json_object_set_string_with_len(object, "state"          , "Playing <insert game here>", 0);
  json_object_set_string_with_len(object, "image-small"    , "default", 0);
  json_object_set_string_with_len(object, "image-large"    , "default", 0);
  json_object_set_string_with_len(object, "party-id"       , "", 0);
  json_object_set_string_with_len(object, "privacy"        , "public", 0);
  
  json_object_set_number(object, "party-current"  , 1);
  json_object_set_number(object, "party-maximum"  , 4);
  json_object_set_number(object, "timestamp-start", 0);
  json_object_set_number(object, "timestamp-end"  , 0);

  json_object_set_value(object, "secrets", json_value_init_object());
  object = json_object_get_object(object, "secrets");

  json_object_set_string_with_len(object, "application-id", "0000000000000000", 16);
  json_object_set_string_with_len(object, "match"         , "secret", 6);
  json_object_set_string_with_len(object, "join"          , "secret", 6);
  json_object_set_string_with_len(object, "spectate"      , "secret", 6);

  object = json_value_get_object(g_root);
  json_object_set_value(object, "buttons", json_value_init_array());
  
  array = json_object_get_array(object, "buttons");
  json_array_append_value(array, json_value_init_object());
  json_array_append_value(array, json_value_init_object());

  for (iter = 0; iter < 2; iter++)
  {
    value  = json_array_get_value(array, iter);
    object = json_value_get_object(value);

    json_object_set_string_with_len(object, "message", "Example Button", 0);
    json_object_set_string_with_len(object, "uri"    , "https://example.com", 0);
  }

  json_serialize_to_file_pretty(g_root, g_filename);
  printf("A new configuration file has been generated\n");
  printf("   Change the configuration and run this program again\n");

  return 0;
}

int config_save()
{
  return 1;
}

int config_validate_value(
  JSON_Value *v,
  char       *k,
  int         t)
{
  JSON_Object *object = NULL;
  JSON_Value  *value  = NULL;
  int          result = 0;

  object = json_value_get_object(v);
  value  = json_object_get_value(object, k);

  if (value == NULL)
  {
    printf("the key '%s' is missing from '%s'\n", k, g_filename);
    goto end;
  }

  if (t != json_value_get_type(value))
  {
    printf("the key '%s' was found but is not of type '%s'\n", k, json_type_to_string(t));
    goto end;
  }

  result = 1;
end:
  return result;
}

int config_validate()
{
  JSON_Object *object = NULL;
  JSON_Value  *value  = NULL;
  JSON_Array  *array  = NULL;
  int          iter   = 0;
  int          result = 0;

  if (!config_validate_value(g_root, "state"          , JSONString) ||
      !config_validate_value(g_root, "image-small"    , JSONString) ||
      !config_validate_value(g_root, "image-large"    , JSONString) ||
      !config_validate_value(g_root, "party-id"       , JSONString) ||
      !config_validate_value(g_root, "party-current"  , JSONNumber) ||
      !config_validate_value(g_root, "party-maximum"  , JSONNumber) ||
      !config_validate_value(g_root, "timestamp-start", JSONNumber) ||
      !config_validate_value(g_root, "timestamp-end"  , JSONNumber) ||
      !config_validate_value(g_root, "privacy"        , JSONString) ||
      !config_validate_value(g_root, "secrets"        , JSONObject))
  {
    goto end;
  }

  object = json_value_get_object(g_root);
  value  = json_object_get_value(object, "secrets");

  if (!config_validate_value(value, "application-id", JSONString) ||
      !config_validate_value(value, "match"         , JSONString) ||
      !config_validate_value(value, "join"          , JSONString) ||
      !config_validate_value(value, "spectate"      , JSONString))
  {
    goto end;
  }

  array = json_object_get_array(object, "buttons");

  if (json_array_get_count(array) < 0x02)
  {
    goto end;
  }

  for (iter = 0; iter < 2; iter++)
  {
    value = json_array_get_value(array, iter);
    
    if (!config_validate_value(value, "message", JSONString) ||
        !config_validate_value(value, "uri"    , JSONString))
    {
      goto end;
    }
  }
  
  result = 1;
end:
  return result;
}

char *
config_get_string(
  char *key)
{
  JSON_Object *object = NULL;
  JSON_Value  *value  = NULL;
  char        *result = NULL;

  object = json_value_get_object(g_root);
  value  = json_object_get_value(object, key);

  if (json_value_get_type(value) != JSONString)
  {
    goto end;
  }

  if (json_value_get_string_len(value) == 0x00)
  {
    goto end;
  }

  result = json_value_get_string(value);
end:
  return result;
}

double
config_get_number(
  char *key)
{
  JSON_Object *object = NULL;
  JSON_Value  *value  = NULL;
  double       result = 0;

  object = json_value_get_object(g_root);
  value  = json_object_get_value(object, key);

  if (json_value_get_type(value) != JSONNumber)
  {
    goto end;
  }

  result = json_value_get_number(value);
end:
  return result;
}

int
config_get_boolean(
  char *key)
{
  JSON_Object *object = NULL;
  JSON_Value  *value  = NULL;
  int          result = 0;

  object = json_value_get_object(g_root);
  value  = json_object_get_value(object, key);

  if (json_value_get_type(value) != JSONBoolean)
  {
    goto end;
  }

  result = json_value_get_boolean(value);
end:
  return result;
}

char *
config_get_secret(
  char *key)
{
  JSON_Object *object = NULL;
  JSON_Value  *value  = NULL;
  char        *result = NULL;

  object = json_value_get_object(g_root);
  value  = json_object_get_value(object, "secrets");

  if (json_value_get_type(value) != JSONObject)
  {
    goto end;
  }

  object = json_value_get_object(value);
  value  = json_object_get_value(object, key);

  if (json_value_get_type(value) != JSONString)
  {
    goto end;
  }

  if (json_value_get_string_len(value) == 0x00)
  {
    goto end;
  }

  result = json_value_get_string(value);
end:
  return result;
}

char *
config_get_button(
  int   index,
  char *key)
{
  JSON_Object *object = NULL;
  JSON_Value  *value  = NULL;
  JSON_Array  *array  = NULL;
  char        *result = NULL;

  if (index < 0)
  {
    index = 0;
  }

  if (index > 1)
  {
    index = 1;
  }

  object = json_value_get_object(g_root);
  array  = json_object_get_array(object, "buttons");

  if (array == NULL)
  {
    goto end;
  }

  if (json_array_get_count(array) <  index ||
      json_array_get_count(array) == 0x00)
  {
    goto end;
  }

  value  = json_array_get_value(array, index);
  object = json_value_get_object(value);
  value  = json_object_get_value(object, key);

  if (json_value_get_type(value) != JSONString)
  {
    goto end;
  }

  if (json_value_get_string_len(value) == 0x00)
  {
    goto end;
  }

  result = json_value_get_string(value);
end:
  return result;
}

_Bool config_set_string(
  char *key,
  char *value)
{
  return 0;
}

_Bool config_set_integer(
  char  *key,
  double value)
{
  return 0;
}

_Bool config_set_boolean(
  char *key,
  _Bool value)
{
  return 0;
}

_Bool config_set_secret(
  char *key,
  char *value)
{
  return 0;
}