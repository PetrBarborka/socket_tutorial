#ifndef __files__h__
#define __files__h__

#include <stdio.h>
#include "bstrlib.h"

// ----- files.h ------
// which files to read and a function to do so

struct bstrList * bstrListAppend(struct bstrList * list, bstring b);

FILE * fopen_expand(bstring fname, char * mode);

struct bstrList * getLines(FILE * fp);

struct bstrList * glob_files(struct bstrList * files);

/*
 * unpack the files in configfile into files using glob
 * param:
 *   configfile - path to text file where each line is
 *                a glob pattern
 *   files      - pointer to an array of absolute file
 *                paths expanded from configfile contents
 * returns:
 *   number of files in files, ERROR: -1 
 */
struct bstrList * get_files_to_read(const char * configfile);

#endif

