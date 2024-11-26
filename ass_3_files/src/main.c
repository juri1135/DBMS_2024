#include "bpt.h"
#include <stdio.h>
#include <stdlib.h>
FILE *output_file = NULL;
int main() {
    int line_number = 1;
    int64_t input;
    char instruction;
    char buf[120];
    char *result;

    output_file = fopen("output2.txt", "w");
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
    printf("끝~\n");
    fclose(output_file);  // 파일 닫기
    return 0;
}
