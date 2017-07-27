#ifndef __sockhelplib__h__
#define __sockhelplib__h__

struct sockaddr in2addr( struct sockaddr_in in );
struct sockaddr_in addr2in( struct sockaddr addr );

int connect_to_server(bstring hostname, int port);
int write_to_socket( int socket_descriptor, bstring buffer );
bstring read_from_socket( int sock_desc );

int bind_and_listen( int port);
int accept_connection( int sockfd );

bstring wait_and_read(int client_sockfd);

#endif //__sockhelplib__h__
