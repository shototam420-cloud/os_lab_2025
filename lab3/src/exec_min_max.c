#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(
            stderr,
            "Usage: %s seed arraysize\n",
            argv[0]);

        return EXIT_FAILURE;
    }

    pid_t child_pid = fork();

    if (child_pid == -1)
    {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (child_pid == 0)
    {
        execl(
            "./sequential_min_max",
            "sequential_min_max",
            argv[1],
            argv[2],
            (char *)NULL);

        perror("execl");
        _exit(127);
    }

    int status = 0;

    if (waitpid(child_pid, &status, 0) == -1)
    {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status))
    {
        int exit_code = WEXITSTATUS(status);

        printf(
            "Child exit code: %d\n",
            exit_code);

        return exit_code;
    }

    fprintf(
        stderr,
        "Child terminated abnormally\n");

    return EXIT_FAILURE;
}