#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <dirent.h>


 // Define maximum number of hunts and treasures
#define MAX_HUNTS 10
#define MAX_TREASURES 100
#define ID_MAX_LENGTH 16
#define USER_NAME_MAX_LENGTH 32
#define CLUE_TEXT_MAX_LENGTH 800
#define TREASURE_BLOCK_LENGTH (sizeof(treasure_data))

typedef struct {
    char treasure_id[ID_MAX_LENGTH];
    char user_name[USER_NAME_MAX_LENGTH];
    char clue_text[CLUE_TEXT_MAX_LENGTH];
    float latitude;
    float longitude;
    int value;
} treasure_data;


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

bool check_if_hunt_exists(const char *hunt_id) {
    struct stat hunt_stat;

    // Check if given hunt exists
    if((stat(hunt_id, &hunt_stat) == 0) && S_ISDIR(hunt_stat.st_mode)) {
        return true;
    }
    else {
        return false;
    }
}

bool find_treasure_in_hunt(const char *hunt_id, const char *treasure_id, treasure_data *treasure) {
    int no_of_treasures = -1;
    char buff[128];
    sprintf(buff, "%s/treasures", hunt_id);
    struct stat file_stat;
    int rc = stat(buff, &file_stat);

    if(rc < 0) {
        // no treasures file => the treasure does not exist
        return false;
    }

    no_of_treasures = file_stat.st_size / TREASURE_BLOCK_LENGTH;
    int file_desc = open(buff, O_RDONLY);
    if(file_desc < 0) {
        printf("ERROR: Cannot open treasure file for: %s\n", hunt_id);
        return false;
    }

    for(int i = 0; i < no_of_treasures; i++) {
        ssize_t bytes_read = read(file_desc, treasure, TREASURE_BLOCK_LENGTH);

        if(bytes_read != TREASURE_BLOCK_LENGTH) {
            close(file_desc);
            printf("ERROR: Read %ld out of %ld bytes!\n", bytes_read, TREASURE_BLOCK_LENGTH);
            return false;
        }
        if(strcmp(treasure_id, treasure->treasure_id) == 0) {
            return true;
        }
    }

    return false;
}

void list_hunts()
{
    DIR *dir;
    struct dirent *read_from_dir;
    const char *hunt_entry_name= "logged_hunt-";
    int length_entry_name = strlen(hunt_entry_name);

    // Open the directory
    dir = opendir(".");

    if(dir) {
        // Read from directory
        while((read_from_dir = readdir(dir)) != NULL) {
            if(strncmp(hunt_entry_name, read_from_dir->d_name, length_entry_name) == 0) {
                printf("Hunt ID: %s\n", read_from_dir->d_name + length_entry_name);
            }
        }

        // Close the directory
        closedir(dir);
    }
}

void list_treasures(const char* hunt_id)
{
    if(!check_if_hunt_exists(hunt_id)) {
        printf("ERROR: Hunt does NOT exist: %s\n", hunt_id);
        return;
    }

    printf("Hunt name: %s\n", hunt_id);

    struct stat stat_buff;
    char buff[128];
    sprintf(buff, "%s/treasures", hunt_id);
    int rc = stat(buff, &stat_buff);

    if(rc != 0) {
        printf("No treasures for this hunt.\n");
        return;
    }

    int no_of_treasures = stat_buff.st_size / TREASURE_BLOCK_LENGTH;
    printf("Number of treasures: %d\n", no_of_treasures);

    treasure_data treasure;
    int file_desc = open(buff, O_RDONLY);
    for(int i = 0; i < no_of_treasures; i++) {
        read(file_desc, &treasure, TREASURE_BLOCK_LENGTH);
        printf("Treasure ID: %s\n", treasure.treasure_id);
    }
}

void view_treasure(const char* hunt_id, const char* treasure_id)
{
    if(!check_if_hunt_exists(hunt_id)) {
        printf("ERROR: Hunt does NOT exist: %s\n", hunt_id);
        return;
    }

    treasure_data treasure;
    bool rc = find_treasure_in_hunt(hunt_id, treasure_id, &treasure);
    if (!rc) {
        printf("ERROR: Treasure %s does not exist in hunt %s\n", treasure_id, hunt_id);
        return;
    }

    printf("Hunt %s, treasure %s:\n", hunt_id, treasure.treasure_id);
    printf("       User: %s\n", treasure.user_name);
    printf("      Value: %d\n", treasure.value);
    printf("   Latitude: %f\n", treasure.latitude);
    printf("  Longitude: %f\n", treasure.longitude);
    printf("  Clue text: %s\n", treasure.clue_text);

}


//---------------------------------
// Monitor (child) code

void perform_operation()
{
    char buff[256];
    char arg1[128];
    char arg2[128];
    // step 1 - read data from file: operation + arguments => into buff, using open, read, close
    int file_desc = open(communication_file, O_RDONLY);
    if(file_desc < 0) {
        printf("ERROR: Cannot open file %s!\n", communication_file);
        return;
    }

    ssize_t bytes_read = read(file_desc, buff, (sizeof(buff)-1)); // space for null terminator
    // Close file
    close(file_desc);

    if(bytes_read <= 0) {
        printf("ERROR: Failed to read from file %s!\n", communication_file);
        return;
    }

    // Add the null terminator
    buff[bytes_read] = '\0';

    // step 2 - get the 1st character from buff => operation discriminant
    char command = buff[0];

    // step 3 - perform the necessary operation, using a switch
    switch (command)
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
        // step1: put values into a buffer
        char buff[256];
        if((strlen(arg1) == 0) && strlen(arg2) == 0) {
            sprintf(buff, "%c", cmd_code);
        }
        else if(strlen(arg2) == 0) {
            sprintf(buff, "%c%s\n", cmd_code, arg1);
        }
        else {
            sprintf(buff, "%c%s %s\n", cmd_code, arg1, arg2);
        }

        // step2: write buffer to file - using open(), write(), close()
        int file_desc = open(communication_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
        if(file_desc < 0) {
            printf("ERROR: Cannot open file %s!\n", communication_file);
            return;
        }

        ssize_t bytes_written = write(file_desc, buff, strlen(buff));
        if(bytes_written <= 0) {
            printf("ERROR: Failed to write into file %s!\n", communication_file);
            return;
        }

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
