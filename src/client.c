#include <signal.h>
#include <stdio.h> //perror, fprintf, printf, fgets
#include <stdlib.h>  //exit, atoi
#include <unistd.h> //write, read
#include <string.h> //bzero, bcopy
#include <netdb.h> 
#include "dbg.h"

#include "../lib/header/bstrlib.h"
#include "../lib/header/sockhelplib.h"

void print_help(){
  printf(
"\tm \"<your message>\" to send message, \n\
\tc c - print cwd \n\
\tc e <command> - execute arbitrary single word command \n\
\tc l - ls cwd \n\
\tc f <filename> receive file from server \n\
\tc k to kill server \n\
\tq to quit\n\n\n"
  );
}


int main(int argc, char *argv[])
{
    // ignore SIGPIPE and get a real error instead
    signal(SIGPIPE, SIG_IGN);

    check (argc == 3 ,"usage %s hostname port\n", argv[0]);

    bstring host = bfromcstr(argv[1]);
    int port = atoi(argv[2]);
    int sock_desc = connect_to_server(host, port);
    check( sock_desc != -1, "failed to connect the socket");
    bdestroy(host);
    
    // -- get stdin input:
    char op = '0';
    while ( op != 'q' ){

      printf("type h for help \n");
      printf(" >> ");
      bstring input_buffer = bgets((bNgetc) fgetc, stdin, '\n');
      printf("\n");
      struct bstrList * tokens =  bsplit(input_buffer, ' ');

      op = bdata(tokens->entry[0])[0];
      switch ( op ){
        case 'q' : 
          debug( "quitting" );
          break;
        case 'h' : 
          print_help();
          break;
        case 'm' :
        case 'c' :
        {
          int rc = write_to_socket( sock_desc, input_buffer);
          check( rc >= 0, "Failed to write to socket");
          bdestroy(input_buffer);
          input_buffer = NULL;

          while ( input_buffer == NULL ){
            input_buffer = read_from_socket( sock_desc );
            usleep(50*1000);
          }
          printf( "Server response:\n" );
          while( input_buffer != NULL ){
            printf( "%s", bdata(input_buffer) );
            input_buffer = read_from_socket( sock_desc );
          }
          printf( "\n" );

          break;
        }
        default:
          printf("invalid operation: %c\n", op );
          break;
      }

      bdestroy(input_buffer);
      input_buffer = NULL;

      bstrListDestroy(tokens);
      tokens = NULL;

      if( op == 'q' ) break;
    }

    int rc = close( sock_desc );
    check( rc == 0, "Failed to close file descriptor");

    return 0;

error:
    return -1;
}
