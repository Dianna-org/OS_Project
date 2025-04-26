#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

// Define maximum number of hunts and treasures
#define MAX_HUNTS 10
#define MAX_TREASURES 100
#define ID_MAX_LENGTH 16
#define USER_NAME_MAX_LENGTH 32
#define CLUE_TEXT_MAX_LENGTH 800
#define TREASURE_BLOCK_LENGTH (sizeof(treasure_data))

// Structure to store information about hunts
typedef struct {
    int id;
    char name[50];
    int treasures;
} Hunt;

// Structure to store information about treasures
typedef struct {
    char treasure_id[ID_MAX_LENGTH];
    char user_name[USER_NAME_MAX_LENGTH];
    char clue_text[CLUE_TEXT_MAX_LENGTH];
    float latitude;
    float longitude;
    int value;
} Treasure;


// Global variables to store hunts, treasures, and their counts
Hunt hunts[MAX_HUNTS];
Treasure treasures[MAX_TREASURES];
int hunt_count = 0;
int treasure_count = 0;
pid_t monitor_pid = -1; // PID of the monitor process

// Function to start the monitor process
void start_monitor() {
    if (monitor_pid == -1) {
        monitor_pid = fork(); // Create a new process
        if (monitor_pid == 0) {
            monitor_process(); // Child process runs the monitor
            exit(0);
        } else {
            printf("Monitor started.\n");
        }
    } else {
        printf("Monitor is already running.\n");
    }
}

// Function to list hunts by sending SIGUSR1 to the monitor
void list_hunts() {
    if (monitor_pid != -1) {
        kill(monitor_pid, SIGUSR1);
    } else {
        printf("Error: Monitor is not running.\n");
    }
}

// Function to list treasures by sending SIGUSR2 to the monitor
void list_treasures() {
    if (monitor_pid != -1) {
        kill(monitor_pid, SIGUSR2);
    } else {
        printf("Error: Monitor is not running.\n");
    }
}

// Function to view a specific treasure by sending SIGUSR2 to the monitor
void view_treasure() {
    if (monitor_pid != -1) {
        kill(monitor_pid, SIGUSR2);
    } else {
        printf("Error: Monitor is not running.\n");
    }
}

// Function to stop the monitor process by sending SIGTERM
void stop_monitor() {
    if (monitor_pid != -1) {
        kill(monitor_pid, SIGTERM);
        waitpid(monitor_pid, NULL, 0); // Wait for the monitor to terminate
        monitor_pid = -1;
        printf("Monitor stopped.\n");
    } else {
        printf("Error: Monitor is not running.\n");
    }
}

// Signal handler to print hunts or treasures based on the signal received
/*void handle_signal(int sig) {
    //TODO
}

// Monitor process that waits for signals and responds accordingly
void monitor_process() {
    //TODO
}*/

int main() {
    return 0;
}
