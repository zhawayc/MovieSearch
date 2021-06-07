/*
 * Author: Yanchen Xu
 * Date: Apr 10
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "QueryProtocol.h"

char *port_string = "1500";
unsigned short int port;
char *ip = "127.0.0.1";

#define BUFFER_SIZE 1000

void RunQuery(char *query) {
  // Find the address
  int s;
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  s = getaddrinfo(ip, port_string, &hints, &result);
  // Create the socket
  // Connect to the server
  connect(sock_fd, result->ai_addr, result->ai_addrlen);
  printf("Connected to movie server.");
  //
  char resp[1000] = {0};
  int len = read(sock_fd, resp, sizeof(resp)-1);
  resp[len] = '\0';
  CheckAck(resp);
  // Do the query-protocol
  write(sock_fd, query, strlen(query));

  memset(resp, 0, sizeof(resp));
  len = read(sock_fd, resp, sizeof(resp)-1);
  resp[len] = '\0';
  while (strcmp("GOODBYE", resp) != 0) {
    SendAck(sock_fd);
    memset(resp, 0, sizeof(resp));
    len = read(sock_fd, resp, sizeof(resp)-1);
    resp[len] = '\0';
    puts(resp);
  }
  // Close the connection
  close(sock_fd);
  freeaddrinfo(result);
}

void RunPrompt() {
  char input[BUFFER_SIZE];

  while (1) {
    printf("Enter a term to search for, or q to quit: ");
    scanf("%s", input);

    printf("input was: %s\n", input);

    if (strlen(input) == 1) {
      if (input[0] == 'q') {
        printf("Thanks for playing! \n");
        return;
      }
    }
    printf("\n\n");
    RunQuery(input);
  }
}

// This function connects to the given IP/port to ensure
// that it is up and running, before accepting queries from users.
// Returns 0 if can't connect; 1 if can.
int CheckIpAddress(char *ip, char *port) {
  // Connect to the server
  // Listen for an ACK
  // Send a goodbye
  // Close the connection
  int s;
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  s = getaddrinfo(ip, port_string, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return 0;
  }
  if (connect(sock_fd, result->ai_addr, result->ai_addrlen) == -1) {
    perror("connect");
    return 0;
  } else {
    printf("Connection is good!\n");
  }
  char resp[1000] = {0};
  int len = read(sock_fd, resp, sizeof(resp)-1);
  resp[len] = '\0';
  CheckAck(resp);
  SendGoodbye(sock_fd);
  close(sock_fd);
  freeaddrinfo(result);
  return 1;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Incorrect number of arguments. \n");
    printf("Correct usage: ./queryclient [IP] [port]\n");
  } else {
    ip = argv[1];
    port_string = argv[2];
  }

  if (CheckIpAddress(ip, port_string)) {
    RunPrompt();
  }
  return 0;
}
