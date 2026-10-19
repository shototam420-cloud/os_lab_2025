#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
int main(void) {
    pid_t child = fork();
    if (child < 0) { perror("fork"); return 1; }
    if (child == 0) _exit(0);
    printf("Parent PID: %ld, child PID: %ld\n", (long)getpid(), (long)child);
    printf("In another terminal: ps -o pid,ppid,state,cmd -p %ld\n", (long)child);
    puts("Child will remain a zombie for about 30 seconds, then waitpid reaps it.");
    fflush(stdout);
    unsigned left = 30;
    while ((left = sleep(left)) != 0) {}
    while (waitpid(child, NULL, 0) == -1) {
        if (errno != EINTR) { perror("waitpid"); return 1; }
    }
    puts("waitpid completed; child reaped.");
    return 0;
}
