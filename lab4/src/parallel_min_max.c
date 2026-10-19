#define _POSIX_C_SOURCE 200809L
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "find_min_max.h"
#include "utils.h"

static volatile sig_atomic_t timed_out = 0;

static void OnAlarm(int signal_number)
{
    (void)signal_number;
    timed_out = 1;
}

static void StopChildren(pid_t *pids, int count)
{
    for (int i = 0; i < count; ++i)
        if (pids[i] > 0) kill(pids[i], SIGKILL);
    for (int i = 0; i < count; ++i)
        if (pids[i] > 0)
            while (waitpid(pids[i], NULL, 0) < 0 && errno == EINTR) {}
}

static int ParsePositive(const char *text)
{
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);

    if (errno || *text == '\0' ||
        *end != '\0' ||
        value <= 0 ||
        value > INT_MAX)
    {
        return -1;
    }

    return (int)value;
}

static void MakeFileName(
    char *buffer,
    size_t buffer_size,
    pid_t parent_pid,
    int process_index)
{
    snprintf(
        buffer,
        buffer_size,
        "/tmp/lab3_%ld_%d.txt",
        (long)parent_pid,
        process_index);
}

int main(int argc, char **argv)
{
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    int timeout_seconds = 0;
    bool with_files = false;

    static struct option options[] = {
        {"seed", required_argument, NULL, 0},
        {"array_size", required_argument, NULL, 0},
        {"pnum", required_argument, NULL, 0},
        {"by_files", no_argument, NULL, 'f'},
        {"timeout", required_argument, NULL, 't'},
        {NULL, 0, NULL, 0}
    };

    while (true)
    {
        int option_index = 0;

        int option = getopt_long(
            argc,
            argv,
            "f",
            options,
            &option_index);

        if (option == -1)
        {
            break;
        }

        if (option == 'f')
        {
            with_files = true;
        }
        else if (option == 't')
        {
            timeout_seconds = ParsePositive(optarg);
        }
        else if (option == 0)
        {
            int value = ParsePositive(optarg);

            if (option_index == 0)
            {
                seed = value;
            }
            else if (option_index == 1)
            {
                array_size = value;
            }
            else if (option_index == 2)
            {
                pnum = value;
            }
        }
        else
        {
            return EXIT_FAILURE;
        }
    }

    if (optind != argc ||
        seed < 1 ||
        array_size < 1 ||
        pnum < 1 ||
        timeout_seconds < 0 ||
        pnum > array_size)
    {
        fprintf(
            stderr,
            "Usage: %s --seed N --array_size N "
            "--pnum N [--by_files] [--timeout SECONDS]\n",
            argv[0]);

        return EXIT_FAILURE;
    }

    int *array = malloc(sizeof(*array) * array_size);

    if (array == NULL)
    {
        perror("malloc");
        return EXIT_FAILURE;
    }

    GenerateArray(array, array_size, seed);

    int (*pipes)[2] = NULL;

    if (!with_files)
    {
        pipes = malloc(sizeof(*pipes) * pnum);

        if (pipes == NULL)
        {
            perror("malloc");
            free(array);
            return EXIT_FAILURE;
        }

        for (int i = 0; i < pnum; ++i)
        {
            if (pipe(pipes[i]) == -1)
            {
                perror("pipe");
                free(pipes);
                free(array);
                return EXIT_FAILURE;
            }
        }
    }

    pid_t parent_pid = getpid();

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    pid_t *pids = calloc((size_t)pnum, sizeof(*pids));
    if (pids == NULL) { perror("calloc"); return EXIT_FAILURE; }
    if (signal(SIGALRM, OnAlarm) == SIG_ERR) {
        perror("signal"); return EXIT_FAILURE;
    }
    if (timeout_seconds > 0) alarm((unsigned)timeout_seconds);

    for (int i = 0; i < pnum; ++i)
    {
        if (timed_out) break;
        pid_t child_pid = fork();

        if (child_pid == -1)
        {
            perror("fork");
            alarm(0);
            StopChildren(pids, i);
            if (with_files) {
                for (int j = 0; j < i; ++j) {
                    char name[128];
                    MakeFileName(name, sizeof(name), parent_pid, j);
                    remove(name);
                }
            }
            free(pids);
            free(pipes);
            free(array);
            return EXIT_FAILURE;
        }

        if (child_pid == 0)
        {
#ifdef DEMO_SLOW_CHILD
            /* Only the separate demo build delays children. */
            sleep(3);
#endif
            unsigned int begin =
                (unsigned int)(
                    (long long)i * array_size / pnum);

            unsigned int end =
                (unsigned int)(
                    (long long)(i + 1) *
                    array_size / pnum);

            struct MinMax part =
                GetMinMax(array, begin, end);

            if (with_files)
            {
                char file_name[128];

                MakeFileName(
                    file_name,
                    sizeof(file_name),
                    parent_pid,
                    i);

                FILE *file = fopen(file_name, "w");

                if (file == NULL)
                {
                    perror("fopen");
                    _exit(EXIT_FAILURE);
                }

                fprintf(
                    file,
                    "%d %d\n",
                    part.min,
                    part.max);

                fclose(file);
            }
            else
            {
                for (int j = 0; j < pnum; ++j)
                {
                    close(pipes[j][0]);

                    if (j != i)
                    {
                        close(pipes[j][1]);
                    }
                }

                ssize_t written = write(
                    pipes[i][1],
                    &part,
                    sizeof(part));

                close(pipes[i][1]);

                if (written != (ssize_t)sizeof(part))
                {
                    _exit(EXIT_FAILURE);
                }
            }

            _exit(EXIT_SUCCESS);
        }
        pids[i] = child_pid;
    }

    if (!with_files)
    {
        for (int i = 0; i < pnum; ++i)
        {
            close(pipes[i][1]);
        }
    }

    int remaining = pnum;
    int failed = 0;
    while (remaining > 0)
    {
        if (timed_out) { failed = 124; break; }
        int status = 0;
        pid_t done = waitpid(-1, &status, timeout_seconds ? WNOHANG : 0);
        if (done == 0) {
            const struct timespec pause_time = {0, 10000000};
            nanosleep(&pause_time, NULL);
            continue;
        }
        if (done == -1) {
            if (errno == EINTR) continue;
            perror("waitpid"); failed = 1; break;
        }
        for (int j = 0; j < pnum; ++j)
            if (pids[j] == done) { pids[j] = 0; break; }
        --remaining;
        if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
            fprintf(stderr, "Child process failed\n"); failed = 1; break;
        }
    }
    alarm(0);
    if (failed) {
        StopChildren(pids, pnum);
        for (int j = 0; j < pnum; ++j) {
            if (with_files) {
                char name[128];
                MakeFileName(name, sizeof(name), parent_pid, j);
                remove(name);
            } else close(pipes[j][0]);
        }
        if (failed == 124)
            fprintf(stderr, "Timeout: %d seconds; remaining children killed and reaped\n", timeout_seconds);
        free(pids); free(pipes); free(array);
        return failed;
    }
    free(pids);

    struct MinMax total;

    total.min = INT_MAX;
    total.max = INT_MIN;

    for (int i = 0; i < pnum; ++i)
    {
        struct MinMax part;

        if (with_files)
        {
            char file_name[128];

            MakeFileName(
                file_name,
                sizeof(file_name),
                parent_pid,
                i);

            FILE *file = fopen(file_name, "r");

            if (file == NULL)
            {
                perror("fopen");
                free(array);
                return EXIT_FAILURE;
            }

            if (fscanf(
                    file,
                    "%d %d",
                    &part.min,
                    &part.max) != 2)
            {
                fprintf(stderr, "Cannot read result file\n");

                fclose(file);
                free(array);

                return EXIT_FAILURE;
            }

            fclose(file);
            remove(file_name);
        }
        else
        {
            ssize_t received = read(
                pipes[i][0],
                &part,
                sizeof(part));

            close(pipes[i][0]);

            if (received != (ssize_t)sizeof(part))
            {
                fprintf(stderr, "Cannot read pipe\n");

                free(pipes);
                free(array);

                return EXIT_FAILURE;
            }
        }

        if (part.min < total.min)
        {
            total.min = part.min;
        }

        if (part.max > total.max)
        {
            total.max = part.max;
        }
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time =
        (finish_time.tv_sec - start_time.tv_sec) *
        1000.0;

    elapsed_time +=
        (finish_time.tv_usec - start_time.tv_usec) /
        1000.0;

    free(pipes);
    free(array);

    printf("Min: %d\n", total.min);
    printf("Max: %d\n", total.max);
    printf("Elapsed time: %.3fms\n", elapsed_time);

    return EXIT_SUCCESS;
}
