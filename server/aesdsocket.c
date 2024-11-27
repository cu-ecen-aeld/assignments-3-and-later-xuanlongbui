#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 500
#define PORT "9000"
#define BACKLOG 5  

int main(int argc, char *argv[])
{
    int                      sfd, s;
    // char                     buf[BUF_SIZE];
    // ssize_t                  nread;
    // socklen_t                peer_addrlen;
    struct addrinfo          hints;
    struct addrinfo          *result, *rp;
    // struct sockaddr_storage  peer_addr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, PORT, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
        Try each address until we successfully bind(2).
        If socket(2) (or bind(2)) fails, we (close the socket
        and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                    rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        close(sfd);
    }

    freeaddrinfo(result);           /* No longer needed */

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    /* Read datagrams and echo them back to sender. */
      if (listen(sfd, BACKLOG) < 0) {
        perror("Listen failed");
        close(sfd);
        exit(EXIT_FAILURE);
    }
    int client_fd;
    struct sockaddr client_address;
    socklen_t client_address_len = sizeof(client_address);

    if ((client_fd = accept(sfd, &client_address, &client_address_len)) < 0) {
        perror("Accept failed");
        close(sfd);
        exit(EXIT_FAILURE);
    }
    printf("Connection accepted\n");
    // Determine the client's IP address
    char client_ip[INET_ADDRSTRLEN];

    printf("Client connected with IP address: %s\n", client_ip);
    }
