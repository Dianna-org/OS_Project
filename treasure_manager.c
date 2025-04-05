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

// Validate that the given string has length between 1 and ID_MAX_LENGTH,
// and contains only letters, digits and/or '_'
int validate_id(const char *id) {
    //TODO 
    return OK_RESPONSE;
}


// Validate that the given string has length between 1 and USER_NAME_MAX_LENGTH,
// and contains only letters, digits and/or '_'
int validate_user_name(const char *user_name) {
    //TODO
    return OK_RESPONSE;
}


// Allocate a treasure, using malloc(), and return it
// NICE TO HAVE: fill in pointers for treasure ID, user name and clue text
treasure_data *alloc_treasure(const char *hunt_id) {
    //TODO
    return OK_RESPONSE;
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
    //TODO
    return OK_RESPONSE;

}


// Write a treasure block into a file.
// NOTE: put the numeric values into the data block, then write the block to file
int write_treasure(FILE *output_file, const treasure_data *treasure) {
    //TODO
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
    //TODO
    return OK_RESPONSE;
}


int add_treasure(const char *hunt_id, const char *treasure_id) {
    //TODO
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
