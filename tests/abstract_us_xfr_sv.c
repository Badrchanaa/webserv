// abstract_us_xfr_sv.c
#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 100
#define ABSTRACT_NAME "my_abstract_socket"

int main(void) {
    int sfd, cfd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    ssize_t numRead;
    socklen_t addrlen;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        perror("socket"), exit(EXIT_FAILURE);

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;

    // Abstract namespace
    addr.sun_path[0] = '\0';  // abstract socket indicator
    strncpy(&addr.sun_path[1], ABSTRACT_NAME, sizeof(addr.sun_path) - 2);

    addrlen = offsetof(struct sockaddr_un, sun_path) + 1 + strlen(ABSTRACT_NAME);

    if (bind(sfd, (struct sockaddr *) &addr, addrlen) == -1)
        perror("bind"), exit(EXIT_FAILURE);

    if (listen(sfd, 5) == -1)
        perror("listen"), exit(EXIT_FAILURE);

    for (;;) {
        cfd = accept(sfd, NULL, NULL);
        if (cfd == -1) {
            perror("accept");
            continue;
        }

        while ((numRead = read(cfd, buf, BUF_SIZE)) > 0)
            if (write(STDOUT_FILENO, buf, numRead) != numRead)
                perror("partial/failed write to stdout");

        if (numRead == -1)
            perror("read");

        close(cfd);
    }
}

