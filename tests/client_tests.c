#include <unistd.h>

#include <sys/types.h>
#include <signal.h>

#include "minunit.h"
#include "assert.h"

#include "bstrlib.h"
#include "sockhelplib.h"

static pid_t child_pid = 0;

char * run_server(){
  child_pid = fork();
  if ( child_pid == 0 ){
    debug("starting server in child process: pid %d",
          child_pid );
    execl("src/server", "src/server", "55555", (char *) NULL);
    mu_assert(0, 
         "The server process failed to start");
  } else {
    debug("continuing after fork in parent process: pid %d",
          child_pid );
    // wait for the server to come up
    usleep(300 * 1000);
  }

  return NULL;
}

char * kill_server(){
  if ( child_pid == 0 ){
    mu_assert( 0, "kill server shall never be called in child");
  } else{
    debug("Ok, killing in parrent - child pid %d", 
           child_pid);
    int rc = kill(child_pid, SIGKILL);
    mu_assert(rc == 0, "kill failed");
  }

  return NULL;
}

char * test_server_open_close(){

  char * rc = run_server();
  mu_assert( rc == NULL, "run server failed" );
  rc = kill_server();
  mu_assert( rc == NULL, "kill server failed" );

  return NULL;
}

char * test_server_send_message(){

  char * rc = run_server();
  mu_assert( rc == NULL, "run server failed" );

  bstring hostname = bfromcstr("localhost");
  int sockfd = connect_to_server(hostname, 55555);
  mu_assert( sockfd != -1, "Failed to connect to server" );
  bstring message = bfromcstr("m ahoj");
  int ret_c = write_to_socket(sockfd, message);
  mu_assert( ret_c != -1, "Failed to send message to server" );
  bstring reply = wait_and_read(sockfd);
  mu_assert( reply != NULL, "Failed to get reply from server" );
  bstring exp = bfromcstr("message received");
  mu_assert(bstrcmp(exp, reply), "didn't get expected reply form message");
  
  bdestroy(hostname);
  bdestroy(message);
  bdestroy(reply);
  bdestroy(exp);

  rc = kill_server();
  mu_assert( rc == NULL, "kill server failed" );

  return NULL;
}

char * all_tests(){

  mu_suite_start();

  mu_run_test(test_server_open_close);
  mu_run_test(test_server_send_message);

  return NULL;
}

RUN_TESTS(all_tests);
