#include <stdio.h>
#include <stdlib.h>

int main(int, char**) {
    float a;
    char c[50] = "10.30";
    sscanf(c, "%f", &a);
    printf("%f\n", a);
    return 0;    
}
