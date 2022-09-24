#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#define DELIMETER " \n\t"
#define ENDOFSTDIN '\0'
#define EMPTYSTRING ' '

void print_error_message()
{
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}
void print_shell_prompt()
{
    printf("TEDShell--2> ");
}

void run_shell(char *cmd, char **pathv, int *pathc, size_t *pathc_limit, char **prev_right_piece)
{

    char *first;
    char *last;
    int errCounter = 0;
    int is_file_descriptor_open = 0;
    char *right_part_of_input = cmd; // chop the command in left side for the command and the right for the piping files.
    strsep(&right_part_of_input, ">");
    if (right_part_of_input != NULL)
    {
        char *filedescriptor_error = right_part_of_input;
        strsep(&filedescriptor_error, ">");
        if (filedescriptor_error != NULL)
        {
            print_error_message();
            return;
        }
        first = right_part_of_input;
        last = right_part_of_input;
        int countOfFiles = 0;
        while (first != NULL)
        {
            strsep(&last, DELIMETER);

            if ((*first != ENDOFSTDIN) && (*first != EMPTYSTRING))
            {
                right_part_of_input = first;
                countOfFiles += 1;
            }

            first = last;
        }

        if (countOfFiles != 1)
        {
            print_error_message();
            return;
        }

        is_file_descriptor_open = 1;
    }

    first = cmd;
    last = cmd;
    int counter1 = 0;
    int input_l = 0;
    int input_size = 10;
    char **user_input = malloc(input_size * sizeof(char *));
    while (first != NULL)
    {
        strsep(&last, DELIMETER);

        if ((*first != ENDOFSTDIN) && (*first != EMPTYSTRING))
        {
            counter1++;
            if (counter1 < 0)
            {
                print_error_message();
            }

            if (input_l + 1 >= input_size)
            {

                input_size *= 2;
                char **temp = realloc(user_input, input_size * sizeof(char *));
                if (temp != NULL)
                {
                    user_input = temp;
                }
                else
                {
                    print_error_message();
                    exit(1);
                }
            }
            user_input[input_l++] = first;
        }

        first = last;
    }
    user_input[input_l] = NULL;

    if (is_file_descriptor_open && input_l == 0)
    {
        print_error_message();
        return;
    }
    int try_inputl2;
    if (try_inputl2 < 1)
    {
        return;
    }
    if (input_l < 1)
    {
        return;
    }
    if (input_l == 0)
    {
        return;
    }

    if (strcasecmp(user_input[0], "exit") == 0)
    {
        if (input_l == 0) 
        {
            return;
        }

        if (input_l > 1)
        {
            print_error_message();
            return;
        }
        exit(0);
    }
    else if (strcasecmp(user_input[0], "cd") == 0)
    {
        if (input_l == 0)
        {
            return;
        }

        if (input_l != 2)
        {
            print_error_message();
            return;
        }
        if (chdir(user_input[1]) != 0)
        {
            print_error_message();
            return;
        }
    }
    else if (strcasecmp(user_input[0], "path") == 0)
    {
        if (input_l == 0) //*
        {
            return;
        }
        for (int i = 0; i < *pathc; i++)
        {
            free(pathv[i]);
        }

        *pathc = 0;
        int do_realloc = 0;
        while (input_l - 1 > *pathc_limit)
        {
            do_realloc = 1;
            *pathc_limit *= 2;
        }

        if (do_realloc)
        {
            char **tmp_pathv = realloc(pathv, *pathc_limit * sizeof(char *));
            if (tmp_pathv != NULL)
            {
                pathv = tmp_pathv;
            }
            else
            {
                print_error_message();
                exit(1);
            }
        }
        for (int i = 1; i < input_l; i++)
        {
            pathv[*pathc] = strdup(user_input[i]);
            *pathc += 1;
        }
    }
    else
    {

        int found_program = 0;
        char path2program[100];
        for (int i = 0; i < *pathc; i++)
        {
            strcpy(path2program, pathv[i]);

            if (pathv[i][strlen(pathv[i]) - 1] != '/')
            {
                strcat(path2program, "/");
            }
            strcat(path2program, user_input[0]);

            if (access(path2program, X_OK) == 0)
            {
                found_program = 1;
                break;
            }
        }

        if (!found_program)
        {
            print_error_message();
            return;
        }

        if (is_file_descriptor_open)
        {
            if (*prev_right_piece == NULL)
            {
                *prev_right_piece = strdup(right_part_of_input);
            }
            else
            {
                if (strcasecmp(right_part_of_input, *prev_right_piece) == 0)
                {
                    return;
                }
            }
        }

        int rc = fork();
        if (rc < 0)
        {
            exit(1);
        }
        else if (rc == 0)
        {

            if (is_file_descriptor_open)
            {

                int fd;
                if ((fd = open(right_part_of_input, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU)) == -1)
                {
                    print_error_message();

                    exit(1);
                }

                int dup_err;
                if ((dup_err = dup2(fd, STDOUT_FILENO)) == -1)
                {
                    print_error_message();
                    exit(1);
                };

                if ((dup_err = dup2(fd, STDERR_FILENO)) == -1)
                {
                    print_error_message();
                    exit(1);
                };

                if (close(fd) == -1)
                {
                    print_error_message();
                    exit(1);
                }
            }

            execv(path2program, user_input);
            exit(0);
        }
        else
        {
            int rc_wait = wait(NULL);
        }
    }

    free(user_input);
}
freeMemoryAll(char *left_part_of_input, char *pathv, char *cli)
{

    free(left_part_of_input);
    free(pathv);
}

int main(int arg_int, char *arg_intv[])
{
    int arg_int2 = 1;
    int err_count = 0;
    FILE *inputStream;
    if (arg_int == 0)
    { // interactive mode
        inputStream = fopen(arg_intv[1], "r");
        if (arg_int2 == 0)
        {
            print_error_message();
            err_count++;
        }
        if (inputStream == NULL)
        {
            print_error_message();
            err_count++;
            exit(1);
        }
    }
    else if (arg_int == 1)
    {
        inputStream = stdin;
        print_shell_prompt();
    }
    else
    {
        print_error_message();
        exit(1);
    }

    int numOfStoredPaths = 1; // number of paths stored
    size_t pathStored_limit = 10;
    char **pathv = malloc(pathStored_limit * sizeof(char *));
    pathv[0] = strdup("/bin");
    size_t lineController = 0;
    char *cli = NULL;

    char *left_part_of_input = NULL;
    while ((lineController = getline(&cli, &lineController, inputStream)) != -1)
    {

        char *begin = cli;
        char *end = cli;
        while (begin != NULL)
        {
            strsep(&end, "&");
            run_shell(begin, pathv, &numOfStoredPaths, &pathStored_limit, &left_part_of_input);
            begin = end;
        }

        if (arg_int == 0)
        {
            print_shell_prompt();
        }
    }
    freeMemoryAll(left_part_of_input, pathv, cli);
    exit(0);
}