#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80

int main(void) {
    char *args[MAX_LINE/2 + 1];
    char *history[MAX_LINE/2 + 1] = {NULL};
    int should_run = 1;

    while (should_run) {
        printf("osh> ");
        fflush(stdout);

        // Taking user input and validating it
        char current_command[MAX_LINE + 1];
        if (fgets(current_command, sizeof(current_command), stdin) == NULL) break;
        
        if (strchr(current_command, '\n') == NULL) {
            int c = getchar();
            
            if (c != '\n') {
                while (c != '\n' && c != EOF) c = getchar();
                printf("Invalid command, max length of a command is %d\n", MAX_LINE);
                continue;
            }
        }

        // parsing input
        int current_arg = 0, in_token = 0;
        for (int i = 0; i <= MAX_LINE; ++i) {
            if (current_command[i] == ' ') {
                current_command[i] = '\0';
                if (in_token) {
                    ++current_arg;
                    in_token = 0;
                }

                continue;
            }

            if (current_command[i] == '\n' || current_command[i] == '\0') {
                current_command[i] = '\0';
                if (in_token) ++current_arg;
                break;
            }

            if (!in_token) {
                args[current_arg] = &current_command[i];
                in_token = 1;
            }
        }

        args[current_arg] = NULL;

        if (current_arg == 0) continue;

        // Exiting if user wants
        if (strcmp(args[0], "exit") == 0) break;

        int willWait = 1;
        if (strcmp(args[current_arg - 1], "&") == 0) {
            willWait = 0;
            args[--current_arg] = NULL;
            if (args[0] == NULL) continue;
        }

        if (strcmp(args[0], "!!") != 0) {
            for (int i = 0; history[i] != NULL; ++i) {
                free(history[i]);
                history[i] = NULL;
            }

            for (int i = 0; args[i] != NULL; ++i) 
                history[i] = strdup(args[i]);
        }

        // creating a child process to execute the command
        pid_t pid = fork();
        if (pid == 0) {
            // History execution handling
            if (strcmp(args[0], "!!") == 0 && current_arg == 1) {
                if (history[0] != NULL) {
                    // Echoing the command
                    for (int i = 0; history[i] != NULL; ++i) {
                        printf("%s", history[i]);
                        printf("%c", (history[i + 1] != NULL) ? ' ' : '\n'); 
                    }
                    
                    int j;
                    for (j = 0; history[j] != NULL; ++j) args[j] = history[j];
                    args[j] = NULL;
                } else {
                    printf("No commands in history.\n");
                    exit(0);
                }
            }

            for (int i = 0; args[i] != NULL; ++i) {
                if (strcmp(args[i], "|") == 0) {
                    if (i == 0 || args[i + 1] == NULL) {
                        printf("Invalid pipe\n");
                        exit(1);
                    }

                    int fd[2];
                    if (pipe(fd) < 0) {
                        perror("osh");
                        exit(1);
                    }

                    args[i] = NULL;

                    pid_t left = fork();
                    if (left == 0) {
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[0]);
                        close(fd[1]);
                        execvp(args[0], args);
                        perror("osh");
                        exit(1);
                    }

                    dup2(fd[0], STDIN_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                    execvp(args[i + 1], &args[i + 1]);
                    perror("osh");
                    exit(1);
                }
            }

            for (int i = 0; args[i] != NULL; ++i) {
                if (strcmp(args[i], ">") == 0 || strcmp(args[i], "<") == 0) {
                    if (i == 0 || args[i + 1] == NULL) {
                        printf("Invalid redirection\n");
                        exit(1);
                    }

                    int fd, target;
                    if (args[i][0] == '>') {
                        fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        target = STDOUT_FILENO;
                    } else {
                        fd = open(args[i + 1], O_RDONLY);
                        target = STDIN_FILENO;
                    }

                    if (fd < 0) {
                        perror("osh");
                        exit(1);
                    }

                    dup2(fd, target);
                    close(fd);

                    args[i] = NULL;
                    break;
                }
            }

            execvp(args[0], args);
            perror("osh");
            exit(1);
        } else {
            if (willWait) waitpid(pid, NULL, 0);
        }
    }
}