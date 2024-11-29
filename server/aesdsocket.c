#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <signal.h>

#define BUFFER_SIZE 1024
#define PORT "9000"
#define BACKLOG 5
const char *file_path = "/var/tmp/aesdsocketdata";
unsigned int should_exit = 0;
int server_fd;
int client_fd;

void handle_signal(int sig)
{
    printf("Received SIGINT (Ctrl+C) or SIGTERM, terminating the program...\n");
    // You can do clean-up here, or use exit() to terminate the program
    should_exit = 1;
    if (remove(file_path) == 0)
    {
        printf("File '%s' deleted successfully.\n", file_path);
    }
    else
    {
        perror("Error deleting file");
    }
    close(client_fd);
    close(server_fd);
}

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
    if (signal(SIGINT, handle_signal) == SIG_ERR)
    {
        perror("Error registering SIGINT handler");
        return -1;
        // exit(1);
    }
    if (signal(SIGTERM, handle_signal) == SIG_ERR)
    {
        perror("Error registering SIGTERM handler");
        return -1;
        // exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* stream socket */
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

    ssize_t total_received = 0;
    ssize_t bytes_received = 0;
    while (!should_exit)
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
            if (buffer[total_received - 1] == '\n')
            {
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
                // Read from the file until EOF is reached
                ssize_t bytesRead = 0;
                char *rbuffer = NULL; // Buffer to store data from the file
                off_t file_size = 0;

                // Get the size of the file
                file_size = lseek(fd, 0, SEEK_END);
                if (file_size == -1)
                {
                    perror("Error getting file size");
                    close(fd);
                    return 1;
                }
                // Move the file pointer back to the beginning of the file
                lseek(fd, 0, SEEK_SET);
                // Allocate memory for the file content
                rbuffer = (char *)malloc(file_size + 1); // +1 for null-terminator
                if (rbuffer == NULL)
                {
                    perror("Memory allocation error");
                    close(fd);
                    return -1;
                }

                if (bytesRead == -1)
                {
                    perror("Error reading file");
                    close(fd);
                    return -1;
                }
                // Read the file content into the buffer
                bytesRead = read(fd, rbuffer, file_size);
                if (bytesRead == -1)
                {
                    perror("Error reading file");
                    free(rbuffer);
                    close(fd);
                    return -1;
                }

                // Null-terminate the buffer to make it a string (optional, if you plan to treat it as text)
                rbuffer[bytesRead] = '\0';
                ssize_t bytesSent = send(client_fd, rbuffer, bytesRead, 0);
                if (bytesSent == -1)
                {
                    perror("Error sending file data");
                    close(fd);
                    close(client_fd);
                    close(server_fd);
                    return -1;
                }

                free(rbuffer);
                break;
            }
        }

        total_received = 0;
        free(buffer);
        buffer_size = BUFFER_SIZE;
        buffer = (char *)malloc(buffer_size);
    }
    // Write some initial data to the file

    // Close the file
    if (close(fd) < 0)
    {
        perror("Failed to close file");
        // exit(EXIT_FAILURE);
        return 1;
    }
    printf("File '%s' closed successfully.\n", file_path);
    closelog();
    return 0;
}
