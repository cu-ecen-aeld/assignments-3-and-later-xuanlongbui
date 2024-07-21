// C program to illustrate
// open system call
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <syslog.h>


char * filespath;
char * contentstr;

int check_exist_dir(const char * __path, const int __size );
int create_parent_path(const char * __path);
int main(int argc,char *argv[])
{

    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog("writer", LOG_CONS| LOG_PID | LOG_NDELAY , LOG_LOCAL1);

    if (argc < 4)
    {
        if(argc < 2){
            syslog(LOG_ERR, "The parameters were not specified " );      
            return 0;
        }else{
            filespath= argv[1];
            contentstr= argv[2];

        }
    }
    else
    {
        syslog(LOG_ERR, "The parameters were not specified " );      
        return 0;

    }
    char * folderpath;
    folderpath = (char*)malloc(100);

    strcpy(folderpath, filespath);


    char *last_slash = strrchr(folderpath, '/');
    if (last_slash) {
        *last_slash = '\0'; // Null-terminate at the last slash
        // printf("Directory path: %s\n", folderpath);
    } else {
        syslog(LOG_ERR, "Invalid filepath\n");
    }
    check_exist_dir(folderpath,  100);


    // create new file
    int fd = open(filespath, O_CREAT | O_RDWR, 0777);

    // printf("Opening file returned fd = %d\n", fd);
 
    if (fd < 0) {
        perror("c1");
        syslog(LOG_ERR, "Couldn't open the file ");
        exit(1);
    }
    write(fd, contentstr, strlen(contentstr)); 
    
    // printf("Writing content returned %d\n", sz); 
    syslog(LOG_DEBUG, "“Writing %s to %s", contentstr ,filespath );

    // Using close system Call
    if (close(fd) < 0) {
        perror("c1");
        syslog(LOG_ERR, "Couldn't close the file ");
        exit(1);
    }
    // printf("closed the fd.\n");
    closelog();
return 0;
}

int check_exist_dir(const char * __path, const int __size ){

    char * obj;
    obj = (char *)malloc(__size);

    for (int i = 1; i < __size; i++)
    {
        char c = *(__path +i);
        if (c == '/' )
        {
            strncpy(obj, __path, i);
            create_parent_path(obj);
        }
        
    }
    create_parent_path(__path);

    return 0;
}

int create_parent_path(const char * __path){
    struct stat sb ={0};
    if (stat(__path, &sb) == -1 ) {
        mkdir(__path, 0777);
    } else {
        //PASS
    }
    return 0;
}