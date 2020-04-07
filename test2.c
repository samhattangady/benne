#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {
    double a;
    clock_t start_time;
    clock_t end_time;
    struct timeval sv;
    struct timeval ev;
    for (int i=0; i< 1000; i++) {
        gettimeofday(&sv, NULL);
        // start_time = clock();
        // end_time = clock();
        // a = (double) (end_time - start_time) / CLOCKS_PER_SEC;
        gettimeofday(&ev, NULL);
        a = (ev.tv_sec - sv.tv_sec) +
            ((ev.tv_usec - sv.tv_usec) / 1000000.0);
        printf("%f\n", 1.0/a);
    }
    return 0;    
}
