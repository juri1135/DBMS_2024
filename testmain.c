#include "bpt.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
FILE *output_file = NULL;
int main() {
    int line_number = 1;
    int64_t input;
    char instruction;
    char buf[120];
    char *result;
    // Variables for CPU time
    clock_t cpu_start, cpu_end;
    double cpu_time_used;

    // Variables for real elapsed time
    struct timeval real_start, real_end;
    long seconds, microseconds;
    double real_elapsed;

    // Start measuring times
    cpu_start = clock();                   // Start CPU time
    gettimeofday(&real_start, NULL); 

    output_file = fopen("output(test).txt", "w");
     open_table("test.db");
    while(scanf("%c", &instruction) != EOF){
        fprintf(output_file,"Debug: Processing line %d, instruction '%c'\n", line_number, instruction);
        if(line_number%1000==0) printf("Debug: Processing line %d, instruction '%c'\n", line_number, instruction);
        // print_leaf();
        switch(instruction){
            case 'i':
                scanf("%ld %s", &input, buf);
                db_insert(input, buf);
                break;
            case 'f':
                scanf("%ld", &input);
                result = db_find(input);
                if (result) {
                    printf("Key: %ld, Value: %s\n", input, result);
                     free(result);
                }
                else
                    printf("Not Exists\n");

                fflush(stdout);
                break;
            case 'd':
                scanf("%ld", &input);
                int check=db_delete(input);
                if(check==-1) fprintf(output_file,"deletion failed/n");
                break;
            case 'q':
                while (getchar() != (int)'\n');
                return EXIT_SUCCESS;
                break;  
                //p 돌릴 땐 max값 4 정도로 수정해서 돌려보기..!!!! 
            case 'p':
                print_bpt();
                break;
            case 'r':
                tree_reset_all();
                break;
        }
        line_number++; 
        while (getchar() != (int)'\n');
    }
    // End measuring times
    cpu_end = clock();                     // End CPU time
    gettimeofday(&real_end, NULL);         // End real elapsed time

    // Calculate CPU time
    cpu_time_used = ((double)(cpu_end - cpu_start)) / CLOCKS_PER_SEC;

    // Calculate real elapsed time
    seconds = real_end.tv_sec - real_start.tv_sec;
    microseconds = real_end.tv_usec - real_start.tv_usec;
    real_elapsed = seconds + microseconds * 1e-6;

    // Print results
    printf("CPU time: %f seconds\n", cpu_time_used);
    printf("Real elapsed time: %f seconds\n", real_elapsed);

    fclose(output_file);  // 파일 닫기
    return 0;
}
