#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

long int Wind(FILE* f) {
    fseek(f, 0, SEEK_END);
    return ftell(f);
}

char Check_Buf(char* buf, char* s) {
    if (strstr(buf, s)) {
        return 1;
    }
    return 0;
}

int main(void) {

    // Open the file
    char* in_file = "<filepath>";
    FILE* f;
    if (!(f = fopen(in_file, "r"))) {
        fprintf(stderr, "Can't open file, my guy\n");
        return 1;
    };

    // Find end of file
    fseek(f, 0, SEEK_END);
    long int prev_pos = Wind(f);
    printf("Seeking to end of file (%ld bytes)\n", prev_pos);

    for(;;) {
        // move to end of file
        long int curr_pos = Wind(f);
        // if end position is different, file has had data appended
        if (curr_pos > prev_pos) {
            long int delta = curr_pos - prev_pos;
            // starting from newly appended data, copy bytes to buffer until eof
            fseek(f, -delta, SEEK_CUR);
            char* buf = (char*)malloc(delta + 1);
            fread(buf, 1, delta, f);

            // check if buffer contains target string
            if (Check_Buf(buf, "youtube")) {
                printf("TRIGGER\n");
            }

            prev_pos = curr_pos;
            free(buf);
        }

        sleep(1);
    }

    fclose(f);
    return 0;

}
