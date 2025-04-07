/*
 * treasure_data hunt manager. 
 *
 * Format of a treasure file:
 *    16 bytes = treasure ID, as string terminated with 0
 *    32 bytes = TODO
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>


#define ID_MAX_LENGTH 16
#define USER_NAME_MAX_LENGTH 32
#define CLUE_TEXT_MAX_LENGTH 800
#define TREASURE_BLOCK_LENGTH (ID_MAX_LENGTH + USER_NAME_MAX_LENGTH + CLUE_TEXT_MAX_LENGTH + 2*sizeof(float) + sizeof(int))

const float MIN_LATITUDE = -90.0;
const float MAX_LATITUDE = 90.0;
const float MIN_LONGITUDE = -180.0;
const float MAX_LONGITUDE = 180.0;
const int MIN_VALUE = 1;
const int MAX_VALUE = 1000;


enum response_codes {
    OK_RESPONSE = 0,
    ERR_STRING_EMPTY,
    ERR_STRING_TOO_LONG,
    ERR_INVALID_CHARACTER,
    ERR_HUNT_NOT_FOUND,
    ERR_TREASURE_NOT_FOUND,
    ERR_EOF,
    ERR_READ_FILE,
    ERR_WRITE_FILE,
};


typedef struct {
    char data_block[TREASURE_BLOCK_LENGTH];
    const char *treasure_id;
    const char *user_name;
    const char *clue_text;
    float latitude;
    float longitude;
    int value;
} treasure_data;


//-----------------------
bool check_if_username_is_correct(const char *user_name) {
    const char *current_char = user_name;
    while(*current_char != '\0'){
        if((isalnum(*current_char) == 0) && *current_char != '_') {
            return false;
        }

        current_char++;
    }

    return true;
}



// Validate that the given string has length between 1 and ID_MAX_LENGTH,
// and contains only letters, digits and/or '_'
int validate_id(const char *id) {
    const char *current_char = id;
    while(*current_char != '\0') {
        if(isdigit(*current_char) == 0) {
            return ERR_INVALID_CHARACTER;
        }

        current_char++;
    }

    int length = strlen(id);
    if((length < 1 || length > ID_MAX_LENGTH)) {
        return ERR_INVALID_CHARACTER;
    }

    return OK_RESPONSE;
}


// Validate that the given string has length between 1 and USER_NAME_MAX_LENGTH,
// and contains only letters, digits and/or '_'
int validate_user_name(const char *user_name) {
    //TODO
    int length = strlen(user_name);
    bool check_username = check_if_username_is_correct(user_name);
    if((check_username == false) || (length < 1 || length > USER_NAME_MAX_LENGTH)) {
        return ERR_INVALID_CHARACTER;
    }

    return OK_RESPONSE;
}



// Allocate a treasure, using malloc(), and return it
// NICE TO HAVE: fill in pointers for treasure ID, user name and clue text
treasure_data *alloc_treasure(const char *hunt_id) {

    treasure_data *treasure = (treasure_data *)malloc(sizeof(treasure_data));
    if (treasure == NULL) {
        return NULL;
    }

    treasure->treasure_id = strdup(hunt_id);
    treasure->user_name = strdup("UnknownUser");
    treasure->clue_text = strdup("No clue available.");
    treasure->latitude = 0.0;
    treasure->longitude = 0.0;
    treasure->value = 0;

    return treasure;
}

// De-allocate treasure, if NOT already deallocated.
void free_treasure(treasure_data **treasure) {
    if (*treasure != NULL) {
        free(*treasure);
        *treasure = NULL;
    }
}


// Read the next treasure block, parse it and put it into the given structure.
// NOTE: read the block, then extract the numeric values from the data block
int read_treasure(FILE *input_file, treasure_data *treasure) {
    if (input_file == NULL || treasure == NULL) {
        return ERR_READ_FILE;
    }

    if (fread(treasure->data_block, TREASURE_BLOCK_LENGTH, 1, input_file) != 1) {
        return ERR_EOF;
    }

    // Parse the block into the treasure_data structure
    treasure->treasure_id = strndup(treasure->data_block, ID_MAX_LENGTH);
    treasure->user_name = strndup(treasure->data_block + ID_MAX_LENGTH, USER_NAME_MAX_LENGTH);
    treasure->clue_text = strndup(treasure->data_block + ID_MAX_LENGTH + USER_NAME_MAX_LENGTH, CLUE_TEXT_MAX_LENGTH);

    // Use pointer arithmetic and type casting to extract numeric values
    treasure->latitude = *(float *)(treasure->data_block + ID_MAX_LENGTH + USER_NAME_MAX_LENGTH + CLUE_TEXT_MAX_LENGTH);
    treasure->longitude = *(float *)(treasure->data_block + ID_MAX_LENGTH + USER_NAME_MAX_LENGTH + CLUE_TEXT_MAX_LENGTH + sizeof(float));
    treasure->value = *(int *)(treasure->data_block + ID_MAX_LENGTH + USER_NAME_MAX_LENGTH + CLUE_TEXT_MAX_LENGTH + 2 * sizeof(float));

    return OK_RESPONSE;
}



// Write a treasure block into a file.
// NOTE: put the numeric values into the data block, then write the block to file
int write_treasure(FILE *output_file, const treasure_data *treasure) {
    if (output_file == NULL || treasure == NULL) {
        return ERR_WRITE_FILE;
    }

    // Prepare the data block for writing
    treasure_data *treasure = (treasure_data *)calloc(1, sizeof(treasure_data));
    strncpy(treasure->data_block, treasure->treasure_id, ID_MAX_LENGTH);
    strncpy(treasure->data_block + ID_MAX_LENGTH, treasure->user_name, USER_NAME_MAX_LENGTH);
    strncpy(treasure->data_block + ID_MAX_LENGTH + USER_NAME_MAX_LENGTH, treasure->clue_text, CLUE_TEXT_MAX_LENGTH);

    // Use pointer arithmetic to write numeric values directly
    *(float *)(treasure->data_block + ID_MAX_LENGTH + USER_NAME_MAX_LENGTH + CLUE_TEXT_MAX_LENGTH) = treasure->latitude;
    *(float *)(treasure->data_block + ID_MAX_LENGTH + USER_NAME_MAX_LENGTH + CLUE_TEXT_MAX_LENGTH + sizeof(float)) = treasure->longitude;
    *(int *)(treasure->data_block + ID_MAX_LENGTH + USER_NAME_MAX_LENGTH + CLUE_TEXT_MAX_LENGTH + 2 * sizeof(float)) = treasure->value;

    // Write the block to the file
    if (fwrite(treasure->data_block, TREASURE_BLOCK_LENGTH, 1, output_file) != 1) {
        return ERR_WRITE_FILE; // Write error
    }

    return OK_RESPONSE;
}

//-----------------------


int add_hunt(const char *hunt_id) {
    //TODO
    return OK_RESPONSE;
}


int list_hunt_treasures(const char *hunt_id) {
    //TODO
    return OK_RESPONSE;
}


int remove_hunt(const char *hunt_id) {
    char directory_path[256];
    snprintf(directory_path, sizeof(directory_path), "./%s", hunt_id);

    // Remove directory and files (use system calls like `unlink` and `rmdir`)
    // TODO: Traverse and delete files if necessary
    if (rmdir(directory_path) != 0) {
        return ERR_HUNT_NOT_FOUND; // Error removing directory
    }

    return OK_RESPONSE;
}



int add_hunt(const char *hunt_id) {
    /*char directory_path[256];
    snprintf(directory_path, sizeof(directory_path), "./%s", hunt_id);

    // Create hunt directory
    if (mkdir(directory_path, 0755) != 0) {
        return ERR_HUNT_NOT_FOUND; // Error creating directory
    }

    // Create log file
    char log_file_path[256];
    snprintf(log_file_path, sizeof(log_file_path), "%s/logged_hunt", directory_path);
    FILE *log_file = fopen(log_file_path, "w");
    if (log_file == NULL) {
        return ERR_WRITE_FILE; // Error creating log file
    }
    fclose(log_file);

    // Create symbolic link
    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    if (symlink(log_file_path, symlink_name) != 0) {
        return ERR_WRITE_FILE; // Error creating symbolic link
    }*/

    return OK_RESPONSE;
}



int view_treasure(const char *hunt_id, const char *treasure_id) {
    //TODO
    return OK_RESPONSE;
}


int remove_treasure(const char *hunt_id, const char *treasure_id) {
    //TODO
    return OK_RESPONSE;
}



//-----------------------

int main(int argc, char **argv) {
    return OK_RESPONSE;
}
