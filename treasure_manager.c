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
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>


#define ID_MAX_LENGTH 16
#define USER_NAME_MAX_LENGTH 32
#define CLUE_TEXT_MAX_LENGTH 800
#define TREASURE_BLOCK_LENGTH (sizeof(treasure_data))

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
    ERR_HUNT_ALREADY_EXISTS,
    ERR_TREASURE_NOT_FOUND,
    ERR_EOF,
    ERR_READ_FILE,
    ERR_WRITE_FILE,
};


typedef struct {
    char treasure_id[ID_MAX_LENGTH];
    char user_name[USER_NAME_MAX_LENGTH];
    char clue_text[CLUE_TEXT_MAX_LENGTH];
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
        if(isalnum(*current_char) == 0) {
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


bool check_if_hunt_exists(const char *hunt_id) {
    struct stat stat_buff;
    int result = stat(hunt_id, &stat_buff);
    return result == 0;
}
//-----------------------


int add_hunt(const char *hunt_id) {
    if(check_if_hunt_exists(hunt_id)) {
        printf("ERROR: Hunt already exists: %s\n", hunt_id);
        return ERR_HUNT_ALREADY_EXISTS;
    }

    // Create a hunt folder
    int rc = mkdir(hunt_id, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);
    if(rc != 0) {
        printf("ERROR: Cannot create hunt folder for: %s\n", hunt_id);
        return ERR_WRITE_FILE;
    }

    // Create hunt_log file
    char buff[128];
    sprintf(buff, "%s/logged_hunt", hunt_id);
    int f_desc = creat(buff, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

    if(rc < 0) {
        printf("ERROR: Cannot write hunt log for: %s\n", hunt_id);
        return ERR_WRITE_FILE;
    }

    rc = close(f_desc);
    if(rc < 0) {
        printf("ERROR: Cannot close hunt folder for: %s\n", hunt_id);
        return ERR_WRITE_FILE;
    }

    // Creating a symbolic link
    char buff_2[128];
    sprintf(buff_2, "logged_hunt-%s", hunt_id);
    rc = symlink(buff, buff_2);
    if(rc < 0) {
        printf("ERROR: Cannot create symbolic link for: %s\n", hunt_id);
        return ERR_WRITE_FILE;
    }
    return OK_RESPONSE;
}


int list_hunt_treasures(const char *hunt_id) {
    if(!check_if_hunt_exists(hunt_id)) {
        printf("ERROR: Hunt does NOT exist: %s\n", hunt_id);
        return ERR_HUNT_NOT_FOUND;
    }

    printf("Hunt name: %s\n", hunt_id);

    struct stat stat_buff;
    char buff[128];
    sprintf(buff, "%s/treasures", hunt_id);
    int rc = stat(buff, &stat_buff);

    if(rc != 0) {
        printf("No treasures for this hunt.\n");
        return OK_RESPONSE;
    }

    int no_of_treasures = stat_buff.st_size/TREASURE_BLOCK_LENGTH;
    printf("Treasure size (in bytes): %ld\n", stat_buff.st_size);
    printf("Number of treasures: %d\n", no_of_treasures);

    struct tm *timeinfo = localtime(&stat_buff.st_mtime);
    // Format the date as "YYYY-MM-DD HH:MM:SS"
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("Last modification date: %s\n", buff);

    sprintf(buff, "%s/treasures", hunt_id);
    int file_desc = open(buff, O_RDONLY);
    if(file_desc < 0) {
        printf("ERROR: Cannot open treasure file for: %s\n", hunt_id);
        return ERR_READ_FILE;
    }

    for(int i = 0; i < no_of_treasures; i++) {
        treasure_data treasure;
        ssize_t bytes_read = read(file_desc, &treasure, TREASURE_BLOCK_LENGTH);

        if(bytes_read != TREASURE_BLOCK_LENGTH) {
            close(file_desc);
            printf("ERROR: Read %ld out of %ld bytes!\n", bytes_read, TREASURE_BLOCK_LENGTH);
            return ERR_READ_FILE;
        }

        printf("Treasure %s: user: %s, value = %d, at %f, %f\n", 
            treasure.treasure_id, treasure.user_name, treasure.value, treasure.longitude, treasure.latitude);
    }
    
    rc = close(file_desc);
    if(rc < 0) {
        printf("ERROR: Failed to close file!\n");
        return ERR_WRITE_FILE;
    }
    return OK_RESPONSE;
}


int remove_hunt(const char *hunt_id) {
    if(!check_if_hunt_exists(hunt_id)) {
        printf("ERROR: Hunt does NOT exist: %s\n", hunt_id);
        return ERR_HUNT_NOT_FOUND;
    }

    char buff[128];
    sprintf(buff, "%s/treasures", hunt_id);
    unlink(buff);

    sprintf(buff, "logged_hunt-%s", hunt_id);
    unlink(buff);

    sprintf(buff, "%s/logged_hunt", hunt_id);
    unlink(buff);

    // Remove directory and files (use system calls like `unlink` and `rmdir`)
    // TODO: Traverse and delete files if necessary
    if (rmdir(hunt_id) != 0) {
        printf("ERROR: Failed to remove hunt folder!\n");
        return ERR_HUNT_NOT_FOUND; // Error removing directory
    }

    return OK_RESPONSE;
}



int add_treasure(const char *hunt_id, const char *treasure_id, const char *user_name) {
    //TODO pass treasure_data as arguments
    // TODO BUG - can add same treasure ID twice!
    treasure_data treasure;

    int rc = validate_user_name(user_name);
    if(rc != OK_RESPONSE) {
        printf("Invalid Username!\n");
        return rc;
    }

    rc = validate_id(treasure_id);
    if(rc != OK_RESPONSE) {
        printf("Invalid treasure ID!\n");
        return rc;
    }

    strcpy(treasure.user_name, user_name);
    strcpy(treasure.treasure_id, treasure_id);
    strcpy(treasure.clue_text, "clue_text"); //TODO
    treasure.latitude = 12.55;
    treasure.longitude = 8.17;
    treasure.value = 75;

    char buff[128];
    sprintf(buff, "%s/treasures", hunt_id);

    int file_desc = open(buff, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if(file_desc < 0) {
        printf("ERROR: Cannot open treasure file for: %s\n", hunt_id);
        return ERR_WRITE_FILE;
    }

    ssize_t written = write(file_desc, &treasure, TREASURE_BLOCK_LENGTH);
    rc = close(file_desc);

    if(written != TREASURE_BLOCK_LENGTH) {
        printf("ERROR: Written %ld out of %ld bytes!\n", written, TREASURE_BLOCK_LENGTH);
        return ERR_WRITE_FILE;
    }

    if(rc < 0) {
        printf("ERROR: Failed to close file!\n");
        return ERR_WRITE_FILE;
    }

    //TODO write to log
    return OK_RESPONSE;
}



int view_treasure(const char *hunt_id, const char *treasure_id) {
    char buff[128];
    sprintf(buff, "%s/treasures", hunt_id);
    struct stat file_stat;
    int rc = stat(buff, &file_stat);

    if(rc < 0) {
        printf("ERROR: Cannot open treasure file for: %s\n", hunt_id);
        return ERR_READ_FILE;
    }
    
    int file_desc = open(buff, O_RDONLY);
    if(file_desc < 0) {
        printf("ERROR: Cannot open treasure file for: %s\n", hunt_id);
        return ERR_READ_FILE;
    }

    bool found = false;
    int no_of_treasures = file_stat.st_size/TREASURE_BLOCK_LENGTH;
    for(int i = 0; i < no_of_treasures; i++) {
        treasure_data treasure;
        ssize_t bytes_read = read(file_desc, &treasure, TREASURE_BLOCK_LENGTH);

        if(bytes_read != TREASURE_BLOCK_LENGTH) {
            close(file_desc);
            printf("ERROR: Read %ld out of %ld bytes!\n", bytes_read, TREASURE_BLOCK_LENGTH);
            return ERR_READ_FILE;
        }

        if(strcmp(treasure_id, treasure.treasure_id) == 0) {
            //TODO print more
            printf("Treasure %s: user: %s, value = %d, at %f, %f\n", 
                treasure.treasure_id, treasure.user_name, treasure.value, treasure.longitude, treasure.latitude);
            found = true;
            break;
        }
    }
    
    rc = close(file_desc);
    if(rc < 0) {
        printf("ERROR: Failed to close file!\n");
        return ERR_WRITE_FILE;
    }

    if(found) {
        return OK_RESPONSE;
    }
    else {
        printf("ERROR: Treasure %s NOT found!\n", treasure_id);
        return ERR_TREASURE_NOT_FOUND;
    }
}


int remove_treasure(const char *hunt_id, const char *treasure_id) {
    char buff[128];
    sprintf(buff, "%s/treasures", hunt_id);
    struct stat file_stat;
    int rc = stat(buff, &file_stat);

    if(rc < 0) {
        printf("ERROR: Cannot open treasure file for: %s\n", hunt_id);
        return ERR_READ_FILE;
    }
    
    int file_desc = open(buff, O_RDWR);
    if(file_desc < 0) {
        printf("ERROR: Cannot open treasure file for: %s\n", hunt_id);
        return ERR_READ_FILE;
    }

    int indx_found = -1;
    int no_of_treasures = file_stat.st_size/TREASURE_BLOCK_LENGTH;
    for(int i = 0; i < no_of_treasures; i++) {
        treasure_data treasure;
        ssize_t bytes_read = read(file_desc, &treasure, TREASURE_BLOCK_LENGTH);

        if(bytes_read != TREASURE_BLOCK_LENGTH) {
            close(file_desc);
            printf("ERROR: Read %ld out of %ld bytes!\n", bytes_read, TREASURE_BLOCK_LENGTH);
            return ERR_READ_FILE;
        }

        if(strcmp(treasure_id, treasure.treasure_id) == 0) {
            indx_found = i;
            break;
        }
    }

    if(indx_found < 0) {
        close(file_desc);
        printf("ERROR: Treasure %s NOT found!\n", treasure_id);
        return ERR_TREASURE_NOT_FOUND;
    }

    //If not removing last treasure, then copy the last treasure over the removed one
    if(indx_found < no_of_treasures - 1) {
        off_t last_offs = TREASURE_BLOCK_LENGTH * (no_of_treasures - 1);
        off_t removed_offs = TREASURE_BLOCK_LENGTH * indx_found;
        treasure_data treasure;

        lseek(file_desc, last_offs, SEEK_SET);
        ssize_t bytes_read = read(file_desc, &treasure, TREASURE_BLOCK_LENGTH);
        if(bytes_read != TREASURE_BLOCK_LENGTH) {
            close(file_desc);
            printf("ERROR: Read %ld out of %ld bytes!\n", bytes_read, TREASURE_BLOCK_LENGTH);
            return ERR_READ_FILE;
        }

        lseek(file_desc, removed_offs, SEEK_SET);
        ssize_t written = write(file_desc, &treasure, TREASURE_BLOCK_LENGTH);
        if(written != TREASURE_BLOCK_LENGTH) {
            close(file_desc);
            printf("ERROR: Written %ld out of %ld bytes!\n", written, TREASURE_BLOCK_LENGTH);
            return ERR_WRITE_FILE;
        }
    }

    rc = ftruncate(file_desc, TREASURE_BLOCK_LENGTH * (no_of_treasures - 1));
    if(rc < 0) {
        close(file_desc);
        printf("ERROR: Failed to truncate file!\n");
        return ERR_WRITE_FILE;
    }
    
    rc = close(file_desc);
    if(rc < 0) {
        printf("ERROR: Failed to close file!\n");
        return ERR_WRITE_FILE;
    }

    return OK_RESPONSE;
}



//-----------------------

int main(int argc, char **argv) {
    if(argc == 3) {
        if(strcmp(argv[1], "--add_hunt") == 0) {
            return add_hunt(argv[2]);
        }

        if(strcmp(argv[1], "--list") == 0) {
            return list_hunt_treasures(argv[2]);
        }

        if(strcmp(argv[1], "--remove_hunt") == 0) {
            return remove_hunt(argv[2]);
        }
    }

    if(argc == 4) {
        if(strcmp(argv[1], "--view") == 0) {
            return view_treasure(argv[2], argv[3]);
        }

        if(strcmp(argv[1], "--remove_treasure") == 0) {
            return remove_treasure(argv[2], argv[3]);
        }
    }

    if(argc == 5)
    {
        if(strcmp(argv[1], "--add_treasure") == 0) {
            //TODO pass all arguments - latitude, longitude, value, cluetext
            return add_treasure(argv[2], argv[3], argv[4]);
        }
    }
    
    printf("Usage: ...\n"); //TODO
    return 1;
}
