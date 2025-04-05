/*
 * Treasure hunt manager. 
 *
 * Format of a treasure file:
 *    
 */


#include <stdio.h>
#include <stdlib.h>


#define ID_MAX_LENGTH 16
#define USER_NAME_MAX_LENGTH 32
#define CLUE_TEXT_MAX_LENGTH 800
#define TREASURE_BLOCK_LENGTH (ID_MAX_LENGTH + USER_NAME_MAX_LENGTH + CLUE_TEXT_MAX_LENGTH + 2*sizeof(float) + sizeof(int))
#define MIN_LATITUDE -90.0
#define MAX_LATITUDE 90.0
#define MIN_LONGITUDE -180.0
#define MAX_LONGITUDE 180.0
#define MIN_VALUE 1
#define MAX_VALUE 1000
#define OK_RESPONSE 0
#define ERR_STRING_TOO_LONG 1





typedef struct {
    char data_block[TREASURE_BLOCK_LENGTH];
    const char hunt_id[ID_MAX_LENGTH];
    const char *treasure_id;
    const char *user_name;
    float latitude;
    float longitude;
    const char *clue_text;
    int value;
} Treasure;


int validate_id(const char *id) {
    //TODO 
    return OK_RESPONSE;
}


int validate_user_name(const char *user_name) {
    //TODO
    return OK_RESPONSE;
}


Treasure *alloc_treasure(const char *hunt_id) {
    //TODO
    return OK_RESPONSE;
}


void free_treasure(Treasure *treasure) {
    //TODO
    return OK_RESPONSE;
}


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


int main(int argc, char **argv) {
    return OK_RESPONSE;
}
