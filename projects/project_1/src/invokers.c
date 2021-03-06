#include "invokers.h"


int fork_read(char* path, int level) {
  int status;
  pid_t pID;
  char* name;
  long int dirSize = 0;

  DIR* dir;
  if ((dir = opendir(path)) == NULL) {
    perror(path);
    exit(2);
  }

  struct dirent *ent;
  while ((ent = readdir(dir)) != NULL) {
    char *entry_name = ent->d_name;
    if (!strcmp(".", entry_name) || !strcmp("..", entry_name))
        continue;

    if (!strcmp(".", path) || !strcmp("..", path))
      return 0;

    struct stat st_buf;
    name = (char*) malloc (BUFFER_SIZE * sizeof(char));
    sprintf(name, "%s%s", path, ent->d_name);
    /* Checking wether we want to distinguish between symlinks and regfiles or not */
    (arguments.dereference) ? stat(name, &st_buf) : lstat(name, &st_buf);

    if (S_ISDIR(st_buf.st_mode)) {
      char *next = (char*) malloc(strlen(path) + strlen(entry_name) + 2);
      sprintf(next, "%s%s/", path, entry_name);

      pID = fork();

      if (pID > 0) {    /* parent */
        create(pID);    /* log process creation */
        waitpid(pID, &status, WUNTRACED);

        /* Getting info from pipe */
        long int tmp;
        if (!arguments.separate_dirs) {  /* check for separated dir size */
          read(fd[READ], &tmp, sizeof(long int));
          recievePipe(tmp); /* log received data from pipe */
          dirSize += tmp;
        }
        /**************************/
      }
      else if (pID == 0) {   /* child */
        fork_read(next, level + 1);  /* recursive calls for deeper levels */
        closedir(dir);               /* directory processed, no need to keep open */
        Exit(0);
      }
      else {   /* failed to fork */
        printf("Failed to fork\n");
        Exit(EXIT_FAILURE);
      }

      if (!arguments.separate_dirs)   /* check for separated dir size */
        dirSize += (arguments.bytes) ? (st_buf.st_size) : (st_buf.st_blocks * 512.0/arguments.block_size);
      free(next);
    }
    else {
      long int fileSize = (arguments.bytes) ? (st_buf.st_size) : (st_buf.st_blocks * 512.0/arguments.block_size);
      dirSize += fileSize;
      if (level < arguments.max_depth && arguments.all) {
        /* Printing info */
        char *toPrint;
        toPrint = (char*) malloc (BUFFER_SIZE * 2 * sizeof(char));
        sprintf(toPrint, "%ld\t%s\n", fileSize, name);
        write(STDOUT_FILENO, toPrint, strlen(toPrint));
        entry(toPrint); /* log new entry registed */
        free(toPrint);
        /*****************/
      }
    }
  }

  if (!arguments.separate_dirs) {
    write(fd[WRITE], &dirSize, sizeof(long int)); /* sending final directory size to the pipe */
    sendPipe(dirSize); /* log sent data to pipe */
  }

  dirSize += (arguments.bytes) ? 4096 : arguments.system_block_size / arguments.block_size;

  if (level <= arguments.max_depth) {
    if (strcmp(path, directory)) {
      if (!strcmp(".", directory))
        path = ".";
      else
        path[strlen(path) - 1] = '\0';
    }
    /* Printing info */
    char *toPrint;
    toPrint = (char*) malloc (BUFFER_SIZE * 2 * sizeof(char));
    sprintf(toPrint, "%ld\t%s\n", dirSize, path);
    write(STDOUT_FILENO, toPrint, strlen(toPrint));
    entry(toPrint);    /* log new entry registed */
    free(toPrint);
    /*****************/
  }

  return dirSize;
}
