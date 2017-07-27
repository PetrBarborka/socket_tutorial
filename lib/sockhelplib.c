#include <unistd.h> //write, read
#include <netdb.h> 
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "dbg.h"
#include "header/bstrlib.h"
#include "header/sockhelplib.h"

struct sockaddr in2addr( struct sockaddr_in in ){
    struct sockaddr sock_addr;  
    char * addr_data = sock_addr.sa_data;

    sock_addr.sa_family = in.sin_family;

    memmove( addr_data,
             (char *)&in.sin_port,
             sizeof(in.sin_port) );
    addr_data += sizeof(in.sin_port);

    memmove( addr_data,
             (char *)&in.sin_addr.s_addr,
             sizeof(in.sin_addr.s_addr) );
    addr_data += sizeof(in.sin_addr.s_addr);

    memset( addr_data,
            '\0',
            8 );

    return sock_addr;
}

struct sockaddr_in addr2in(struct sockaddr sock_addr){
    struct sockaddr_in in;  
    char * addr_data = sock_addr.sa_data;

    in.sin_family = sock_addr.sa_family;

    memmove( (char *)&in.sin_port,
             addr_data,
             sizeof(in.sin_port) );
    addr_data += sizeof(in.sin_port);

    memmove( (char *)&in.sin_addr.s_addr,
             addr_data,
             sizeof(in.sin_addr.s_addr) );
    addr_data += sizeof(in.sin_addr.s_addr);

    return in;
}

// client

int connect_to_server(bstring hostname, int port){

    int sock_desc = -1;
    int rc = -1;

    #ifdef METHOD_GETADDRINFO

      struct addrinfo hints, *servinfo, *p;
      memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;

      char portbuf[16];
      sprintf(portbuf, "%d", port);
      rc = getaddrinfo(bdata(hostname), portbuf, &hints, &servinfo);
      check( rc != -1, "failed to get addrinfo");
      
      for( p = servinfo; p != NULL; p = p->ai_next ){
        sock_desc = socket(p->ai_family, p->ai_socktype, 
                          p->ai_protocol);
        if( sock_desc == -1 ) continue;

        rc = connect( sock_desc, p->ai_addr, p->ai_addrlen );
        check( rc != -1, "failed to connect to server" );

        break;
      }

      freeaddrinfo(servinfo);
      check(p != NULL, "failed to connect w/ any addresinfo");

    #else // default method - gethostbyname etc
      // -- setup socket:
      // open socket, get its index in file desc table
      sock_desc = socket(AF_INET, SOCK_STREAM, 0);
      check( sock_desc >= 0, "ERROR opening socket" );

      // get struct hostent from the hostname from command line
      struct hostent * host = gethostbyname(bdata(hostname));
      check( host != NULL, "ERROR, no such host\n" );

      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port);
      addr.sin_addr.s_addr = *((in_addr_t *)host->h_addr_list[0]);

      struct sockaddr sock_addr = in2addr(addr);

      // -- connect to socket:
      debug( "connecting to server" );
      rc = connect(sock_desc,
                      &sock_addr,
                      sizeof(addr));
      debug("Connect exited w/ rc %d", rc);
      check(rc == 0, "ERROR connecting");
    #endif

    return sock_desc;
error:
    return -1;
}

int write_to_socket( int socket_descriptor, bstring buffer ){

    check( buffer != NULL, "NULL buffer");
    check( bdata(buffer) != NULL, "empty buffer");
    if ( blength(buffer) == 0 ) return 0;

    debug( "writing to socket" );
    int rc = write( socket_descriptor, bdata(buffer), blength(buffer) );
    check (rc != -1, "ERROR writing to socket");
    check (rc != 0, "ERROR writing to socket: no bytes written");
    debug("%d bytes written", rc);

    return rc;
error:
    return -1;
}

bstring read_from_socket( int sock_desc ){

  // is there anything to read on the socket?
  int num_bytes;
  ioctl(sock_desc, FIONREAD, &num_bytes);
  if( num_bytes == 0 ){
    /*debug("nothing to read");*/
    return NULL;
  }
  
  char * buff[1024] = { 0 };
  int rc = read( sock_desc, buff, 1024 );
  check( rc >= 0, "Error reading from socket" );
  if (rc == 0) return NULL;

  bstring output = blk2bstr(buff, 1024);
  check( output != NULL , "ERROR allocating bstr" );
  debug( "reading from socket" );

  return output;
error:
  return NULL;
}

// server

int bind_and_listen( int port ){

  int sock_desc = -1;
  int rc = -1;

  #ifdef METHOD_GETADDRINFO

    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char portbuf[16];
    sprintf(portbuf, "%d", port);
    rc = getaddrinfo(NULL, portbuf, &hints, &servinfo);
    check( rc != -1, "failed to get addrinfo");
    
    for( p = servinfo; p != NULL; p = p->ai_next ){
      sock_desc = socket(p->ai_family, p->ai_socktype, 
                        p->ai_protocol);

      int yes = 1;
      //allow bind calls to reuse sockets that are not connected
      //to anything -- potential (not real in this case) 
      //security risk
      rc = setsockopt( sock_desc, SOL_SOCKET, SO_REUSEADDR,
                      &yes, sizeof(yes) );

      if( sock_desc == -1 ) continue;

      rc = bind( sock_desc, p->ai_addr, p->ai_addrlen );
      check( rc != -1, "failed to connect to server" );

      break;
    }

    freeaddrinfo(servinfo);
    check(p != NULL, "failed to connect w/ any addresinfo");

  #else // default method - gethostbyname etc

    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    //allow bind calls to reuse sockets that are not connected
    //to anything -- potential (not real in this case) security risk
    rc = setsockopt( sock_desc, SOL_SOCKET, SO_REUSEADDR,
                     &yes, sizeof(yes) );
    check(sock_desc != -1, "ERROR opening socket");
    
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    struct sockaddr sock_addr = in2addr(serv_addr);

    debug("binding");
    rc = bind(sock_desc, &sock_addr, sizeof(sock_addr));
    check( rc == 0, "ERROR on binding");

  #endif

  debug("listening on socket");
  listen(sock_desc,5);
  check( rc == 0, "ERROR listening");

  return sock_desc;
error:
  return -1;

}

int accept_connection( int sockfd ){

  check( sockfd != -1, "Invalid socket file descriptor" );

  struct sockaddr cli_addr;
  socklen_t client_len = sizeof(cli_addr);
  debug("accepting connection");
  int client_sockfd = accept( sockfd, 
                              &cli_addr, 
                              &client_len );
  check(client_sockfd >= 0, "ERROR on accept");
  debug( "Client connected.\n");
  struct sockaddr_in cli_addr_in = addr2in(cli_addr);
  debug( "Client sin_family: %s", 
         cli_addr_in.sin_family == AF_INET ? "AF_INET" : "OTHER" );
  debug( "Client port: %d", ntohs(cli_addr_in.sin_port) );
  char * ip = (char *) &cli_addr_in.sin_addr.s_addr;
  debug( "Client s_addr: %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

  return client_sockfd;
error:
  return -1;

}

bstring wait_and_read(int client_sockfd){
  bstring message = NULL;
  while ( message == NULL ){
    message = read_from_socket( client_sockfd );
    usleep( 100 * 1000);
  }

  return message;
}
