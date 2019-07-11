/*
  Copyright:  Copyright (c) MOSEK ApS, Denmark. All rights reserved.

  File:       opt_server_async.c

  Purpose :   Demonstrates how to use MOSEK OptServer
              to solve optimization problem asynchronously
*/
#include "mosek.h"
#ifdef _WIN32
#include "windows.h"
#else
#include "unistd.h"
#endif

static void MSKAPI printstr(void *handle, const char str[])
{
  printf("%s", str);
}

int main(int argc, char * argv[])
{

  char token[33];

  int         numpolls = 10;
  int         i = 0;

  MSKbooleant respavailable;

  MSKenv_t    env   = NULL;
  MSKtask_t   task  = NULL;

  MSKrescodee res   = MSK_RES_OK;
  MSKrescodee trm;
  MSKrescodee resp;

  const char * filename = "../data/25fv47.mps";
  const char * host     = "karise";
  const char * port     = "30080";

  if (argc < 5)
  {
    fprintf(stderr, "Syntax: opt_server_async filename host port numpolls\n");
    return 0;
  }

  if (argc > 1) filename = argv[1];
  if (argc > 2) host     = argv[2];
  if (argc > 2) port     = argv[3];
  if (argc > 4) numpolls = atoi(argv[4]);

  res = MSK_makeenv(&env, NULL);

  if (res == MSK_RES_OK)
    res = MSK_maketask(env, 0, 0, &task);
  if (res == MSK_RES_OK)
    res = MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

  if (res == MSK_RES_OK)
    res = MSK_readdata(task, filename);

  res = MSK_asyncoptimize(task,
                          host,
                          port,
                          token);
  MSK_deletetask(&task);
  printf("token = %s\n", token);

  if (res == MSK_RES_OK)
    res = MSK_maketask(env, 0, 0, &task);

  if (res == MSK_RES_OK)
    res = MSK_readdata(task, filename);

  if (res == MSK_RES_OK)
    res = MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

  for (i = 0; i < numpolls &&  res == MSK_RES_OK ; i++)
  {
#if __linux__
    sleep(1);
#elif defined(_WIN32)
    Sleep(1000);
#endif

    printf("poll %d\n ", i);


    res = MSK_asyncpoll(task,
                        host,
                        port,
                        token,
                        &respavailable,
                        &resp,
                        &trm);

    puts("polling done");

    if (respavailable)
    {
      puts("solution available!");

      res = MSK_asyncgetresult(task,
                               host,
                               port,
                               token,
                               &respavailable,
                               &resp,
                               &trm);

      MSK_solutionsummary(task, MSK_STREAM_LOG);
      break;
    }

  }


  if (i == numpolls)
  {
    printf("max num polls reached, stopping %s", host);
    MSK_asyncstop(task, host, port, token);
  }

  MSK_deletetask(&task);
  MSK_deleteenv(&env);

  printf("%s:%d: Result = %d\n", __FILE__, __LINE__, res); fflush(stdout);

  return res;
}