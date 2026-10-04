#include <getopt.h>
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

static int ParsePositive(const char *text)
{
    char *end = NULL;
    long value = strtol(text, &end, 10);

    if (*text == '\0' ||
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
    bool with_files = false;

    static struct option options[] = {
        {"seed", required_argument, NULL, 0},
        {"array_size", required_argument, NULL, 0},
        {"pnum", required_argument, NULL, 0},
        {"by_files", no_argument, NULL, 'f'},
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
        pnum > array_size)
    {
        fprintf(
            stderr,
            "Usage: %s --seed N --array_size N "
            "--pnum N [--by_files]\n",
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

    for (int i = 0; i < pnum; ++i)
    {
        pid_t child_pid = fork();

        if (child_pid == -1)
        {
            perror("fork");
            free(pipes);
            free(array);
            return EXIT_FAILURE;
        }

        if (child_pid == 0)
        {
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
    }

    if (!with_files)
    {
        for (int i = 0; i < pnum; ++i)
        {
            close(pipes[i][1]);
        }
    }

    for (int i = 0; i < pnum; ++i)
    {
        int status = 0;

        if (wait(&status) == -1 ||
            !WIFEXITED(status) ||
            WEXITSTATUS(status) != EXIT_SUCCESS)
        {
            fprintf(stderr, "Child process failed\n");

            free(pipes);
            free(array);

            return EXIT_FAILURE;
        }
    }

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