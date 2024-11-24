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
// #define LEAF_MAX 31
// #define INTERNAL_MAX 248
#define LEAF_MAX 2
#define INTERNAL_MAX 2
//leaf node에서 node가 갖고 있는 key, record pair
typedef struct record{
    int64_t key;
    char value[120];
}record;
//internal node에서 node가 갖고 있는 key, 자식 가리키는 pointer pair
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


extern int fd;

extern page * rt;

extern H_P * hp;
// FUNCTION PROTOTYPES.
int open_table(char * pathname);
int originopen_table(char * pathname);
H_P * load_header(off_t off);
H_P * originload_header(off_t off);
page * load_page(off_t off);
page * originload_page(off_t off);
void reset(off_t off);
off_t new_page();
void freetouse(off_t fpo);
int cut(int length);
int parser();
void start_new_file(record rec);

char * db_find(int64_t key);
int db_insert(int64_t key, char * value);
int db_delete(int64_t key);

void print_keys(off_t page_offset, int depth);
void print_bpt();
page * find_leaf(off_t root, int key);
off_t find_offset(int64_t key);
record make_record(int64_t key, char * value);
int get_left_index(page* parent, off_t left_offset);
page * insert_into_new_root(page* left, off_t left_offset, int key, page* right, off_t right_offset);
page * insert_into_node(page * root, page * page, off_t page_offset, int left_index, int64_t new_key, off_t new_offset);
page * insert_into_leaf(page * leaf, int64_t key, record pair);
page * insert_into_parent(page * root, page * origin_child, off_t origin_child_offset, int64_t key, page* new_child, off_t new_child_offset);
page * insert_into_node_after_splitting(page * root, off_t old_node_offset, page * old_node, int left_index, int64_t new_key, off_t new_pointer);
page * insert_into_leaf_after_splitting(page * root, page * leaf, int64_t key, record pair);
page * leaf_right_rotation(page * root, off_t leaf_offset, page* leaf, off_t right_sibling_offset, page * right_sibling, record pair);
page * internal_left_rotation(page * root, off_t left_sibling_offset, page * left_sibling, off_t node_offset, page* node, int64_t key, off_t pointer);
page * internal_right_rotation(page * root, off_t node_offset, page* node, off_t right_sibling_offset, page * right_sibling, int64_t key, off_t pointer);

page * remove_entry_from_node(page * n, off_t n_offset,int key);
page * adjust_root(page * root);
page * coalesce_nodes(page * root, page * n, off_t n_offset, page * neighbor, off_t neighbor_offset, int neighbor_index, int k_prime);
page * redistribute_nodes(page * root, page * n, off_t n_offset, page * neighbor, off_t neighbor_offset, int neighbor_index, int k_prime_index, int k_prime);
page * delete_entry(page * root, off_t n_offset, int key);
void tree_reset_all();
void reset_tree_nodes(page *current);

#endif /* __BPT_H__*/


