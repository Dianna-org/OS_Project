#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/*// Define maximum number of hunts and treasures
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
*/

// Global variables
pid_t hub_pid = -1;
pid_t monitor_pid = -1;
int monitor_is_busy = 0;
int stop_monitor_was_sent = 0;
const char* communication_file = "hub_communication.tmp";


// 


//---------------------------------
// Treasure functionalities

// Make list_hunts, list_treasures and view_treasure to work

void list_hunts()
{
    // TODO implement this
}

void list_treasures(const char* hunt_id)
{
    // TODO implement this
}

void view_treasure(const char* hunt_id, const char* treasure_id)
{
    // TODO implement this
}


//---------------------------------
// Monitor (child) code

void perform_operation()
{
    char buff[256];
    char arg1[128];
    char arg2[128];
    // step 1 - read data from file: operation + arguments => into buff, using open, read, close
    // step 2 - get the 1st character from buff => operation discriminant

    // step 3 - perform the necessary operation, using a switch
    switch (buff[0])
    {
        case 'H':
            // list hunts
            list_hunts();
            break;

        case 'T':
            // list treasures
            sscanf(buff + 1, "%s", arg1);
            list_treasures(arg1);
            break;

        case 'V':
            // view specific treasure
            sscanf(buff + 1, "%s %s", arg1, arg2);
            view_treasure(arg1, arg2);
            break;
    }
}

void monitor_signal_handler(int signal)
{
    if (signal == SIGUSR1) {
        perform_operation();
        // send signal to HUB, that the monitor has finished its command
        kill(hub_pid, SIGUSR1);
    }
    else if (signal == SIGUSR2) {
        // On SIGUSR2 do a delay and then exit
        usleep(10123123);
        exit(0);
    }
}

void run_monitor()
{
    // set up signal handler
    struct sigaction monitor_action;
    monitor_action.sa_handler = monitor_signal_handler;
    sigaction(SIGUSR1, &monitor_action, NULL);
    sigaction(SIGUSR2, &monitor_action, NULL);
    // start waiting for actions from parent
    while (1) {
        pause(); // Wait for signals
    }
}


//---------------------------------
// Hub (parent) code

// Function to start the monitor process
void start_monitor() {
    if (monitor_pid == -1) {
        monitor_pid = fork(); // Create a new process
        if (monitor_pid == 0) {
            // Child process: Monitor
            run_monitor();
        } else if (monitor_pid == -1) {
            printf("ERROR: Monitor failed to start\n");
        } else {
            printf("Monitor started with PID %d\n", monitor_pid);
        }
    } else {
        printf("Monitor is already running.\n");
    }
}


// Function to stop the monitor process
void stop_monitor() {
    if (monitor_pid == -1) {
        printf("ERROR: Monitor is not running.\n");
    } else if (stop_monitor_was_sent) {
        printf("ERROR: Stop signal was sent to monitor.\n");
    } else {
        stop_monitor_was_sent = 1;
        kill(monitor_pid, SIGUSR2);
        printf("Sent stop signal to monitor\n");
    }

    // if (monitor_pid != -1) {
    //     kill(monitor_pid, SIGTERM); // Send signal to terminate monitor
    //     // usleep(100000); // Delay to simulate monitor's exit delay
    //     waitpid(monitor_pid, NULL, 0); // Wait for the monitor to terminate
    //     monitor_pid = -1;
    //     printf("Monitor stopped.\n");
    // } else {
    //     printf("ERROR: Monitor is not running.\n");
    // }
}

// Signal handler function
void handle_signal(int sig) {
    if (sig == SIGCHLD) {
        wait(NULL); // Clean up child process
        stop_monitor_was_sent = 0;
        monitor_pid = -1;
    } else if (sig == SIGUSR1) {
        monitor_is_busy = 0; // false
    }
}

void send_command_to_monitor(const char* command, char cmd_code, const char* arg1, const char* arg2)
{
    if (monitor_pid == -1) {
        printf("ERROR: Monitor is not running.\n");
    } else if (stop_monitor_was_sent) {
        printf("ERROR: Stop signal was sent to monitor.\n");
    } else {
        // TODO write file contents:
        // step1: put values into a buffer
        char buff[256];
        sprintf(buff, "%c%s %s", cmd_code, arg1, arg2);
        // step2: write buffer to file - using open(), write(), close()

        // signal to monitor that it should run the operation
        monitor_is_busy = 1; // true
        kill(monitor_pid, SIGUSR1);
        while (monitor_is_busy) {
            pause();
        }
        printf("Monitor has finished executing command: %s\n", command);
    }
}

int can_exit_hub() {
    if (monitor_pid != -1) {
        printf("ERROR: Monitor is still running.\n");
        return 0;
    } else if (stop_monitor_was_sent) {
        printf("ERROR: Waiting for monitor to end.\n");
        return 0;
    } else {
        printf("HUB can exit.\n");
        return 1;
    }
}

int main() {
    char command[50];
    char arg1[128];
    char arg2[128];

    // get hub (parent) PID
    hub_pid = getpid();

    // Set up signal handler for SIGCHLD
    struct sigaction process_action;
    process_action.sa_handler = handle_signal;
    sigaction(SIGCHLD, &process_action, NULL);
    sigaction(SIGUSR1, &process_action, NULL);

    int can_exit = 0; // false
    while (!can_exit) {
        // print monitor status before each command
        if (monitor_pid == -1) {
            printf("Monitor is not running.\n");
        } else if (stop_monitor_was_sent) {
            printf("Stop signal was sent to monitor.\n");
        } else {
            printf("Monitor is running, PID=%d\n", monitor_pid);
        }

        arg1[0] = 0;
        arg2[0] = 0;
        printf("Enter command: ");
        scanf("%s", command);

        if (strcmp(command, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(command, "list_hunts") == 0) {
            send_command_to_monitor(command, 'H', arg1, arg2); // list_hunts();
        } else if (strcmp(command, "list_treasures") == 0) {
            scanf("%s", arg1);
            send_command_to_monitor(command, 'T', arg1, arg2); // list_treasures();
        } else if (strcmp(command, "view_treasure") == 0) {
            scanf("%s %s", arg1, arg2);
            send_command_to_monitor(command, 'V', arg1, arg2); // view_treasure();
        } else if (strcmp(command, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(command, "exit") == 0) {
            can_exit = can_exit_hub();
        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}
