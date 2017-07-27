#include "header/files.h"
#include "header/bstrlib.h"

#include <wordexp.h>
#include <stdlib.h>
#include <glob.h>
#include <sys/stat.h>

/*#define NDEBUG */
#include "dbg.h" 

#define MAX_FILES 1024

struct bstrList * bstrListAppend(struct bstrList * list, bstring b){

  check(b != NULL, "NULL b");
  check(list != NULL, "NULL list");
  check(list->qty < list->mlen, "out of space, allocate more");
  list->entry[list->qty] = b;
  list->qty++;

  return list;
error:
  return NULL;
}

FILE * fopen_expand(bstring fname, char * mode){

  // expand config filename
  wordexp_t exp_result;
  int rc = wordexp(bdata(fname), &exp_result, 0);
  check(rc == 0, "Wordexp failed for file name %s", bdata(fname));
  check(exp_result.we_wordc > 0, "Invalid expansion");
  char * filename = exp_result.we_wordv[0];
  debug("opening file %s", filename);
  FILE * fp = fopen(filename, mode);
  check(fp, "Failed to open file %s", filename);
  wordfree(&exp_result);

  return fp;
error:
  return NULL;
}

struct bstrList * getLines(FILE * fp){

  struct bstrList * list = bstrListCreate();
  bstrListAlloc( list, MAX_FILES );
  char * line = NULL;
  size_t len = 0;

  while(getline(&line, &len, fp) != -1){
    if( line[strlen(line) - 1] == '\n' ) line[strlen(line) -1] = '\0';
    list = bstrListAppend( list, bfromcstr(line) );
    check( list != NULL, "failed to append to list" );
  }
  free(line); line = NULL; len = 0;


  return list;

error:
  return NULL;

}

struct bstrList * glob_files(struct bstrList * files){

  struct bstrList * list = bstrListCreate();
  check_mem(list);
  bstrListAlloc(list, MAX_FILES);
  
  glob_t glob_buf;

  int i = 0;
  if (files->qty > 0) glob(bdata(files->entry[i]), GLOB_TILDE, NULL, &glob_buf);
  for( i = 1; i < files->qty; i++ ){
    debug("line[%d]: [%s]", i, bdata(files->entry[i]));
    glob(bdata(files->entry[i]), GLOB_TILDE| GLOB_APPEND, NULL, &glob_buf);
  }

  debug("%lu globbed files:\n", glob_buf.gl_pathc);
  for(size_t i = 0; i < glob_buf.gl_pathc; i++){
    struct stat stat_buf;

    char * curr_file = glob_buf.gl_pathv[i];
    int status = stat(curr_file, &stat_buf);
    check(status >= 0, "File status check failed");
    if ( S_ISREG(stat_buf.st_mode) ) {
      check(list->qty < MAX_FILES, "Too many files");
      debug("file\t%s\n", curr_file);
      list = bstrListAppend(list, bfromcstr(curr_file));
    } else {
      debug("dir\t%s\n", curr_file);
    }
  }

  globfree(&glob_buf);

  return list;
error:

  globfree(&glob_buf);
  bstrListDestroy(list);
  return NULL;
}

//given filename of configfile, glob out names of individual files
//matching patterns it contains
struct bstrList * get_files_to_read(const char * configfile){

  FILE * fp = NULL;

  check( configfile != NULL, "NULL config file!" );
  bstring fname = blk2bstr(configfile, strlen(configfile));
  fp = fopen_expand(fname, "r");
  bdestroy(fname);
  check_mem(fp);


  struct bstrList * lines = getLines(fp);
  check( lines != NULL, "Failed to read lines");
  fclose(fp); fp = NULL;

  struct bstrList * list = glob_files(lines);
  check( list != NULL, "failed to glob files" );
  bstrListDestroy(lines);

  return list;

error:
  if(fp) {
    fclose(fp);
    fp = NULL;
  }

  return NULL;
}
