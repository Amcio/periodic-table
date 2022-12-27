#include <ncurses.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#define DBFile "elements.csv"

typedef struct element {
    char* name;
    char* symbol;
    uint8_t anum;
    uint32_t amass;
    char* comment;
} element;

int offsets[] = {
    offsetof(element, name),
    offsetof(element, symbol),
    offsetof(element, anum),
    offsetof(element, amass),
    offsetof(element, comment)
};

/**
 * This function will load all elements from a .csv file into an array.
 * @param length is a variable to store the length of the array
 * @return An array filled with element structs
*/
element* readElements(size_t* length) {
    FILE* fp = fopen(DBFile, "r");
    if (fp == NULL) {
        perror("fopen");
        if (errno == 2) {
            printf("Are you sure you created the database?");
        }
        exit(1);
    }
    char* line = NULL;
    *length = (size_t)1;
    size_t len = 0; // Actual length that got read by getline()
    ssize_t nread; // Size of data read by getline(), has to be signed
    element* elements = malloc(sizeof(element));
    while((nread = getline(&line, &len, fp)) != -1) {
        // Load each line into a struct and add into an array
        element* Element = malloc(sizeof(element));
        char* token = strtok(line, ",");
        for (int i = 0; i < 5; i++) {
            printf("%d: %s\n", i, token);
            switch (i) {
            case 2:
                printf("Reached 2.\n");
                *(uint8_t*)(Element+offsets[i]) = (uint8_t)(*token - '0');
                break;
            case 3:
                *(uint32_t*)(Element+offsets[i]) = (uint32_t)((*token) - '0');
                break;
            default:
                char* field = (char*)(Element+offsets[i]);
                field = token;
                // *(char*)(Element+offsets[i]) = *token;
                break;
            }
            token = strtok(NULL, ",");
        }
        elements[0] = *Element;
        
    }
    return elements;
}

int main() {
    FILE* fp = fopen("test", "wb");
    size_t buf = 20;
    char* str = "Something\\n\n";
    fwrite(str, sizeof(char), strlen(str), fp);
    fclose(fp);
    fp = fopen("test", "r");
    char* two;
    getline(&two, &buf, fp);
    printf("%s", two);
    size_t len;
    element* elements = readElements(&len);
    printf("%d\n%d\n", elements[0].anum, elements[0].amass);

    return 0;
}