#ifndef AESDSOCKET_H
#define AESDSOCKET_H

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
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <sys/queue.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define PORT "9000"
#define BACKLOG 5

// the TAILQ_ENTRY macro uses the struct name.
struct node
{
    pthread_t tid;
    int client_fd;
    // This macro does the magic to point to other nodes
    TAILQ_ENTRY(node) nodes;
};
// This typedef creates a head_t that makes it easy for us to pass pointers to
// head_t without the compiler complaining.

void *message_handler(void *args);
void write_ip_to_syslog(struct sockaddr_in in_sockaddr, char out_ip[]);
int mutex_init();
void mutex_destroy();

int init_file(const char * file_path);
int close_file();
void *append_timestamp(void *arg);
#endif