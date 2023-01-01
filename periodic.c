#include <ncurses.h>
#include <form.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h> // ftruncate
#include <ctype.h> // isspace()
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
/**
 * Create a string for writing or reading into the csv file
 * @param str char* for storing the string
 * @param Element The element to convert
 * @return 1 on any error
*/
int elementToStr(char** str, element* Element) {
    if (0 > asprintf(str, "%s,%s,%d,%d,%s", Element->name, Element->symbol, Element->anum, Element->amass, Element->comment)) {
        return 1;
    }
    return 0;
}
/**
 * Search the list of elements based on one of the parameters
 * @param query What to look for
 * @param offset The offset to use for the field (0 or 1 for name or symbol respectively)
 * @return Pointer to the element that matches the query, struct will contain NULL if failed
*/
element* searchElement(void* query, int offset) {
    if (offset == 0 || offset == 1) {
        // String
    } else if (offset == 2 || offset == 3) {
        // Int
    } else {
        // Error
    }
}
/**
 * Overwrite an element's data
 * @param oldData The old element to look for
 * @param newData THe new data to write
 * @return Pointer to the newData struct, update the array with it
*/
element updateElement(element* oldData, element* newData) {
    int count = 0;
    char* line = NULL;
    size_t len = 0; // Actual length that got read by getline()
    ssize_t nread; // Size of data read by getline(), has to be signed
    FILE *dbp, *tmpfile;
    // here be demons
    char* ElementStr, *ElementStrN, *ElementStrNew;
    printf("ad");
    asprintf(&ElementStr, "%s,%s,%d,%d,%s", oldData->name, oldData->symbol, oldData->anum, oldData->amass, oldData->comment);
    // if (elementToStr(&ElementStr, oldData)) {
    //     perror("Error creating ElementStr in updateElement()");
    //     exit(1);
    // }
    printf("%s", ElementStr);
    if (elementToStr(&ElementStrNew, newData)) {
        perror("Error creating ElementStr in updateElement()");
        exit(1);
    }
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
            strcat(ElementStrNew, "\n");
            fputs(ElementStrNew, tmpfile);
        } else {
            fputs(line, tmpfile);
        }
    }
    fclose(tmpfile);
    fclose(dbp);
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
    if (elementToStr(&ElementStr, Element)) {
        perror("Error creating ElementStr in removeElement()");
        exit(1);
    }
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
// TODO: INCOMPLETE, read only one digit for amass and anum, also size of elements array is hardcoded to 2
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
                char* n = malloc(strlen(token) + 1);
                strcpy(n, token);
                Element.symbol = n;
                break;
            }
            case 2:
                *(uint8_t*)((char*)&Element+offsets[i]) = *token - '0';
                break;
            case 3:
                *(uint32_t*)((char*)&Element+offsets[i]) = *token - '0';
                break;
            case 4: {
                char* n = malloc(strlen(token) + 1);
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
    printf("%s\n%d\n", elements[1].name, elements[1].anum);
    element test = {.name = "Nitrogen", .symbol = "N", .anum = 7, .amass = 14, .comment = "Favourite Element"};
    // saveElement(&test);
    // removeElement(&test);
    updateElement(&(elements[1]), &test);
    return 0;
}