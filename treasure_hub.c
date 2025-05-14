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


typedef struct {
    char operation;
    char hunt_id[ID_MAX_LENGTH];
    char treasure_id[ID_MAX_LENGTH];
} monitor_request;


// Global variables
pid_t hub_pid = -1;
pid_t monitor_pid = -1;
int stop_monitor_was_sent = 0;

// Pipe descriptors
int pipe_monitor_to_hub[2];
int pipe_hub_to_monitor[2];
FILE *monitor_to_hub;

// Marker for the end of a command output from the monitor
const char* finished_monitor_output = "finished_monitor_output\n";

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
    monitor_request request;

    // step 1 - read data from pipe
    int expected_size = sizeof(monitor_request);
    if (read(pipe_hub_to_monitor[0], &request, expected_size) != expected_size) {
        printf("ERROR: Failed to read from pipe!\n");
        return;
    }

    // step 2 - perform the necessary operation, using a switch
    switch (request.operation)
    {
        case 'H':
            // list hunts
            list_hunts();
            break;

        case 'T':
            // list treasures
            list_treasures(request.hunt_id);
            break;

        case 'V':
            // view specific treasure
            view_treasure(request.hunt_id, request.treasure_id);
            break;
    }
}

void monitor_signal_handler(int signal)
{
    if (signal == SIGUSR1) {
        perform_operation();
        // send end of response message
        printf("%s", finished_monitor_output);
    }
    else if (signal == SIGUSR2) {
        // On SIGUSR2 do a delay and then exit
        usleep(10123123);
        // Close the pipes
        close(pipe_hub_to_monitor[0]);
        close(pipe_monitor_to_hub[1]);
        exit(0);
    }
}

void run_monitor()
{
    // We're in the hub => close the pipe ends which we do not use
    close(pipe_hub_to_monitor[1]);
    close(pipe_monitor_to_hub[0]);

    // Redirect standard output to the pipe
    dup2(pipe_monitor_to_hub[1], 1);

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
        // Create the pipes
        if (pipe(pipe_hub_to_monitor) < 0 || pipe(pipe_monitor_to_hub) < 0) {
            perror("Pipe creation failed");
            return;
        }

        // Create a new process
        monitor_pid = fork(); 
        if (monitor_pid == 0) {
            // Child process: Monitor
            run_monitor();
        } else if (monitor_pid == -1) {
            printf("ERROR: Monitor failed to start\n");
        } else {
            // We're in the hub => close the pipe ends which we do not use
            close(pipe_hub_to_monitor[0]);
            close(pipe_monitor_to_hub[1]);
            // Wrap the monitor-to-hub read descriptor into a FILE
            monitor_to_hub = fdopen(pipe_monitor_to_hub[0], "rt");
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
        // Close the pipes
        close(pipe_hub_to_monitor[1]);
        fclose(monitor_to_hub);
    }
}

// Signal handler function
void handle_signal(int sig) {
    if (sig == SIGCHLD) {
        wait(NULL); // Clean up child process
        stop_monitor_was_sent = 0;
        monitor_pid = -1;
    }
}

void send_command_to_monitor(monitor_request *request)
{
    if (monitor_pid == -1) {
        printf("ERROR: Monitor is not running.\n");
    } else if (stop_monitor_was_sent) {
        printf("ERROR: Stop signal was sent to monitor.\n");
    } else {
        // step 1 - write request data to pipe
        int expected_size = sizeof(monitor_request);
        if (write(pipe_hub_to_monitor[1], request, expected_size) != expected_size) {
            printf("ERROR: Failed to write to pipe!\n");
            return;
        }

        // step 2 - signal to monitor that it should run the operation
        kill(monitor_pid, SIGUSR1);

        // step 3 - read output from monitor, until we get the end messsage
        char buff[256];
        while (fscanf(monitor_to_hub, "%255[^\n]\n", buff) != EOF) {
            if (strcmp(buff, finished_monitor_output) == 0) {
                break;
            }
            printf("%s", buff);
        }
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
    monitor_request request;

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

        request.hunt_id[0] = 0;
        request.treasure_id[0] = 0;
        printf("Enter command: ");
        scanf("%s", command);

        if (strcmp(command, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(command, "list_hunts") == 0) {
            request.operation = 'H';
            send_command_to_monitor(&request); // list_hunts();
        } else if (strcmp(command, "list_treasures") == 0) {
            request.operation = 'T';
            scanf("%s", request.hunt_id);
            send_command_to_monitor(&request); // list_treasures();
        } else if (strcmp(command, "view_treasure") == 0) {
            request.operation = 'V';
            scanf("%s %s", request.hunt_id, request.treasure_id);
            send_command_to_monitor(&request); // view_treasure();
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
