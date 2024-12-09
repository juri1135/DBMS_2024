#include "bpt.h"

int main(){
    int64_t input;
    char instruction;
    char buf[120];
    char *result;
    int index;
    // Assignment 4
    // open_table("table1.db", "table2.db");
    // "table1.db", "table2.db"라는 두 파일을 열 수 있게 수정
    
    while(scanf("%c", &instruction) != EOF){
        switch(instruction){
            case 'i':
                scanf("%d %ld %s", &index, &input, buf);
                db_insert(index, input, buf);
                break;
            case 'f':
                scanf("%d %ld", &index, &input);
                result = db_find(index, input);
                if (result) {
                    printf("Key: %ld, Value: %s\n", input, result);
                    free(result);
                }
                else
                    printf("Not Exists\n");

                fflush(stdout);
                break;
            case 'd':
                scanf("%d %ld", &index, &input);
                db_delete(index, input);
                break;
            
            // Assignment 4
            case 'j':
                db_join();
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



