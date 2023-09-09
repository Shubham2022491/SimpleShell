#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_HISTORY_SIZE 100

// Function to read user input
char* read_user_input() {
    char input[256];
    printf("Enter the command :  ");
    fgets(input, sizeof(input), stdin);
    int len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0'; // Remove newline character
    }
    char* command = strdup(input); // Copy and return the input
    return command;
}

// Function to create a child process and run a command
int create_process_and_run(char* command) {
    int status = fork();
    if (status < 0) {
        perror("fork");
        return 0; // Error in forking
    } else if (status == 0) {
        // This code runs in the child process
        // Split the command into tokens
        char* tokens[10]; // Adjust the array size as needed
        int i = 0;
        char* token = strtok(command, " ");
        while (token != NULL && i < 10) {
            tokens[i++] = token;
            token = strtok(NULL, " ");
        }
        tokens[i] = NULL; // Null-terminate the argument list

        // Execute the user command using exec
        if (execvp(tokens[0], tokens) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        // This code runs in the parent process
        int child_status;
        // Wait for the child process to complete
        waitpid(status, &child_status, 0);
        printf("Child process exited with status %d\n", child_status);
    }
    return 1;
}

// Function to maintain command history
void maintain_history(char* command, char* history[], int* history_count) {
    if (*history_count < MAX_HISTORY_SIZE) {
        history[*history_count] = strdup(command);
        (*history_count)++;
    } else {
        // Shift the existing history to accommodate the new command
        for (int i = 0; i < MAX_HISTORY_SIZE - 1; i++) {
            free(history[i]);
            history[i] = strdup(history[i + 1]);
        }
        free(history[MAX_HISTORY_SIZE - 1]);
        history[MAX_HISTORY_SIZE - 1] = strdup(command);
    }
}

// Function to print command history
void print_history(char* history[], int history_count) {
    printf("Command History:\n");
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

// Function to launch a command
int launch(char *command, char* history[], int* history_count) {
    if (strcmp(command, "history") == 0) {
        print_history(history, *history_count);
        return 1;
    }
    
    maintain_history(command, history, history_count);
    
    int status = create_process_and_run(command);
    return status;
}

int main() {
    int status;
    char* history[MAX_HISTORY_SIZE];
    int history_count = 0;
    
    do {
        char* command = read_user_input();
        if (strcmp(command, "exit") == 0)
            break;
        status = launch(command, history, &history_count);
        free(command); // Free dynamically allocated memory
    } while (status);

    // Free history commands
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    return 0;
}
