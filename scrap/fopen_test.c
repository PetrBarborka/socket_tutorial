#include <stdio.h>
#include <stdlib.h>
#include "../src/dbg.h"

int main(int argc, char * argv[]){

  check( argc > 1, "USAGE: %s <filename>", argv[0]);
  FILE * fp = fopen(argv[1], "r");
  char * file_contents = calloc(sizeof(char), 4096);
  ssize_t bytes_read = fread(file_contents, 1, 4096, fp);
  printf("%zd bytes:\n %s", bytes_read, file_contents);
  free(file_contents); file_contents = NULL;

  return 0;
error:
  return -1;
}
