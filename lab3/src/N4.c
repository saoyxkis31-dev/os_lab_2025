#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("format: %s rand_seed count\n", argv[0]);
    return 1;
  }
  pid_t sub_process = fork();

  if (sub_process < 0) {
    printf("ne create\n");
    return 1;
  }

  if (sub_process == 0) {
    // дочерний процесс подготавливает параметры для N1
    char *params[] = {"./N1", argv[1], argv[2], NULL};

    // замена образа процесса на N1
    execv("./N1", params);

    perror("execution failed");
    return 1;
  } else {
    int status;
    waitpid(sub_process, &status, 0);
    printf("task finished in sub-process [pid %d]\n", sub_process);
  }

  return 0;
}