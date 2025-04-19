#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>

int main() {
    int sv[2]; // Socket pair
    pid_t pid;
    char buf[1024];

    // Create a pair of connected sockets
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        perror("socketpair");
        exit(1);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {  // Child process
        close(sv[1]); // Close parent's end
        const char *msg = "Hello from child";
        write(sv[0], msg, strlen(msg));
        close(sv[0]);
    } else {         // Parent process
        close(sv[0]); // Close child's end
        read(sv[1], buf, sizeof(buf));
        printf("Parent received: %s\n", buf);
        close(sv[1]);
    }

    return 0;
}

