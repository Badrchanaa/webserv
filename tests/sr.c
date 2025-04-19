#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  // Create socket
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("socket");
    return 1;
  }

  // Allow address reuse
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt");
    close(server_fd);
    return 1;
  }

  // Bind to port 8080
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr))) {
    perror("bind");
    close(server_fd);
    return 1;
  }

  // Set backlog to 2
  if (listen(server_fd, 2) == -1) { // Queue holds MAX 2 pending connections
    perror("listen");
    close(server_fd);
    return 1;
  }

  printf("Server listening on port 8080. Backlog=2.\n");
  printf("Waiting 10 seconds before accepting connections...\n");
  sleep(10); // Simulate delay in accepting connections

  // Accept connections (after queue may have filled)
  while (1) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
      perror("accept");
      break;
    }
    printf("Accepted a connection.\n");
    close(client_fd);
  }

  close(server_fd);
  return 0;
}
