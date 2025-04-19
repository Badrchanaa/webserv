#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    int sv[2]; // the pair of sockets

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }

    // Now sv[0] and sv[1] are connected sockets
    write(sv[0], "hello", 5);

    char buf[10];
    read(sv[1], buf, 10);

    printf("Received: %.*s\n", 5, buf);

    close(sv[0]);
    close(sv[1]);
    return 0;
}

