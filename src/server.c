/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include "dbg.h"
#include "../lib/header/bstrlib.h"
#include "../lib/header/files.h"

#include "../lib/header/sockhelplib.h"


int print_message_and_reply(int client_sockfd, struct bstrList * tokens)
{
  bstring space = bfromcstr(" ");
  bstring joined = bjoin(tokens, space);
  printf( "received message: %s", bdata(joined) );

  bstring message = bfromcstr("message received");
  int rc = write_to_socket( client_sockfd, message );
  check( rc != -1, "socket write failed" );

  bdestroy(space); space = NULL;
  bdestroy(message); message = NULL;
  bdestroy(joined);

  return 0;
error:
  return -1;
}

int ls_server_directory(int client_sockfd)
{
  debug( "doing ls" );
  bstring here = bfromcstr("*");
  check_mem(here);
  struct bstrList * list = bstrListCreate();  
  check_mem(list);
  list = bstrListAppend(list, here);
  struct bstrList * ls = glob_files(list);
  check_mem(ls);
  bstring space = bfromcstr(" ");
  bstring joined = bjoin(ls, space);
  int rc = write_to_socket( client_sockfd, joined );

  //cleanup
  bdestroy(here); here = NULL;
  bdestroy(space); space = NULL;
  bdestroy(joined); joined = NULL;
  bstrListDestroy(list); list = NULL;
  bstrListDestroy(ls); ls = NULL;

  return rc;
error:
  return -1;
}

int send_file(int client_sockfd, struct bstrList * tokens)
{
  check( tokens->qty = 3, "provide a filename" );
  char * filename = bdata(tokens->entry[2]);
  size_t fname_len = strlen(filename);
  filename[fname_len - 1] = '\0';
  FILE * fp = fopen(filename, "r");
  check(fp != NULL, "Failed to open file [%s]", 
        filename);
  bstring file_contents = bread((bNread)fread, fp);
  int rc = write_to_socket( client_sockfd, file_contents );
  check( rc != -1, "failed to write to socket" );
  bdestroy(file_contents);
  fclose(fp);

  return rc;
error:
  return -1;
}

bstring exec_command(bstring command)
{ 

  FILE * fp = NULL;
  bstring file_contents = NULL;

  int rc = bcatcstr(command, " 2>&1");
  check( rc == BSTR_OK, "failed to cat bstring");
  debug( "command: %s", bdata(command) );
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

int main(int argc, char *argv[])
{

  check( argc == 2, "Usage: %s <port>", argv[0]);

  int rc = -1;

  int port = atoi( argv[1] );

  int sockfd = bind_and_listen( port );
  check( sockfd != -1, "ERROR binding & listening" );

  // accept client connection 
  int client_sockfd = accept_connection( sockfd );
  check( client_sockfd != -1, "ERROR accepting connection" );

  char op = 0;
  while ( op != 'k' ){

    // read client message
    bstring message = wait_and_read(client_sockfd);    
    // print message
    struct bstrList * tokens =  bsplit(message, ' ');
    op = bdata(tokens->entry[0])[0];
    switch ( op )
    {
      case 'm': // message
      {
        rc = print_message_and_reply(client_sockfd, tokens);
        check( rc != -1, "failed to process message" );
        break;
      }
      case 'c': // control
      {
        check(tokens->qty > 1, "provide control command");
        char ctrl = bdata(tokens->entry[1])[0];
        switch( ctrl )
        {
          case 'k': // kill server
          {
            op = 'k'; // will exit the while
            debug( "Killing in the name of" );
            message = bfromcstr("server shutting down"); 
            rc = write_to_socket( client_sockfd, message );
            break;
          }
          case 'l': // ls server directory
          {
            rc = ls_server_directory( client_sockfd );
            check( rc != -1, "Failed to ls server dir" );
            break;
          }
          case 'c': // cwd
          {
            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            bstring work_dir = blk2bstr(cwd, sizeof(cwd));
            rc = write_to_socket( client_sockfd, work_dir );
            bdestroy(work_dir);

            break;
          }
          case 'f': // send file
          {
            rc = send_file(client_sockfd, tokens);
            check( rc != -1, "File sending failed" );
          
            break;
          }
          case 'e': // execute command
          {
            char * filename = bdata(tokens->entry[2]);
            size_t fname_len = strlen(filename);

            bstring cmd = blk2bstr(filename, fname_len - 1);
            bstring output = exec_command(cmd);
            check( output != NULL, "cmd execution failed" );
            rc = write_to_socket(client_sockfd, output);
            check( rc != -1, "socket write failed");

            bdestroy(cmd);
            bdestroy(output);
          
            break;
          }
          default:
          {
            debug("invalid control sequence");
          }
        } // switch control command
        break;
      } // op case c 
    } // switch op 

    bstrListDestroy( tokens );
    tokens = NULL;

    bdestroy( message ); 
    message = NULL;

  } // while

  //clean up
  rc = close(sockfd);
  check(rc == 0, "ERROR closing socket");
  rc = close(client_sockfd);
  check(rc == 0, "ERROR closing client socket");

  return 0; 
error:
  return -1;
}
