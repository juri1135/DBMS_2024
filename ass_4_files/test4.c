#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void generate_test_case_file(const char *filename, int num_cases) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    srand((unsigned int)time(NULL));

    for (int i = 0; i < num_cases; i++) {
        int table_index = rand() % 2;           // 0 또는 1
        int key = rand() % 1000 + 1;            // 1 ~ 1000
        char value[6];

        for (int j = 0; j < 5; j++) {
            value[j] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"[rand() % 62];
        }
        value[5] = '\0';

        fprintf(file, "i %d %d %s\n", table_index, key, value);
    }

    // 마지막에 join 명령
    fprintf(file, "j\n");

    fclose(file);
}

int main() {
    int num_cases = 1000; // 500000줄 생성
    char filename[50];
    sprintf(filename, "testcase_%d.txt", num_cases);
    printf("Generating test case file '%s' with %d cases...\n", filename, num_cases);
    generate_test_case_file(filename, num_cases);
    printf("Test case file generated successfully.\n");
    return 0;
}