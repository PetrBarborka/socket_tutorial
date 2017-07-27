
#include <stdio.h>
#include <stdlib.h>
#include "../src/dbg.h"
#include "../lib/header/bstrlib.h"

bstring exec_command(bstring command){ 

  FILE * fp = NULL;
  bstring file_contents = NULL;

  int rc = bcatcstr(command, " 2>&1");
  check( rc == BSTR_OK, "failed to cat bstring");
  fp = popen( bdata(command), "r" );

  file_contents = bread( (bNread)fread, fp );
  printf("%s", bdata(file_contents));

  fclose(fp); fp = NULL;
  
  return file_contents;
error:
  if(fp) fclose(fp);
  if(file_contents) bdestroy(file_contents);
  return NULL;
}

int main(int argc, char * argv[]){

  bstring cmdbuff = NULL;
  bstring file_contents = NULL;

  check( argc > 1, "USAGE: %s <filename>", argv[0]);

  cmdbuff = blk2bstr(argv[1], strlen(argv[1]));

  file_contents = exec_command(cmdbuff);
  printf("%s", bdata(file_contents));

  bdestroy(file_contents); file_contents = NULL;
  bdestroy(cmdbuff); cmdbuff = NULL;

  return 0;
error:
  if(cmdbuff) bdestroy(cmdbuff);
  if(file_contents) bdestroy(file_contents);

  return -1;
}
