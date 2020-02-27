#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "benne_string.h"

int BASE_STRING_LENGTH = 1024;

int string_length(string* s) {
    return strlen(s->text);
}

string string_from(char* text) {
    int len = BASE_STRING_LENGTH;
    while (len < strlen(text)) {
        len = len * 2;
    }
    char* s = (char*) malloc(sizeof(char) * len);
    s[0] = '\0';
    strcat(s, text);
    return {s, len};
}

string empty_string() {
    return string_from("");
}

int append_string(string* base, string* appendage) {
    while(base->memory_allotted < string_length(base) + string_length(appendage)) {
        base->text = (char*) realloc(base->text, base->memory_allotted*2);
        base->memory_allotted *= 2;
    }
    strcat(base->text, appendage->text);
    return 0;
}

int dispose_string(string* s) {
    free(s->text);    
    return 0;
}

int append_sprintf(string* s, char* base, ...) {
    va_list args;
    // TODO (24 Feb 2020 sam): PERFORMANCE. See if we can do this in a single
    // va loop. Currently, we use vsnprintf to get the length of the temp
    // buffer. Then, we use vsprintf to actually copy the required stuffs to the
    // string. There might be a way to do it in a single rep.
    va_start(args, base);
    int size = vsnprintf(NULL, 0, base, args) + 1;
    va_end(args);
    char* appendage = (char*) malloc(sizeof(char) * size);
    appendage[0] = '\0';
    va_start(args, base);
    vsprintf(appendage, base, args);
    va_end(args);
    appendage[size] = '\0';
    // TODO (24 Feb 2020 sam): PERFORMANCE. We can improve by not creating an
    // additional malloc from string_from, and just strcat (with memory checks)
    // from within this function itself.
    string appendage_string = string_from(appendage);
    append_string(s, &appendage_string);
    dispose_string(&appendage_string);
    free(appendage);
    return 0;
}

