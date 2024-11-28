#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#define BUFFER_SIZE 2
#define PORT "9000"
#define BACKLOG 5
const char *file_path = "/var/tmp/aesdsocketdata";

int main(int argc, char *argv[])
{
    int server_fd, s;
    char *buffer;
    // ssize_t                  nread;
    // socklen_t                peer_addrlen;
    size_t buffer_size = BUFFER_SIZE;

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    // struct sockaddr_storage  peer_addr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */
    hints.ai_protocol = 0;           /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    openlog("SyslogSocket", LOG_PID | LOG_CONS, LOG_USER);
    s = getaddrinfo(NULL, PORT, &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        // exit(EXIT_FAILURE);
        return -1;
    }

    /* getaddrinfo() returns a list of address structures.
        Try each address until we successfully bind(2).
        If socket(2) (or bind(2)) fails, we (close the socket
        and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        server_fd = socket(rp->ai_family, rp->ai_socktype,
                           rp->ai_protocol);
        if (server_fd == -1)
            continue;

        if (bind(server_fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break; /* Success */

        close(server_fd);
    }

    freeaddrinfo(result); /* No longer needed */

    if (rp == NULL)
    { /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        // exit(EXIT_FAILURE);
        return -1;
    }

    /* Read datagrams and echo them back to sender. */
    if (listen(server_fd, BACKLOG) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        // exit(EXIT_FAILURE);
        return -1;
    }
    int client_fd;
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_len)) < 0)
    {
        perror("Accept failed");
        close(server_fd);
        // exit(EXIT_FAILURE);
        return -1;
    }
    printf("Connection accepted\n");
    // Dynamically allocate memory for the buffer
    buffer = (char *)malloc(buffer_size);
    if (buffer == NULL)
    {
        perror("Failed to allocate memory");
        close(client_fd);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    char client_ip[INET_ADDRSTRLEN];
    // Determine the client's IP address
    // Convert client IP address to string
    inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
    syslog(LOG_INFO, "Accepted connection from %s", client_ip);
    printf("Client connected with IP address: %s and port: %d\n",
           client_ip, ntohs(client_address.sin_port));

    const char *file_path = "/var/tmp/aesdsocketdata";
    int fd;

    // Open the file, create it if it doesn't exist, with read/write permissions
    fd = open(file_path, O_CREAT | O_RDWR, 0644);
    if (fd < 0)
    {
        perror("Failed to open or create file");
        // exit(EXIT_FAILURE);
        return -1;
    }

    printf("File '%s' created or opened successfully.\n", file_path);

    ssize_t total_received =0;
    ssize_t bytes_received =0;
    while (1)
    {

        while (1)
        {
            bytes_received = recv(client_fd, buffer + total_received, buffer_size, 0);
            if (bytes_received == -1)
            {
                perror("Receive failed");
                break;
            }
            else if (bytes_received == 0)
            {
                printf("Client disconnected.\n");
                break;
            }

            total_received += bytes_received;

            // Check if more space is needed in the buffer
            if (total_received + BUFFER_SIZE > buffer_size)
            {
                buffer_size += BUFFER_SIZE; // Double the buffer size
                buffer = (char *)realloc(buffer, buffer_size);
                if (buffer == NULL)
                {
                    perror("Failed to reallocate memory");
                    close(client_fd);
                    close(server_fd);
                    // exit(EXIT_FAILURE);
                    return -1;
                }
            }

            // Stop receiving if the client sends less than a chunk
            if (buffer[total_received -1 ] == '\n')
            {
                break;
            }
        }

        printf("Received: %s", buffer);

        ssize_t bytes_written = write(fd, buffer, total_received); // -1 to exclude null terminator
        if (bytes_written < 0)
        {
            perror("Failed to write to file");
            close(fd);
            // exit(EXIT_FAILURE);
            return -1;
        }
        printf("Wrote %ld bytes to '%s'.\n", bytes_written, file_path);

        // Check for "exit" command to break the loop
        if (strcmp(buffer, "exit\n") == 0)
        {
            printf("Exit command received. Closing connection.\n");
            break;
        }
    }
    // Write some initial data to the file
    // const char *data = "Hello, aesdsocketdata!\n";

    // Close the file
    if (close(fd) < 0)
    {
        perror("Failed to close file");
        // exit(EXIT_FAILURE);
        return 1;
    }

    printf("File '%s' closed successfully.\n", file_path);
    close(server_fd);
    closelog();
    return 0;
}
