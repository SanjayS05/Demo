//Waiting professor
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_STUDENTS 10

sem_t student_sem;
sem_t professor_sem;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int num_students_waiting = 0;

void* professor_thread(void* arg) {
    while (1) {
        // Check if there are students waiting
        pthread_mutex_lock(&mutex);
        if (num_students_waiting == 0) {
            pthread_mutex_unlock(&mutex);
            printf("Professor is idle and going to sleep.\n");
            sem_wait(&professor_sem); // Professor sleeps
        } else {
            // Signal one student to enter the office
            sem_post(&student_sem);
            num_students_waiting--;
            pthread_mutex_unlock(&mutex);

            // Discuss a topic with the student
            printf("Professor is discussing a topic with a student.\n");
            // ...

            // Student goes back to studying, professor calls IdleProf again
        }
    }

    return NULL;
}

void* student_thread(void* arg) {
    int student_id = *(int*)arg;

    // Student arrives and waits outside the office
    pthread_mutex_lock(&mutex);
    num_students_waiting++;
    pthread_mutex_unlock(&mutex);

    printf("Student %d is waiting outside the professor's office.\n", student_id);

    // Check if professor is sleeping, wake them up if necessary
    pthread_mutex_lock(&mutex);
    if (num_students_waiting == 1) {
        sem_post(&professor_sem);
    }
    pthread_mutex_unlock(&mutex);

    // Wait for professor's signal to enter the office
    sem_wait(&student_sem);

    // Discuss a topic with the professor
    printf("Student %d is discussing a topic with the professor.\n", student_id);
    // ...

    // Student goes back to studying

    return NULL;
}

int main() {
    pthread_t professor;
    pthread_t students[MAX_STUDENTS];

    // Initialize semaphores
    sem_init(&student_sem, 0, 5);
    sem_init(&professor_sem, 0, 0);

    // Create professor thread
    pthread_create(&professor, NULL, professor_thread, NULL);

    // Create student threads
    int student_ids[MAX_STUDENTS];
    for (int i = 0; i < MAX_STUDENTS; i++) {
        student_ids[i] = i + 1;
        pthread_create(&students[i], NULL, student_thread, &student_ids[i]);
    }

    // Wait for professor thread to finish (which will never happen)
    pthread_join(professor, NULL);

    // Wait for student threads to finish (which will never happen)
    for (int i = 0; i < MAX_STUDENTS; i++) {
        pthread_join(students[i], NULL);
    }

    // Destroy semaphores
    sem_destroy(&student_sem);
    sem_destroy(&professor_sem);

    return 0;
}

//Pizza 
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_STUDENTS 5
#define PIZZA_SLICES 8

sem_t pizza_mutex;          // Mutex to control access to pizza
sem_t pizza_available;      // Semaphore to signal availability of pizza
int pizza_slices = PIZZA_SLICES; // Number of slices available

void* student(void* arg) {
    int student_id = *(int*)arg;
    while (1) {
        sem_wait(&pizza_mutex); // Wait for pizza mutex
        if (pizza_slices == 0) { // If pizza is exhausted
            sem_post(&pizza_mutex); // Release pizza mutex
            sem_wait(&pizza_available); // Wait for pizza to arrive
        } else {
            pizza_slices--; // Consume a slice of pizza
            printf("Student %d takes a slice of pizza, remaining slices: %d\n", student_id, pizza_slices);
            sem_post(&pizza_mutex); // Release pizza mutex
            usleep(2000000); // Sleep to simulate eating time
        }
    }
    return NULL;
}

void* kamal_pizza_delivery(void* arg) {
    while (1) {
        sem_wait(&pizza_mutex); // Wait for pizza mutex
        if (pizza_slices > 0) { // If pizza is not exhausted, release mutex and continue
            sem_post(&pizza_mutex);
            continue;
        }
        // If pizza is exhausted, call Kamal's Pizza
        printf("Pizza is exhausted. Calling Kamal's Pizza for another pizza...\n");
        pizza_slices = PIZZA_SLICES; // Reset pizza slices
        sem_post(&pizza_available); // Signal availability of pizza
        sem_post(&pizza_mutex); // Release pizza mutex
        usleep(2000000); // Sleep to simulate pizza delivery time
    }
    return NULL;
}

int main() {
    pthread_t students[NUM_STUDENTS], kamal_thread;
    int student_ids[NUM_STUDENTS];

    sem_init(&pizza_mutex, 0, 1); // Initialize pizza mutex
    sem_init(&pizza_available, 0, 0); // Initialize pizza available semaphore

    // Create student threads
    for (int i = 0; i < NUM_STUDENTS; i++) {
        student_ids[i] = i + 1;
        pthread_create(&students[i], NULL, student, &student_ids[i]);
    }

    // Create Kamal's pizza delivery thread
    pthread_create(&kamal_thread, NULL, kamal_pizza_delivery, NULL);

    // Join student threads
    for (int i = 0; i < NUM_STUDENTS; i++) {
        pthread_join(students[i], NULL);
    }

    // Join Kamal's pizza delivery thread
    pthread_join(kamal_thread, NULL);

    // Clean up resources
    sem_destroy(&pizza_mutex);
    sem_destroy(&pizza_available);

    return 0;
}

//Dining Philosopher
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define LEFT (phnum + 4) % NUM_PHILOSOPHERS
#define RIGHT (phnum + 1) % NUM_PHILOSOPHERS

sem_t mutex;
sem_t S[NUM_PHILOSOPHERS];

void* philosopher(void* num);
void take_fork(int phnum);
void put_fork(int phnum);
void test(int phnum);

int state[NUM_PHILOSOPHERS];
int phil[NUM_PHILOSOPHERS] = {0, 1, 2, 3, 4};

int main() {
    pthread_t thread_id[NUM_PHILOSOPHERS];
    sem_init(&mutex, 0, 1);

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        sem_init(&S[i], 0, 0);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_create(&thread_id[i], NULL, philosopher, &phil[i]);
        printf("Philosopher %d is thinking\n", i + 1);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(thread_id[i], NULL);
    }

    return 0;
}

void* philosopher(void* num) {
    while (1) {
        int* i = num;
        sleep(1);
        take_fork(*i);
        sleep(0);
        put_fork(*i);
    }
}

void take_fork(int phnum) {
    sem_wait(&mutex);
    state[phnum] = HUNGRY;
    printf("Philosopher %d is hungry\n", phnum + 1);
    test(phnum);
    sem_post(&mutex);
    sem_wait(&S[phnum]);
    sleep(1);
}

void put_fork(int phnum) {
    sem_wait(&mutex);
    state[phnum] = THINKING;
    printf("Philosopher %d putting forks %d and %d down\n", phnum + 1, LEFT + 1, phnum + 1);
    printf("Philosopher %d is thinking\n", phnum + 1);
    test(LEFT);
    test(RIGHT);
    sem_post(&mutex);
}

void test(int phnum) {
    if (state[phnum] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[phnum] = EATING;
        sleep(2);
        printf("Philosopher %d takes forks %d and %d\n", phnum + 1, LEFT + 1, phnum + 1);
        printf("Philosopher %d is eating\n", phnum + 1);
        sem_post(&S[phnum]);
    }
}

//Producer-consumer
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
#define MAX_VALUE 100

int buffer[BUFFER_SIZE];
sem_t full, empty;
pthread_mutex_t mutex;

void* producer(void* arg) {
    int item;
    while (1) {
        item = rand() % MAX_VALUE; // Produce a random item
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        // Critical section: Add item to the buffer
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (buffer[i] == -1) {
                buffer[i] = item;
                printf("Producer produced item: %d\n", item);
                break;
            }
        }
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
        usleep(rand() % 1000000); // Sleep for a random amount of time
    }
}

void* consumer(void* arg) {
    int item;
    while (1) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        // Critical section: Remove item from the buffer
        for (int i = BUFFER_SIZE - 1; i >= 0; i--) {
            if (buffer[i] != -1) {
                item = buffer[i];
                buffer[i] = -1;
                printf("Consumer consumed item: %d\n", item);
                break;
            }
        }
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
        usleep(rand() % 1000000); // Sleep for a random amount of time
    }
}

int main() {
    pthread_t producers[NUM_PRODUCERS], consumers[NUM_CONSUMERS];
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    // Initialize buffer
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = -1;
    }

    // Create producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_create(&producers[i], NULL, producer, NULL);
    }

    // Create consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer, NULL);
    }

    // Join producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }

    // Join consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    // Clean up resources
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}
