#include "aesdsocket.h"

static pthread_mutex_t lock; 

static int fd =0;
int init_file(const char * file_path){
    // Open the file, create it if it doesn't exist, with read/write permissions
    fd = open(file_path, O_CREAT | O_RDWR, 0644);
    if (fd < 0)
    {
        // exit(EXIT_FAILURE);
        return -1;
    }
    printf("File '%s' created or opened successfully.\n", file_path);
    return 0;
}
int close_file(){
        // Close the file
    if (close(fd) < 0)
    {
        return -1;
    }
    return 0;
}
int mutex_init(){
       if (pthread_mutex_init(&lock, NULL) != 0) { 
        return -1; 
    }
    return 0;
}
void mutex_destroy(){
        pthread_mutex_destroy(&lock); 
}
void write_ip_to_syslog(struct sockaddr_in in_sockaddr, char out_ip[])
{
    // Convert client IP address to string
    inet_ntop(AF_INET, &in_sockaddr.sin_addr, out_ip, INET_ADDRSTRLEN);
    syslog(LOG_INFO, "Accepted connection from %s", out_ip);
    printf("Client connected with IP address: %s and port: %d\n",
           out_ip, ntohs(in_sockaddr.sin_port));
}

void *message_handler(void *args)
{
    int * c_fd= (int *)args;
    int client_fd = * c_fd;
    ssize_t bytes_received = 0;
    char *buffer;
    size_t buffer_size = BUFFER_SIZE;
    size_t total_received = 0;
    // Dynamically allocate memory for the buffer
    buffer = (char *)malloc(buffer_size);
    if (buffer == NULL)
    {
        perror("Failed to allocate memory");
        // return -1;
    }

    while (1)
    {
        bytes_received = recv(client_fd, buffer + total_received, BUFFER_SIZE, 0);
        if (bytes_received == -1)
        {
            perror("Receive failed");
            break;
        }
        else if (bytes_received == 0)
        {
            printf("Client %d disconnected.\n", client_fd);
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
                // exit(EXIT_FAILURE);
                // return -1;
            }
        }

        // Stop receiving if the client sends less than a chunk
        if (buffer[total_received - 1] == '\n')
        {
            pthread_mutex_lock(&lock); 
            
            printf("Received: %s", buffer);
            ssize_t bytes_written = write(fd, buffer, total_received); // -1 to exclude null terminator
            if (bytes_written < 0)
            {
                perror("Failed to write to file");
                close(fd);
                // return -1;
            }
            printf("Wrote %ld bytes .\n", bytes_written);
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
                // return 1;
                break;
            }
            // Move the file pointer back to the beginning of the file
            lseek(fd, 0, SEEK_SET);
            // Allocate memory for the file content
            rbuffer = (char *)malloc(file_size + 1); // +1 for null-terminator
            if (rbuffer == NULL)
            {
                perror("Memory allocation error");
                break;
                // return -1;
            }
            // Read the file content into the buffer
            bytesRead = read(fd, rbuffer, file_size);
            if (bytesRead == -1)
            {
                perror("Error reading file");
                free(rbuffer);
                break;
                // return -1;
            }

            pthread_mutex_unlock(&lock); 
            // Null-terminate the buffer to make it a string (optional, if you plan to treat it as text)
            rbuffer[bytesRead] = '\0';
            ssize_t bytesSent = send(client_fd, rbuffer, bytesRead, 0);
            if (bytesSent == -1)
            {
                perror("Error sending file data");
                close(client_fd);
                free(rbuffer);
                // return -1;
                break;
            }

            free(rbuffer);
            free(buffer);
            total_received = 0;
            buffer_size = BUFFER_SIZE;
            buffer = (char *)malloc(buffer_size);
        }
    }
    free(buffer);
    if (client_fd >= 0)
    {
        close(client_fd);
    }
    return NULL;
}