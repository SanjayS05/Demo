#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define NUM_HOLES 8

// Function prototypes
void perform_allocation(int holes[], int num_requests, int algorithm);

int main() {
    int holes[] = {10, 4, 20, 18, 7, 9, 12, 15}; // Sizes of memory holes in MB
    int num_requests;
    int algorithm;

    // Create pipe for communication
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    // Fork a child process
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Child process
        // Close write end of the pipe
        close(pipe_fd[1]);
        // Read the number of requests and algorithm choice from the pipe
        read(pipe_fd[0], &num_requests, sizeof(num_requests));
        read(pipe_fd[0], &algorithm, sizeof(algorithm));
        // Perform memory allocation for the given number of requests using the specified algorithm
        perform_allocation(holes, num_requests, algorithm);
        // Close read end of the pipe
        close(pipe_fd[0]);
    } else { // Parent process
        // Close read end of the pipe
        close(pipe_fd[0]);
        // Send the number of requests and algorithm choice to the child process
        printf("Enter the number of requests: ");
        scanf("%d", &num_requests);
        printf("Choose allocation algorithm:\n1. First Fit\n2. Best Fit\n3. Worst Fit\n4. Next Fit\nEnter your choice: ");
        scanf("%d", &algorithm);
        write(pipe_fd[1], &num_requests, sizeof(num_requests));
        write(pipe_fd[1], &algorithm, sizeof(algorithm));
        // Close write end of the pipe
        close(pipe_fd[1]);
        // Wait for the child process to finish
        wait(NULL);
    }

    return 0;
}

// Function to perform memory allocation using first-fit algorithm
void perform_allocation(int holes[], int num_requests, int algorithm) {
    printf("\nMemory Allocation:\n");
    int i, j;
    int last_index = 0; // For next fit algorithm
    for (i = 0; i < num_requests; i++) {
        int request_size;
        printf("Enter the size of request %d: ", i + 1);
        scanf("%d", &request_size);
        int hole_index = -1;
        switch (algorithm) {
            case 1: // First Fit
                for (j = 0; j < NUM_HOLES; j++) {
                    if (holes[j] >= request_size) {
                        hole_index = j;
                        break;
                    }
                }
                break;
            case 2: // Best Fit
                hole_index = -1;
                for (j = 0; j < NUM_HOLES; j++) {
                    if (holes[j] >= request_size && (hole_index == -1 || holes[j] < holes[hole_index])) {
                        hole_index = j;
                    }
                }
                break;
            case 3: // Worst Fit
                hole_index = -1;
                for (j = 0; j < NUM_HOLES; j++) {
                    if (holes[j] >= request_size && (hole_index == -1 || holes[j] > holes[hole_index])) {
                        hole_index = j;
                    }
                }
                break;
            case 4: // Next Fit
                hole_index = -1;
                for (j = last_index; j < NUM_HOLES; j++) {
                    if (holes[j] >= request_size) {
                        hole_index = j;
                        last_index = (j + 1) % NUM_HOLES; // Update last_index for next fit
                        break;
                    }
                }
                if (hole_index == -1) {
                    for (j = 0; j < last_index; j++) {
                        if (holes[j] >= request_size) {
                            hole_index = j;
                            last_index = (j + 1) % NUM_HOLES; // Update last_index for next fit
                            break;
                        }
                    }
                }
                break;
            default:
                printf("Invalid choice. Exiting.\n");
                return;
        }
        if (hole_index != -1) {
            printf("Request %d of size %d MB allocated to hole %d of size %d MB\n", i + 1, request_size, hole_index + 1, holes[hole_index]);
            holes[hole_index] -= request_size; // Reduce the hole size
        } else {
            printf("Request %d of size %d MB cannot be allocated. No suitable hole found.\n", i + 1, request_size);
        }
    }
}
