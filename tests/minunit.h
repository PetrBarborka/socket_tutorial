//#undef NDEBUG
#ifndef __munit_h__
#define __munit_h__

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "dbg.h"

#define mu_suite_start() char * message = NULL

#define mu_assert(test, message) \
  if(!(test)){ \
    log_err(message); \
    return message; \
  }

#define mu_run_test(test) \
  debug("\n-----%s", " " #test); \
  message = test(); \
  tests_run++; \
  if( message ) return message;

#ifdef BENCHMARK
#define mu_benchmark_test(test, reps) \
  debug("----- benchmarking %s ", " " #test); \
  printf("----- benchmarking %s \n", " " #test); \
  diff = 0; \
  start = clock(); \
  for( int i = 0; i < reps; i++ ){ \
    message = test(); \
  } \
  if( message ) return message; \
  diff = (clock() - start); \
  debug("----- %d reps took %ld [clocks] = %f [s]", \
        reps, diff, diff/(double)CLOCKS_PER_SEC); \
  printf("----- %d reps took %ld [clocks] = %f [s]\n", \
         reps, diff, diff/(double)CLOCKS_PER_SEC); 
#else
#define mu_benchmark_test(test, reps)
#endif
  

#define RUN_TESTS(name) \
  int main(int argc, char * argv[]){ \
    argc = 1; \
    /* bypass unused warning */ \
    (void)argc; \
    debug("----- RUNNING %s", argv[0]); \
    printf("-----\nRUNNING %s\n", argv[0]); \
    char * result = name(); \
    if (result != 0){ \
      printf( "FAILED %s\n", result);\
    } else { \
      printf( "ALL TESTS PASSED!\n" );\
    }\
    printf("Tests run: %d\n", tests_run); \
    return(result != 0); \
  }

int tests_run;
clock_t start, diff;

#endif // __munit_h__
