#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
typedef struct {
    int key;
    bool in_tree;
} KeyState;
void generate_test_case_file(const char *filename, int num_cases) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    srand((unsigned int)time(NULL));
    KeyState *key_states = malloc(num_cases * sizeof(KeyState)); // Track all key states
    if (key_states == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    int key_count = 0;
    for (int i = 0; i < num_cases; i++) {
        // Randomly choose between insertion (70% chance) and deletion (30% chance)
        if (key_count > 0 && rand() % 100 < 30) {
            int candidate_index = rand() % key_count;
            if (key_states[candidate_index].in_tree) {
                fprintf(file, "d %d\n", key_states[candidate_index].key);
                key_states[candidate_index].in_tree = false;
            } else {
                i--;
                continue;
            }
        } else {
            // Generate a random unique key for insertion
            int key = rand() % 1000000 + 1;
            bool key_exists = false;
            for (int j = 0; j < key_count; j++) {
                if (key_states[j].key == key) {
                    key_exists = true;
                    break;
                }
            }
            if (key_exists) {
                i--;
                continue;
            }
            char value[6];
            for (int j = 0; j < 5; j++) {
                value[j] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"[rand() % 62];
            }
            value[5] = '\0';
            fprintf(file, "i %d %s\n", key, value);
            key_states[key_count].key = key;
            key_states[key_count].in_tree = true;
            key_count++;
        }
    }
    free(key_states);
    fclose(file);
}
int main() {
    int num_cases_array[] = {500,1000};
    int num_test_cases = sizeof(num_cases_array) / sizeof(num_cases_array[0]);
    for (int i = 0; i < num_test_cases; i++) {
        char filename[50];
        sprintf(filename, "testcase_%d.txt", num_cases_array[i]);
        printf("Generating test case file '%s' with %d cases...\n", filename, num_cases_array[i]);
        generate_test_case_file(filename, num_cases_array[i]);
    }
    printf("All test case files generated successfully.\n");
    return 0;
}