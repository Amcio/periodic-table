#define _GNU_SOURCE // asprintf()
#include <ncurses.h>
#include <form.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h> // ftruncate
#include <ctype.h> // isspace()
#include "periodic.h"


int offsets[] = {
    offsetof(element, name),
    offsetof(element, symbol),
    offsetof(element, anum),
    offsetof(element, amass),
    offsetof(element, comment)
};

/**
 * Remove trailing and leading whitespace from str
 * @param s Pointer to string
 * @return Pointer to our new string. (which is the same string)
*/
char* strstrip(char* s) {
        size_t size;
        char *end;

        size = strlen(s);

        if (!size) {
            return s;
        }

        end = s + size - 1;
        while (end >= s && isspace(*end)) {
            end--;
        }
        *(end + 1) = '\0';

        while (*s && isspace(*s)) {
            s++;
        }

        return s;
}

/**
 * Create a string for writing or reading into the csv file
 * @param str char* for storing the string
 * @param Element The element to convert
 * @return -1 on any error, otherwise length of string
*/
int elementToStr(char** str, element* Element) {
    return asprintf(str, "%s,%s,%d,%d,%s", Element->name, Element->symbol, Element->anum, Element->amass, Element->comment);
}

/**
 * Compare two elements based on their atomic numbers.
 * @param elem1 The first element
 * @param elem2 The second element
 * @return -1 if less than, 0 if equal, 1 if more than
*/
int compareElement(const void* elem1, const void* elem2) {
    element* p1 = (element*)elem1;
    element* p2 = (element*)elem2;
    if (p1->anum < p2->anum) {
        return -1;
    } else if (p1->anum == p2->anum) {
        return 0;
    } else if (p1->anum > p2->anum) {
        return 1;
    }
}

/**
 * Search the list of elements based on one of the parameters
 * @param Elements the array of elements
 * @param length length of the array (provided by readElements)
 * @param query What to look for, must be an address
 * @param offset The offset to use for the field (0 - name, 1 - symbol, 2 - atomic number, 3 - atomic mass)
 * @return Index of the element in the array, -1 if failed
*/
int32_t searchElement(element* Elements, size_t length, void* query, int offset) {
    if (offset == 0 || offset == 1) {
        // String
        char* queryS = (char*)query;
        for (int i = 0; i < length; i++) {
            /* Compare name of element and symbol of element to the query */
            if ((strcmp(Elements[i].name, queryS) == 0) || (strcmp(Elements[i].symbol, queryS) == 0)) {
                return i;
            }
        }
        return -1;
    } else if (offset == 2 || offset == 3) {
        // Int
        for (int i = 0; i < length; i++) {
            if (offset == 2) {
                if (Elements[i].anum == *(int*)query) {
                    return i;
                }
            } else {
                if (Elements[i].amass == *(int*)query) {
                   return i;
             }
            }
        }
        return -1;
    } else {
        // Error
        return -1;
    }
}
/**
 * Overwrite an element's data
 * @param oldData The old element to look for
 * @param newData THe new data to write
 * @return Pointer to the newData struct, update the array with it
*/
element updateElement(element* oldData, element* newData) {
    int count = 0, strl = 0;
    char* line = NULL;
    size_t len = 0; // Actual length that got read by getline()
    ssize_t nread; // Size of data read by getline(), has to be signed
    FILE *dbp, *tmpfile;
    char* ElementStr, *ElementStrN, *ElementStrNew;
    if ((strl = elementToStr(&ElementStr, oldData)) == -1) {
        perror("Error creating ElementStr in updateElement()");
        exit(1);
    }
    if (elementToStr(&ElementStrNew, newData) == -1) {
        perror("Error creating ElementStrNew in updateElement()");
        exit(1);
    }
    ElementStrN = malloc(sizeof(char) * (strl + 2)); // length of ElementStr + \n + \0
    strcpy(ElementStrN, ElementStr);
    strcat(ElementStrN, "\n"); // Create a variation of our string with newline character at the end
    dbp = fopen(DBFile, "r");
    if (dbp == NULL) {
        perror("Error opening DBFile in updateElement()");
        exit(1);
    }
    tmpfile = fopen(".TEMPFILE", "w");
    if (tmpfile == NULL) {
        perror("Error opening temp file in updateElement()");
        exit(1);
    }
    while ((nread = getline(&line, &len, dbp)) != -1) {
        if ((strcmp(ElementStr, line) == 0)) {
            fputs(ElementStrNew, tmpfile);
        } else if (strcmp(ElementStrN, line) == 0) {
            ElementStrN = realloc(ElementStrN, strlen(ElementStrN) + 2);
            strcat(ElementStrNew, "\n");
            fputs(ElementStrNew, tmpfile);
        } else {
            fputs(line, tmpfile);
        }
    }
    fclose(tmpfile);
    fclose(dbp);
    free(ElementStr);
    free(ElementStrN);
    free(ElementStrNew);
    remove(DBFile);
    rename(".TEMPFILE", DBFile);
    return *newData;
}
/**
 * Remove an element from the .csv file. This works be rewriting the whole file except for the line with our element
 * @param Element THe element to be removed
*/
void removeElement(element* Element) {
    char* ElementStr, *ElementStrN;
    FILE* dbp, *temp;
    char* line = NULL; // Line has to be a initialized pointer
    size_t read = 0;
    ssize_t nread;
    // ElementStr = How does the element look in csv format? The string WILL have a newline at the end
    if (elementToStr(&ElementStr, Element) == -1) {
        perror("Error creating ElementStr in removeElement()");
        exit(1);
    }
    ElementStrN = malloc(sizeof(char) * (strlen(ElementStr) + 2)); // ElementStr + \n + \0
    strcpy(ElementStrN, ElementStr);
    strcat(ElementStrN, "\n"); // Create a variation of our string with newline character at the end
    dbp = fopen(DBFile, "r+");
    if (dbp == NULL) {
        perror("Error opening DBFile");
        exit(1);
    }
    temp = fopen(".TEMPFILE", "w");
    if (temp == NULL) {
        perror("Error opening temp file");
        exit(1);
    }
    while ((nread = getline(&line, &read, dbp)) != -1) {
        // Last condition probably not needed
        if ((strcmp(line, ElementStr) == 0) || (strcmp(line, ElementStrN) == 0) || (strcmp(line, "\n") == 0)) {
             continue; // Skip our element
        }
        fputs(line, temp);
    }
    // Remove the last newline
    fseek(temp, 0, SEEK_END);
    long int fsize = ftell(temp);
    int fd = fileno(temp); // File descriptor for temp file
    if (ftruncate(fd, fsize - 1) == -1) {
        perror("Error truncating tempfile");
        exit(1);
    }
    fclose(temp);
    fclose(dbp);
    free(ElementStr);
    free(ElementStrN);
    remove(DBFile);
    rename(".TEMPFILE", DBFile);
}
/**
 * Save an element at the end of the .csv file, it is your job to update the array and it's length variable
 * @param Element The element to be added
*/
void saveElement(element* Element) {
    FILE* fp = fopen(DBFile, "a");
    if (fp == NULL) {
        perror("saveElement: fopen");
        exit(1);
    }
    fprintf(fp, "\n%s,%s,%d,%d,%s", Element->name, Element->symbol, Element->anum, Element->amass, Element->comment); 
    fclose(fp);
}
/**
 * This function will load all elements from a .csv file into an array. The array is NOT terminated.
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
    *length = (size_t)0; // Initialize length of array to 0
    size_t len = 0; // Actual length that got read by getline()
    ssize_t nread; // Size of data read by getline(), has to be signed
    element* elements = (element*)NULL;
    while((nread = getline(&line, &len, fp)) != -1) {
        // Load each line into a struct and add into an array
        element Element;
        char* token = strtok(line, ",");
        for (int i = 0; i < 5; i++) {
            // printf("%d: %s\n", i, token);
            switch (i) {
            case 0: {
                char* n = (char*)malloc(strlen(token) + 1); // Case must be inside code block to declare variables
                strcpy(n, token);
                Element.name = n;
                break;
            }
            case 1: {
                char* n = (char*)malloc(strlen(token) + 1);
                strcpy(n, token);
                Element.symbol = n;
                break;
            }
            case 2:
                *(uint8_t*)((char*)&Element+offsets[i]) = strtol(token, (char **)NULL, 10);
                break;
            case 3:
                *(uint32_t*)((char*)&Element+offsets[i]) = strtol(token, (char**)NULL, 10);
                break;
            case 4: {
                char* n = (char*)malloc(strlen(token) + 1);
                strcpy(n, token);
                Element.comment = n;
                break;
            }
            default:
                exit(1);
                break;
            }
            token = strtok(NULL, ",");
        }
        elements = (element*)realloc(elements, sizeof(element) * (*length + 1));
        elements[*length] = Element;
        (*length)++;
        
    }
    fclose(fp);
    qsort(elements, *length, sizeof(element), compareElement);
    return elements;
}

// int main() {
//     FILE* fp = fopen("test", "wb");
//     size_t buf = 20;
//     char* str = "Something\\n\n";
//     fwrite(str, sizeof(char), strlen(str), fp);
//     fclose(fp);
//     fp = fopen("test", "r");
//     char* two;
//     getline(&two, &buf, fp);
//     printf("%s", two);
//     size_t len;
//     element* elements = readElements(&len);
//     printf("%s\n%d\n", elements[0].name, elements[0].anum);
//     printf("%s\n%d\n", elements[1].name, elements[1].anum);
//     element test = {.name = "Nitrogen", .symbol = "N", .anum = 7, .amass = 14, .comment = "Favourite Element"};
//     // saveElement(&test);
//     // removeElement(&test);
//     // updateElement(&(elements[1]), &test);
//     int a = 14;
//     printf("Found: %s\n", searchElement(elements, len, &a, 3)->name);
//     return 0;
// }