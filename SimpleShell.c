#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h> // Include for gettimeofday
#include <time.h>

#define MAX_HISTORY_SIZE 100
#define MAX_COMMAND_LENGTH 256

// Function to read user input
char *read_user_input() {
    char input[256];
    printf("Enter the command :  ");
    fgets(input, sizeof(input), stdin);
    int len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0'; // Remove newline character
    }
    char *command = strdup(input); // Copy and return the input
    return command;
}

// Function to create a child process and run a command
int create_process_and_run(char *command) {
    int status = fork();
    if (status < 0) {
        perror("fork");
        return 0; // Error in forking
    } else if (status == 0) {
        // This code runs in the child process
        // Split the command into tokens
        char *Words[10]; // Adjust the array size as needed
        int i = 0;
        char *Word = strtok(command, " ");
        while (Word != NULL && i < 10) {
            Words[i++] = Word;
            Word = strtok(NULL, " ");
        }
        Words[i] = NULL; // Null-terminate the argument list

        // Execute the user command using exec
        if (execvp(Words[0], Words) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        // This code runs in the parent process
        int child_status;
        // Wait for the child process to complete
        waitpid(status, &child_status, 0);
        printf("Parent process PID : %d \n", getpid());
        printf("Child process (PID: %d) exited with status %d\n", status, child_status);
    }
    return 1;
}

// Function to maintain command history
void maintain_command_history(char *command, char *history[], int *history_count) {
    if (*history_count < MAX_HISTORY_SIZE) {
        history[*history_count] = strdup(command);
        (*history_count)++;
    }
}

// Function to maintain detailed history
void maintain_detailed_history(char *command, char detailed_history[][5][256], int *detailed_history_count) {
    if (*detailed_history_count < MAX_HISTORY_SIZE) {
        // Store command in detailed history
        strcpy(detailed_history[*detailed_history_count][0], command);

        // Record execution start time in microseconds
        struct timeval start_time;
        gettimeofday(&start_time, NULL);
        snprintf(detailed_history[*detailed_history_count][1], sizeof(detailed_history[*detailed_history_count][1]),
                 "%ld.%06ld", start_time.tv_sec, start_time.tv_usec);

        // Record execution end time (to be calculated later)
        strcpy(detailed_history[*detailed_history_count][2], "");

        // Record duration (to be calculated later)
        strcpy(detailed_history[*detailed_history_count][3], "");

        (*detailed_history_count)++;
    }
}

// Function to calculate duration for each command in detailed history
void calculate_duration(char detailed_history[][5][256], int detailed_history_count) {
    for (int i = 0; i < detailed_history_count; i++) {
        double start_time;
        double end_time;

        sscanf(detailed_history[i][1], "%lf", &start_time);
        sscanf(detailed_history[i][2], "%lf", &end_time);

        // Calculate duration in microseconds
        double duration = (end_time - start_time) * 1e6;
        snprintf(detailed_history[i][3], sizeof(detailed_history[i][3]), "%.6lf", duration);
    }
}

// Function to print command history
void print_command_history(char *history[], int history_count) {
    printf("Command History:\n");
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

// Function to print detailed history
void print_detailed_history(char detailed_history[][5][256], int detailed_history_count) {
    printf("Detailed Command History:\n");
    for (int i = 0; i < detailed_history_count; i++) {
        printf("Command %d:\n", i + 1);
        printf("Command: %s\n", detailed_history[i][0]);
        printf("Execution Start Time: %s seconds\n", detailed_history[i][1]);
        printf("Execution End Time: %s seconds\n", detailed_history[i][2]);
        printf("Duration: %s microseconds\n", detailed_history[i][3]);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    }
}

// Function to launch a command
int launch(char *command, char *history[], char detailed_history[][5][256], int *history_count, int *detailed_history_count) {
    if (strcmp(command, "history") == 0) {
        print_command_history(history, *history_count);
        return 1;
    }

    if (strcmp(command, "detailed_history") == 0) {
        calculate_duration(detailed_history, *detailed_history_count);
        print_detailed_history(detailed_history, *detailed_history_count);
        return 1;
    }

    maintain_command_history(command, history, history_count);

    int status = create_process_and_run(command);

    // Wait for the child process to complete and get its exit status
    int child_status;
    wait(&child_status);

    maintain_detailed_history(command, detailed_history, detailed_history_count);

    // Update the execution end time for the current command in microseconds
    struct timeval end_time;
    gettimeofday(&end_time, NULL);
    snprintf(detailed_history[*detailed_history_count - 1][2], sizeof(detailed_history[*detailed_history_count - 1][2]),
             "%ld.%06ld", end_time.tv_sec, end_time.tv_usec);

    return status;
}

int main() {
    int status;
    char *history[MAX_HISTORY_SIZE];
    char detailed_history[MAX_HISTORY_SIZE][5][256];
    int history_count = 0;
    int detailed_history_count = 0;

    do {
        char *command = read_user_input();
        if (strcmp(command, "exit") == 0)
            break;
        status = launch(command, history, detailed_history, &history_count, &detailed_history_count);
        free(command); // Free dynamically allocated memory
    } while (status);

    // Print the detailed history after the termination of the program
    calculate_duration(detailed_history, detailed_history_count);
    print_detailed_history(detailed_history, detailed_history_count);

    // Free memory for command history
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    return 0;
}
