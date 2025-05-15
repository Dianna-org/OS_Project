#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#define ID_MAX_LENGTH 16
#define USER_NAME_MAX_LENGTH 32
#define CLUE_TEXT_MAX_LENGTH 800
#define TREASURE_BLOCK_LENGTH (sizeof(treasure_data))
#define MAX_USERS 50 // maximum number of distinct users

typedef struct {
    char treasure_id[ID_MAX_LENGTH];
    char user_name[USER_NAME_MAX_LENGTH];
    char clue_text[CLUE_TEXT_MAX_LENGTH];
    float latitude;
    float longitude;
    int value;
} treasure_data;

typedef struct {
    char user[USER_NAME_MAX_LENGTH];
    int score;
} user_score;

//-----------------------------------------------------------------------------------------------------

int main(int argc, char **argv) {
    // Check if only one argument(hunt_id) was provided
    if(argc < 2) {
        printf("ERROR: Wrong number of arguments. Usage : %s <hunt_id>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Read and store the hunt_id
    char *hunt_id = argv[1];

    // Build the path to the treasure file for the given hunt
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/treasures", hunt_id);

    // Open the treasure file for reading
    int file_desc = open(file_path, O_RDONLY);
    if(file_desc < 0) {
        printf("ERROR: Cannot find hunt: %s\n", hunt_id);
        exit(EXIT_FAILURE);
    }

    // Obtain file status information
    struct stat stat_buff;
    if(stat(file_path, &stat_buff) < 0) {
        printf("ERROR: Cannot obtain information about hunt: %s\n", hunt_id);
        close(file_desc);
        exit(EXIT_FAILURE);
    }

    // Compute the number of treasures
    int no_of_treasures = stat_buff.st_size / TREASURE_BLOCK_LENGTH;
    if(no_of_treasures <= 0) {
        printf("No treasures found for hunt: %s\n", hunt_id);
        close(file_desc);
        exit(EXIT_FAILURE);
    }

    // Store the scores
    user_score score_array[MAX_USERS];
    int no_of_users = 0;

    // Initialize the array
    for(int i = 0; i < MAX_USERS; i++) {
        score_array[i].user[0] = '\0';
        score_array[i].score = 0;
    }

    // Buffer to store each treasure
    treasure_data treasure;

    // Go through all treasures from the file
    for(int i = 0; i < no_of_treasures; i++) {
        ssize_t bytes_read = read(file_desc, &treasure, TREASURE_BLOCK_LENGTH);
        if(bytes_read != TREASURE_BLOCK_LENGTH) {
            printf("ERROR: Read %ld out of %ld bytes!\n", bytes_read, TREASURE_BLOCK_LENGTH);
            close(file_desc);
            exit(EXIT_FAILURE);
        }

        // Check if the user associated with the treasure already exists in our scores
        bool found = false;
        for(int j = 0; j < no_of_users; j++) {
            if(strcmp(score_array[j].user, treasure.user_name) == 0) {
                score_array[j].score += treasure.value;
                found = true;
                break;
            }
        }

        // Check if the user was found
        // => if not, and we have not reached MAX_USERS => add a new record
        if(!found && no_of_users < MAX_USERS) {
            strncpy(score_array[no_of_users].user, treasure.user_name, USER_NAME_MAX_LENGTH-1);
            score_array[no_of_users].user[USER_NAME_MAX_LENGTH-1] = '\0';
            score_array[no_of_users].score = treasure.value;
            no_of_users++;
        }
    }

    // Close the treasure file
    close(file_desc);

    // Print the result
    printf("Scores for hunt %s:\n", hunt_id);
    for(int i = 0; i < no_of_users; i++) {
        printf("User: %s, Score: %d\n", score_array[i].user, score_array[i].score);
    }

    return EXIT_SUCCESS;
}