#include <ncurses.h>
#include <form.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h> // isspace()
#define DBFile "elements.csv"
/**
 * Remove trailing and leading whitespace from str
 * @param s Pointer to string
 * @return Pointer to our new string. (which is the same string)
*/
char* strstrip(char* s)
{
        size_t size;
        char *end;

        size = strlen(s);

        if (!size)
                return s;

        end = s + size - 1;
        while (end >= s && isspace(*end))
                end--;
        *(end + 1) = '\0';

        while (*s && isspace(*s))
                s++;

        return s;
}

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
 * Remove an element from the .csv file. This works be rewriting the whole file except for the line with our element
 * @param Element THe element to be removed
*/
// !!!!! DOES NOT REMOVE NEWLINE CHARACTER
void removeElement(element* Element) {
    char* ElementStr, *ElementStrN;
    FILE* fp, *temp;
    char* line = NULL; // Line has to be a initialized pointer
    size_t read = 0;
    ssize_t nread;
    // ElementStr = How does the element look in csv format? The string WILL have a newline at the end
    if (0 > asprintf(&ElementStr, "%s,%s,%d,%d,%s", Element->name, Element->symbol, Element->anum, Element->amass, Element->comment)) {
        perror("Error creating ElementStr in removeElement()");
        exit(1);
    }
    strcpy(ElementStrN, ElementStr);
    strcat(ElementStrN, "\n"); // Create a variation of our string with newline character at the end
    fp = fopen(DBFile, "r+");
    if (fp == NULL) {
        perror("Error opening DBFile");
        exit(1);
    }
    temp = fopen(".TEMPFILE", "w");
    if (temp == NULL) {
        perror("Error opening temp file");
        exit(1);
    }
    while ((nread = getline(&line, &read, fp)) != -1) {
        if ((strcmp(line, ElementStr) == 0) || (strcmp(line, ElementStrN) == 0)) {
             continue; // Skip our element
        }
        fputs(line, temp);
    }
    fclose(temp);
    fclose(fp);
    remove(DBFile);
    rename(".TEMPFILE", DBFile);
}
/**
 * Save an element at the end of the .csv file
 * @param Element The element to be added
*/
void saveElement(element* Element) {
    FILE* fp = fopen(DBFile, "a");
    fprintf(fp, "\n%s,%s,%d,%d,%s", Element->name, Element->symbol, Element->anum, Element->amass, Element->comment); 
    fclose(fp);
}
/**
 * This function will load all elements from a .csv file into an array.
 * @param length a variable to store the length of the array
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
    char* line = NULL; // Line read from file
    *length = (size_t)1; // Initialize length of array to 1
    size_t len = 0; // Actual length that got read by getline()
    ssize_t nread; // Size of data read by getline(), has to be signed
    element* elements = malloc(sizeof(element) * 2);
    while((nread = getline(&line, &len, fp)) != -1) {
        // Load each line into a struct and add into an array
        element Element;
        char* token = strtok(line, ",");
        for (int i = 0; i < 5; i++) {
            printf("%d: %s\n", i, token);
            switch (i) {
            case 0: {
                char* n = malloc(strlen(token) + 1); // Case must be inside code block to declare variables
                strcpy(n, token);
                Element.name = n;
                break;
            }
            case 1: {
                char* n1 = malloc(strlen(token) + 1);
                strcpy(n1, token);
                Element.symbol = n1;
                break;
            }
            case 2:
                *(uint8_t*)((char*)&Element+offsets[i]) = *token - '0';
                break;
            case 3:
                *(uint32_t*)((char*)&Element+offsets[i]) = *token - '0';
                break;
            case 4: {
                char* n4 = malloc(strlen(token) + 1);
                strcpy(n4, token);
                Element.comment = n4;
                break;
            }
            default:
                exit(1);
                break;
            }
            token = strtok(NULL, ",");
        }
        elements[*length-1] = Element;
        (*length)++;
        
    }
    fclose(fp);
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
    printf("%s\n%d\n", elements[0].name, elements[0].anum);
    printf("%s\n%d\n", elements[2].name, elements[1].anum);
    element test = {.name = "Nitrogen", .symbol = "N", .anum = 7, .amass = 14, .comment = "Favourite Element"};
    // saveElement(&test);
    removeElement(&test);
    return 0;
}