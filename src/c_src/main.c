#include <gmp.h>
#include <limits.h>
#include <math.h>
#include <omp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "client.h"
#include "ini.h"
#include "msg.h"
#include "prime.h"


/* GLOBALS */
int client;


/* PROTOTYPES */
static void handle_sigterm(int sig);
static int handler(void* user, const char* section, const char* name, const char* value);


/* TYPES */
typedef struct
{
  const char* server_address;
  int port;
  int retry_count;
  int thread_count;
} configuration;


/* FUNCTIONS */
static void handle_sigterm(int sig)
{
  printf("Exiting client...\n");
  if (client)
    shutdown_sock(client);
  exit(EXIT_SUCCESS);
}

static int handler(void* user, const char* section, const char* name, const char* value)
{
  configuration* pconfig = (configuration*)user;

  if (strcmp(name, "server_address") == 0)
    pconfig->server_address = strdup(value);
  else if (strcmp(name, "port") == 0)
    pconfig->port = atoi(value);
  else if (strcmp(name, "retry_count") == 0)
    pconfig->retry_count = atoi(value);
  else if (strcmp(name, "thread_count") == 0)
    pconfig->thread_count = atoi(value);
  else
    return 0;
  return 1;
}

int main()
{
  clock_t start, end;
  char* sieve;
  char* msg;
  char* prime_results;
  job* task;
  configuration config;
  mpz_t size, lower, upper;
  int retry_count;

  // Load the configuration file
  if (ini_parse("config.ini", handler, &config) < 0)
  {
    printf("Unable to load 'config.ini'\n");
    return EXIT_FAILURE;
  }
  printf("Config loaded from 'config.ini'\n");

  // Initialize the socket and connect to the server
  client = init_sock(config.server_address, config.port, config.retry_count);

  mpz_init_set_ui(size, 0);
  omp_set_num_threads(config.thread_count);

  signal(SIGINT, handle_sigterm);
  signal(SIGTERM, handle_sigterm);

  // Begin the main program loop
  while (1)
  {
    retry_count = config.retry_count;
    // Receive a job from the server
    while (!recv_msg(client, &msg) && retry_count)
    {
      retry_count--;
      sleep(5);
    }
    if (retry_count < 0)
    {
      printf("Server timed out. Exiting...\n");
      break;
    }
    retry_count = config.retry_count;

    // Convert the job from json to job struct
    task = parse_job(msg);
    // Shutdown the client if received a quit command
    if (!strcmp(task->op, QUIT))
    {
      shutdown_sock(client);
      break;
    }

    mpz_init_set_str(upper, task->upper_limit, 10);
    mpz_init_set_str(lower, task->lower_limit, 10);
    mpz_sub(size, upper, lower);

    // Begin the computation
    start = clock();

    printf("Finding primes from %s to %s...", task->lower_limit, task->upper_limit);
    fflush(stdout);
    sieve = sieve_of_eratosthenes(task->lower_limit, task->upper_limit);

    // Build the message from the results
    prime_results = build_msg(sieve, size);
    // compress_msg(prime_results);
    free(msg);
    msg = build_result(task, prime_results);
    int len = compress_msg(&msg);

    end = clock();
    printf("Done (%lf sec(s))\n", ((double) (end - start)) / CLOCKS_PER_SEC);

    // Send the results back to the server
    while (!send_msg(client, msg, len) && retry_count)
    {
      retry_count--;
      sleep(5);
    }
    if (retry_count < 0)
    {
      printf("Server timed out. Exiting...\n");
      break;
    }

    // Free the resources that were allocated
    free(sieve);
    free(msg);
    free(task);
  }

  mpz_clear(size);
  free(msg);
  free(task);

  return EXIT_SUCCESS;
}
