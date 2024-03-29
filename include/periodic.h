#include <stddef.h>
#include <stdint.h>
#ifndef periodic
#define periodic

#define DBFile "elements.csv"

typedef struct element {
    char* name;
    char* symbol;
    uint8_t anum;
    uint32_t amass;
    char* comment;
} element;

extern int offsets[];

char* strstrip(char* s);

int elementToStr(char** str, element* Element);

int compareElement(const void* elem1, const void* elem2);

int32_t searchElement(element* Elements, size_t length, void* query, int offset);

element updateElement(element* oldData, element* newData);

void removeElement(element* Element);

void saveElement(element* Element);

element* readElements(size_t* length);

void freeElements(element* Elements, size_t length);

void freeElement(element* Element);

#endif