#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define SERVER_PORT "1337"

#define BACKLOG 128

#define MAX_EVENTS 1024

// print IP address of the new client
void print_ip(struct sockaddr_storage *client_saddr) {
  char str[INET6_ADDRSTRLEN];
  struct sockaddr_in *ptr;
  struct sockaddr_in6 *ptr1;
  if (client_saddr->ss_family == AF_INET) {
    ptr = (struct sockaddr_in *)&client_saddr;
    inet_ntop(AF_INET, &(ptr->sin_addr), str, sizeof(str));

  } else if (client_saddr->ss_family == AF_INET6) {
    ptr1 = (struct sockaddr_in6 *)&client_saddr;
    inet_ntop(AF_INET6, &(ptr1->sin6_addr), str, sizeof(str));

  } else {
    ptr = NULL;
    fprintf(stderr, "Address family is neither AF_INET nor AF_INET6\n");
    return;
  }
  printf("////////////////////////////////////////////////////\n");
  printf("We recive connection From: -> %s\n", str);
  printf("////////////////////////////////////////////////////\n");
}

void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
  char recv_message[4095];
  const char *const ident = "flight-time-server";

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;     /* allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* Stream socket */
  hints.ai_flags = AI_PASSIVE;     /* for wildcard IP address */

  struct addrinfo *result;
  int s;
  if ((s = getaddrinfo(NULL, SERVER_PORT, &hints, &result)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  /* Scan through the list of address structures returned by
     getaddrinfo. Stop when the the socket and bind calls are successful. */

  int listener, optval = 1;
  socklen_t length;
  struct addrinfo *rptr;
  for (rptr = result; rptr != NULL; rptr = rptr->ai_next) {
    listener = socket(rptr->ai_family, rptr->ai_socktype, rptr->ai_protocol);
    if (listener == -1)
      continue;

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) ==
        -1)
      error("setsockopt");

    if (bind(listener, rptr->ai_addr, rptr->ai_addrlen) == 0) // Success
      break;

    if (close(listener) == -1)
      error("close");
  }

  if (rptr == NULL) { // Not successful with any address
    fprintf(stderr, "Not able to bind\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(result);

  // Mark socket for accepting incoming connections using accept
  if (listen(listener, BACKLOG) == -1)
    error("listen");

  int efd;
  if ((efd = epoll_create(1)) == -1)
    error("epoll_create1");
  struct epoll_event ev, ep_event[MAX_EVENTS];

  // ev.events = EPOLLIN;
  ev.events = EPOLLIN | EPOLLET;
  ev.data.fd = listener;
  if (epoll_ctl(efd, EPOLL_CTL_ADD, listener, &ev) == -1)
    error("epoll_ctl");

  int nfds = 0;

  socklen_t addrlen;
  struct sockaddr_storage client_saddr;
  struct tnode *root = NULL;

  while (1) {
    // monitor readfds for readiness for reading
    if ((nfds = epoll_wait(efd, ep_event, MAX_EVENTS, -1)) == -1)
      error("epoll_wait");

    // Some sockets are ready. Examine readfds
    for (int i = 0; i < nfds; i++) {

      if ((ep_event[i].events & EPOLLIN) == EPOLLIN) {
        if (ep_event[i].data.fd == listener) { // request for new connection
          addrlen = sizeof(struct sockaddr_storage);
          int fd_new;
          if ((fd_new = accept(listener, (struct sockaddr *)&client_saddr,
                               &addrlen)) == -1)
            error("accept");
          // add fd_new to epoll
          ev.events = EPOLLIN;
          ev.data.fd = fd_new;
          if (epoll_ctl(efd, EPOLL_CTL_ADD, fd_new, &ev) == -1)
            error("epoll_ctl");
          print_ip(&client_saddr);

        } else // data from an existing connection, receive it
        {
          memset(&recv_message, '\0', sizeof(recv_message));
          ssize_t numbytes =
              recv(ep_event[i].data.fd, &recv_message, sizeof(recv_message), 0);

          if (numbytes == -1)
            error("recv");
          else if (numbytes == 0) {
            // connection closed by client
            fprintf(stderr, "Socket %d closed by client\n",
                    ep_event[i].data.fd);
            // delete fd from epoll
            if (epoll_ctl(efd, EPOLL_CTL_DEL, ep_event[i].data.fd, &ev) == -1)
              error("epoll_ctl");
            if (close(ep_event[i].data.fd) == -1)
              error("close");
          } else {
            // data from client
            // for (int j = 0; j < numbytes; j++)
            //   recv_message[j] = toupper((unsigned char) recv_message[j]);
            write(1, &recv_message, numbytes);
            // if (send(ep_event[i].data.fd, &recv_message,
            // sizeof(recv_message), 0) == -1)
            //    error("send");

            // if (send(ep_event[i].data.fd, &send_message,
            // sizeof(send_message), 0) == -1)
            //   error("send");
          }
        }
      } // if (fd == ...
    } // for
  } // while (1)

  exit(EXIT_SUCCESS);
} // main
