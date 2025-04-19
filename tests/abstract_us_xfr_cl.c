// abstract_us_xfr_cl.c
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
#define ABSTRACT_NAME "my_abstract_soc"

int main(void) {
    int sfd;
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
    addr.sun_path[0] = '\0';
    strncpy(&addr.sun_path[1], ABSTRACT_NAME, sizeof(addr.sun_path) - 2);

    addrlen = offsetof(struct sockaddr_un, sun_path) + 1 + strlen(ABSTRACT_NAME);

    if (connect(sfd, (struct sockaddr *) &addr, addrlen) == -1)
        perror("connect"), exit(EXIT_FAILURE);

    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0)
        if (write(sfd, buf, numRead) != numRead)
            perror("partial/failed write");

    if (numRead == -1)
        perror("read");

    exit(EXIT_SUCCESS);
}

