// void print_commits_info(const char *username, const char *repo) {
//     JSON_Value *root_value;
//     JSON_Array *commits;
//     JSON_Object *commit;
//     size_t i;

//     char curl_command[512];
//     char cleanup_command[256];
//     char output_filename[] = "commits.json";

//     /* it ain't pretty, but it's not a libcurl tutorial */
//     sprintf(curl_command,
//         "curl -s \"https://api.github.com/repos/%s/%s/commits\" > %s",
//         username, repo, output_filename);
//     sprintf(cleanup_command, "rm -f %s", output_filename);
//     system(curl_command);

//     /* parsing json and validating output */
//     root_value = json_parse_file(get_file_path(output_filename));
//     if (json_value_get_type(root_value) != JSONArray) {
//         system(cleanup_command);
//         return;
//     }

//     /* getting array from root value and printing commit info */
//     commits = json_value_get_array(root_value);
//     printf("%-10.10s %-10.10s %s\n", "Date", "SHA", "Author");
//     for (i = 0; i < json_array_get_count(commits); i++) {
//         commit = json_array_get_object(commits, i);
//         printf("%.10s %.10s %s\n",
//                json_object_dotget_string(commit, "commit.author.date"),
//                json_object_get_string(commit, "sha"),
//                json_object_dotget_string(commit, "commit.author.name"));
//     }

//     /* cleanup code */
//     json_value_free(root_value);
//     system(cleanup_command);

    
// void test_suite_9(void) {
//     const char *filename = "test_2_pretty.txt";
//     const char *temp_filename = "test_2_serialized_pretty.txt";
//     char *file_contents = NULL;
//     char *serialized = NULL;
//     JSON_Value *a = NULL;
//     JSON_Value *b = NULL;
//     size_t serialization_size = 0;
//     a = json_parse_file(get_file_path(filename));
//     TEST(json_serialize_to_file_pretty(a, get_file_path(temp_filename)) == JSONSuccess);
//     b = json_parse_file(get_file_path(temp_filename));
//     TEST(json_value_equals(a, b));
//     remove(temp_filename);
//     serialization_size = json_serialization_size_pretty(a);
//     serialized = json_serialize_to_string_pretty(a);
//     TEST((strlen(serialized)+1) == serialization_size);

//     file_contents = read_file(get_file_path(filename));

//     TEST(STREQ(file_contents, serialized));
// }

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
    config_save_defaults();
  }

  result = 1;
end:
  return result;
}

int config_save_defaults()
{
  json_value_free(g_root);

  /*
    Create a new json object with what we want...
  */

  json_serialize_to_file_pretty(g_root, g_filename);

  /*
    We can continue as normal from here 
           as long as no errors occured.
    
    No need to free the new root as it can be reused.
  */

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

  if (!config_validate_value(g_root, "status"         , JSONString) ||
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

/* Returns a copy, This needs to be freed after use. */
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

  result = calloc((json_value_get_string_len(value)), sizeof(char));

  strncpy(result,
          json_value_get_string(value),
          json_value_get_string_len(value));
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

  result = calloc((json_value_get_string_len(value)), sizeof(char));

  strncpy(result,
          json_value_get_string(value),
          json_value_get_string_len(value));
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

  result = calloc((json_value_get_string_len(value)), sizeof(char));

  strncpy(result,
          json_value_get_string(value),
          json_value_get_string_len(value));
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