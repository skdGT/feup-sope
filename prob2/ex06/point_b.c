#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h> 


// Adapeted from @TiagooGomess

int main(int argc, char *argv[]) {
    DIR *dirp;
    struct dirent *direntp;
    struct stat stat_buf;
    char *str;

 if (argc != 2) {
    fprintf( stderr, "Usage: %s dir_name\n", argv[0]);
    exit(1);
 }

 if ((dirp = opendir( argv[1])) == NULL) {
    perror(argv[1]);
    exit(2);
 }

 chdir(argv[1]);

 printf("\n%-25s   %9s   %10s   %8s\n", "Name:", "Type:", "i-node:", "Size:");
 printf("---------------------------------------------------------------\n");

 while ((direntp = readdir( dirp)) != NULL) {
    if (lstat(direntp->d_name, &stat_buf) == -1) { 
        exit(3);    
    }

    if (S_ISREG(stat_buf.st_mode))
        str = "regular";
    else if (S_ISDIR(stat_buf.st_mode))
        str = "directory";
    else
        str = "other";

    printf("%-25s   %9s   %10d", direntp->d_name, str, (int) stat_buf.st_ino);

    if (strcmp(str, "regular") == 0) {
        printf("   %8d", (int) stat_buf.st_size);
    }

    printf("\n");
 }

    closedir(dirp);
    exit(0);
}