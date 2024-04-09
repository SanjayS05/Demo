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

//
