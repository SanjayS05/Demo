#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

// Function to allocate memory to blocks as per First fit algorithm
void firstFit(int blockSize[], int m, int processSize[], int n, int *allocation)
{
    int i, j;

    // pick each process and find suitable blocks according to its size and assign to it
    for (i = 0; i < n; i++) //here, n -> number of processes
    {
        for (j = 0; j < m; j++) //here, m -> number of blocks
        {
            if (blockSize[j] >= processSize[i])
            {
                // allocating block j to the ith process
                allocation[i] = j;

                // Reduce available memory in this block.
                blockSize[j] -= processSize[i];

                break; //go to the next process in the queue
            }
        }
    }
}

// Function to allocate memory to blocks as per Worst fit algorithm
void worstFit(int blockSize[], int m, int processSize[], int n, int *allocation)
{
    int i, j;

    // pick each process and find the worst fit block
    for (i = 0; i < n; i++) //here, n -> number of processes
    {
        // Find the worst fit block index
        int worstFitIdx = -1;
        for (j = 0; j < m; j++) //here, m -> number of blocks
        {
            if (blockSize[j] >= processSize[i])
            {
                if (worstFitIdx == -1 || blockSize[j] > blockSize[worstFitIdx])
                    worstFitIdx = j;
            }
        }

        if (worstFitIdx != -1)
        {
            // allocating block worstFitIdx to the ith process
            allocation[i] = worstFitIdx;

            // Reduce available memory in this block.
            blockSize[worstFitIdx] -= processSize[i];
        }
    }
}

// Function to allocate memory to blocks as per Best fit algorithm
void bestFit(int blockSize[], int m, int processSize[], int n, int *allocation)
{
    int i, j;

    // pick each process and find the best fit block
    for (i = 0; i < n; i++) //here, n -> number of processes
    {
        // Find the best fit block index
        int bestFitIdx = -1;
        for (j = 0; j < m; j++) //here, m -> number of blocks
        {
            if (blockSize[j] >= processSize[i])
            {
                if (bestFitIdx == -1 || blockSize[j] < blockSize[bestFitIdx])
                    bestFitIdx = j;
            }
        }

        if (bestFitIdx != -1)
        {
            // allocating block bestFitIdx to the ith process
            allocation[i] = bestFitIdx;

            // Reduce available memory in this block.
            blockSize[bestFitIdx] -= processSize[i];
        }
    }
}

// Function to allocate memory to blocks as per Next fit algorithm
void nextFit(int blockSize[], int m, int processSize[], int n, int *allocation)
{
    int i, j;
    int lastAllocated = 0; // stores the index of the last allocated block

    // pick each process and find suitable blocks according to its size and assign to it
    for (i = 0; i < n; i++) //here, n -> number of processes
    {
        for (j = lastAllocated; j < m; j++) //here, m -> number of blocks
        {
            if (blockSize[j] >= processSize[i])
            {
                // allocating block j to the ith process
                allocation[i] = j;

                // Reduce available memory in this block.
                blockSize[j] -= processSize[i];

                lastAllocated = j; // update the last allocated block index

                break; //go to the next process in the queue
            }
        }
    }
}

int main()
{
    int m;                                //number of blocks in the memory
    int n;                                //number of processes in the input queue
    int blockSize[] = {100, 500, 200, 300, 600};
    int processSize[] = {212, 417, 112, 426};
    m = sizeof(blockSize) / sizeof(blockSize[0]);
    n = sizeof(processSize) / sizeof(processSize[0]);

    int choice;
    printf("Select memory allocation algorithm:\n");
    printf("1. First Fit\n");
    printf("2. Worst Fit\n");
    printf("3. Best Fit\n");
    printf("4. Next Fit\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    int fd1[2], fd2[2];
    pid_t pid;

    if (pipe(fd1) == -1 || pipe(fd2) == -1)
    {
        fprintf(stderr, "Pipe failed");
        return 1;
    }

    pid = fork();

    if (pid < 0)
    {
        fprintf(stderr, "Fork failed");
        return 1;
    }

    if (pid > 0)
    { // Parent process
        close(fd1[0]); // Close reading end of first pipe
        close(fd2[1]); // Close writing end of second pipe

        int allocation[n];

        // Write user choice to pipe
        write(fd1[1], &choice, sizeof(choice));

        // Read the result from the child process
        read(fd2[0], allocation, sizeof(allocation));

        printf("\nProcess No.\tProcess Size\tBlock no.\n");
        for (int i = 0; i < n; i++)
        {
            printf(" %i\t\t\t", i + 1);
            printf("%i\t\t\t\t", processSize[i]);
            if (allocation[i] != -1)
                printf("%i", allocation[i] + 1);
            else
                printf("Not Allocated");
            printf("\n");
        }

        close(fd1[1]); // Close writing end of first pipe
        close(fd2[0]); // Close reading end of second pipe

        wait(NULL);
    }
    else
    { // Child process
        close(fd1[1]); // Close writing end of first pipe
        close(fd2[0]); // Close reading end of second pipe

        int choice;
        read(fd1[0], &choice, sizeof(choice));

        int allocation[n];
        for (int i = 0; i < n; i++)
            allocation[i] = -1;

        switch (choice)
        {
        case 1:
            firstFit(blockSize, m, processSize, n, allocation);
            break;
        case 2:
            worstFit(blockSize, m, processSize, n, allocation);
            break;
        case 3:
            bestFit(blockSize, m, processSize, n, allocation);
            break;
        case 4:
            nextFit(blockSize, m, processSize, n, allocation);
            break;
        default:
            fprintf(stderr, "Invalid choice!\n");
        }

        // Write the result to the parent process
        write(fd2[1], allocation, sizeof(allocation));

        close(fd1[0]); // Close reading end of first pipe
        close(fd2[1]); // Close writing end of second pipe
    }

    return 0;
}
