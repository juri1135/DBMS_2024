#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#define LEAF_MAX 31
#define INTERNAL_MAX 248

typedef struct record{
    int64_t key;
    char value[120];
}record;

typedef struct inter_record {
    int64_t key;
    off_t p_offset;
}I_R;

typedef struct Page{
    off_t parent_page_offset;
    int is_leaf;
    int num_of_keys;
    char reserved[104];
    off_t next_offset;
    union{
        I_R b_f[248];
        record records[31];
    };
}page;

typedef struct Header_Page{
    off_t fpo;
    off_t rpo;
    int64_t num_of_pages;
    char reserved[4072];
}H_P;


extern int fd[2];

extern page * rt[2];

extern H_P * hp[2];
// FUNCTION PROTOTYPES.
int open_table(char * pathname1, char * pathname2);
H_P * load_header(int index, off_t off);
page * load_page(int index, off_t off);

void reset(int index, off_t off);
off_t new_page(int index);
off_t find_leaf(int index, int64_t key);
char * db_find(int index, int64_t key);
void freetouse(int index, off_t fpo);
int cut(int length);
int parser();

void start_new_file(int index, record rec);
int db_insert(int index, int64_t key, char * value);
off_t insert_into_leaf(int index, off_t leaf, record inst);
off_t insert_into_leaf_as(int index, off_t leaf, record inst);
off_t insert_into_parent(int index, off_t old, int64_t key, off_t newp);
int get_left_index(int index, off_t left);
off_t insert_into_new_root(int index, off_t old, int64_t key, off_t newp);
off_t insert_into_internal(int index, off_t bumo, int left_index, int64_t key, off_t newp);
off_t insert_into_internal_as(int index, off_t bumo, int left_index, int64_t key, off_t newp);

int db_delete(int index, int64_t key);
void delete_entry(int index, int64_t key, off_t deloff);
void redistribute_pages(int index, off_t need_more, int nbor_index, off_t nbor_off, off_t par_off, int64_t k_prime, int k_prime_index);
void coalesce_pages(int index, off_t will_be_coal, int nbor_index, off_t nbor_off, off_t par_off, int64_t k_prime);
void adjust_root(int index, off_t deloff);
void remove_entry_from_page(int index, int64_t key, off_t deloff);
void usetofree(int index, off_t wbf);

// Assignment 4
void db_join();

#endif /* __BPT_H__*/


