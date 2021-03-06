#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "cb_string.h"

int BASE_STRING_LENGTH = 1024;

int append_chars(string* base, char* chars) {
    short should_realloc = 0;
    uint memory_allotted = base->memory_allotted;
    while(memory_allotted < string_length(base) + strlen(chars)) {
        memory_allotted *= 2;
        should_realloc = 1;
    }
    if (should_realloc == 1) {
        printf("reallocing... append_chars\n");
        base->text = (char*) realloc(base->text, memory_allotted);
        base->memory_allotted = memory_allotted;
    }
    strcat(base->text, chars);
    return 0;
}

int va_append_sprintf(string* base, char* fbase, va_list args) {
    // TODO (24 Feb 2020 sam): PERFORMANCE. See if we can do this in a single
    // va loop. Currently, we use vsnprintf to get the length of the temp
    // buffer. Then, we use vsprintf to actually copy the required stuffs to the
    // string. There might be a way to do it in a single rep. Since we do it twice,
    // we also require an extra variable (args_copy);
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, fbase, args) + 1;
    char s[size];
    s[0] = '\0';
    vsprintf(&s[0], fbase, args_copy);
    s[size-1] = '\0';
    append_chars(base, s);
    return 0;
}

int string_length(string* s) {
    return strlen(s->text);
}

int clear_string(string* s) {
    s->text[0] = '\0';
    return 0;
}

int print_string(string* s) {
    printf(s->text);
}

int dispose_string(string* base) {
    free(base->text);
    return 0;
}

string empty_string() {
    return string_from("");
}

string string_from(char* text) {
    unsigned int len = BASE_STRING_LENGTH;
    while (len < strlen(text))
        len *= 2;
    printf("mallocing... string_from\n");
    char* s = (char*) malloc(len * sizeof(char));
    s[0] = '\0';
    strcat(s, text);
    // TODO (31 Mar 2020 sam): for some reason, returning without allocating
    // doesn't work here. We can't just return {s, len}; for som reason...
    string result = {s, len};
    return result;
}

int append_string(string* base, string* appendage) {
    append_chars(base, appendage->text);
    return 0;
}

string stringf(char* base, ...) {
    va_list args;
    va_start(args, base);
    string result = empty_string();
    va_append_sprintf(&result, base, args);
    va_end(args);
    return result;
}

int append_sprintf(string* base, char* fbase, ...) {
    va_list args;
    va_start(args, fbase);
    va_append_sprintf(base, fbase, args);
    va_end(args);
    return 0;
}
