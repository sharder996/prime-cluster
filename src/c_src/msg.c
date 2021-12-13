#include <gmp.h>
#include <math.h>
#include <json-c/json.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "msg.h"


/* Prototypes */
char* build_msg(char* mark, mpz_t size);
int compress_msg(char** msg);
job* parse_job(char* msg);
char* build_result(job* task, char* results);


/* Functions */
char* build_msg(char* mark, mpz_t size)
{
  mpz_t temp;

  mpz_init_set_ui(temp, 0);
  
  char* msg = (char*)malloc(sizeof(char)*mpz_get_ui(size));
  char* position = msg;
  int length = 0;

  int j = 0;
  for (int i = 0; mpz_cmp(temp, size) < 0; mpz_add_ui(temp, temp, 1))
  {
    if (mark[mpz_get_ui(temp)] == '1')
      i += 1;
    else if (mark[mpz_get_ui(temp)] == '0')
    {
      sprintf(position, "%d|", i);
      position += (int)(floor(log10(abs(i))) + 1) + 1;

      length += (int)(floor(log10(abs(i))) + 1) + 1;
      i = 1;
      j++;
    }
  }
  *--position = '\0';

  mpz_clear(temp);

  return msg;
}

#define windowBits 15
#define GZIP_ENCODING 16

int compress_msg(char** msg)
{
  unsigned char* out = (unsigned char*)malloc(sizeof(unsigned char)*strlen(*msg));

  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  
  deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits | GZIP_ENCODING, 8,
                Z_DEFAULT_STRATEGY);

  strm.avail_in = strlen(*msg);
  strm.next_in = (unsigned char*)*msg;

  strm.avail_out = strlen(*msg);
  strm.next_out = out;
  deflate(&strm, Z_FINISH);
  int len = strlen(*msg) - strm.avail_out;

  deflateEnd(&strm);

  free(*msg);
  *msg = (char*)out;
  return len;
}

job* parse_job(char* msg)
{
  job* task = (job*)malloc(sizeof(job));
  json_object* task_json = json_tokener_parse(msg);
  json_object* value;

  json_object_object_get_ex(task_json, "op", &value);
  task->op = json_object_get_string(value);

  if (!strcmp(task->op, QUIT))
    return task;

  json_object_object_get_ex(task_json, "upper_limit", &value);
  task->upper_limit = json_object_get_string(value);
  
  json_object_object_get_ex(task_json, "lower_limit", &value);
  task->lower_limit = json_object_get_string(value);
  
  json_object_object_get_ex(task_json, "rank", &value);
  task->job_no = json_object_get_int(value);

  json_object_put(value);

  return task;
}

char* build_result(job* task, char* results)
{
  json_object* msg = json_object_new_object();

  json_object_object_add(msg, "op", json_object_new_string(SUCCESS));
  json_object_object_add(msg, "upper_limit", json_object_new_string(task->upper_limit));
  json_object_object_add(msg, "lower_limit", json_object_new_string(task->lower_limit));
  json_object_object_add(msg, "rank", json_object_new_int(task->job_no));
  json_object_object_add(msg, "payload", json_object_new_string(results));

  return (char*)json_object_to_json_string(msg);
}
