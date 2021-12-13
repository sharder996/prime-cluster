#ifndef MSG_H
#define MSG_H

#include <gmp.h>


/* DEFINITIONS */
#define QUIT "quit"
#define SUCCESS "success"


/* TYPES */
typedef struct
{
  const char* op;
  const char* upper_limit;
  const char* lower_limit;
  unsigned int job_no;
} job;

typedef struct
{
  const char* op;
  const char* upper_limit;
  const char* lower_limit;
  unsigned int job_no;
  const char* payload;
} result;


/* FUNCTIONS */
char* build_msg(char* mark, mpz_t size);
int compress_msg(char** msg);
job* parse_job(char* msg);
char* build_result(job* task, char* results);


#endif // MSG_H
