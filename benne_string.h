#ifndef BENNE_STRING_DEFINED
#define BENNE_STRING_DEFINED
typedef struct string {
    char* text;
    int memory_allotted;
} string;


string empty_string();
string string_from(char* text);
int append_string(string* base, string* appendage);
int dispose_string(string* s);
int string_length(string* s);
int append_sprintf(string* s, char* base, ...);
#endif
