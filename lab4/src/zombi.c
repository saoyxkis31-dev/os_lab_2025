#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    printf("parent start, PID: %d\n", getpid());
    pid_t worker_pid = fork();
    if (worker_pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (worker_pid == 0) {
        printf("child start, PID: %d\n", getpid());
        printf("child  done work gg\n");
        exit(0);
    }

    printf("parent creat child with PID: %d\n", worker_pid);
    printf("parent sleeping 25 seconds\n");
    printf(" run ps aux | grep Z\n");
    sleep(20); 
    printf("\nparent woke up\n");
    wait(NULL);
    sleep(10);
    return 0;
}