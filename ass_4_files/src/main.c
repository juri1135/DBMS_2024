#include "bpt.h"
#include <stdio.h>
#include <time.h>
int main(){
    int64_t input;
    char instruction;
    char buf[120];
    char *result;
    int index;
    // Assignment 4
    open_table("table1.db", "table2.db");
    // "table1.db", "table2.db"라는 두 파일을 열 수 있게 수정
    
    while(scanf("%c", &instruction) != EOF){
        switch(instruction){
            case 'i':
                scanf("%d %ld %s", &index, &input, buf);
                db_insert(index, input, buf);
                //printf("insertion done\n");
                break;
            case 'f':
                scanf("%ld",&input);
                result = db_find(index, input);
                if (result) {
                    printf("Key: %ld, Value: %s\n", input, result);
                    free(result);
                }
                else
                    printf("Not Exists\n");

                fflush(stdout);
                break;
            // case 'd':
            //     scanf("d %ld",&input);
            //     db_delete(input);
            //     break;
            
            // Assignment 4
            case 'j':
                // struct timespec start, end;
                // if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
                //     perror("clock_gettime start failed");
                //     return 1;
                // }
                db_join();
                // if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) {
                //     perror("clock_gettime end failed");
                //     return 1;
                // }

                // double elapsed_time = (end.tv_sec - start.tv_sec) 
                //                     + (end.tv_nsec - start.tv_nsec) / 1e9;

                // printf("Join operation completed in %.9f seconds\n", elapsed_time);
                break;


            //////////////////////////////////////////////////////////////////

            case 'q':
                while (getchar() != (int)'\n');
                return EXIT_SUCCESS;
                break;   

        }
        while (getchar() != (int)'\n');
    }
    printf("\n");
    return 0;
}



