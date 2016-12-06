#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void createThread(int);

int main(int argc, char *argv[]){
  // first check that an argument was passed
  if (argc != 2){
    printf("ERROR: program must be passed exactly one integer argument\n");
    exit(-1);
  }

  int numThreads = strtol(argv[1], NULL, 10);

  if (numThreads > 0){
    createThread(numThreads - 1);
  }

  exit(0);
}

void createThread(int numberOfChildren){
  printf("Hello from process: %i\n", getpid());

  if (numberOfChildren > 0){
    pid_t pid = fork();

    if (pid == 0){
      createThread(numberOfChildren - 1);
    } else if (pid < 0){
      printf("There was an error creating a child process\n");
      exit(-1);
    } else {
      int status = 0;
      waitpid(pid, &status, 0);
      printf("Goodbye from process: %i\n", getpid());
    }
  } else {
    printf("Goodbye from process: %i\n", getpid());
  }
}
