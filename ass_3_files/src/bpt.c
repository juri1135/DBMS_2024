#include "bpt.h"
//header page. root, free, number of pages
H_P * hp;

page * rt = NULL; //root is declared as global

int fd = -1; //fd is declared as global
static int table_counter = 0; 

void print_bpt() {
    if (hp->rpo == 0) {
        printf("Tree is empty.\n");
        return;
    }

    printf("B+ Tree keys:\n");
    printf("hp->rpo = %ld\n", hp->rpo);
    print_keys(hp->rpo, 0);
}


// B+ Tree의 키를 출력하는 함수
void print_keys(off_t page_offset, int depth) {
    // 노드를 로드
    page *current_page = load_page(page_offset);

    // 들여쓰기(Depth) 출력
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    // 현재 노드의 키 출력
    printf("[");
    for (int i = 0; i < current_page->num_of_keys; i++) {
        if (current_page->is_leaf) {
            printf("%" PRId64, current_page->records[i].key); // 리프 노드
        } else {
            printf("%" PRId64, current_page->b_f[i].key); // 내부 노드
        }
        if (i < current_page->num_of_keys - 1) {
            printf(", ");
        }
    }
    printf("]\n");

    // 리프 노드인 경우 탐색 종료
    if (current_page->is_leaf) {
        free(current_page);
        return;
    }

    off_t child_offset;
    // 내부 노드라면 자식 노드를 탐색
    for (int i = 0; i <= current_page->num_of_keys; i++) {
        if(current_page->num_of_keys == INTERNAL_MAX && i == current_page->num_of_keys){
            child_offset = current_page->next_offset;
        }
        else{
            child_offset = current_page->b_f[i].p_offset;
        }
        print_keys(child_offset, depth + 1); // 재귀적으로 자식 노드 탐색
    }

    free(current_page);
}



H_P * load_header(off_t off) {
    H_P * newhp = (H_P*)calloc(1, sizeof(H_P));
    if (sizeof(H_P) > pread(fd, newhp, sizeof(H_P), 0)) {

        return NULL;
    }
    return newhp;
}


page * load_page(off_t off) {
    page* load = (page*)calloc(1, sizeof(page));
    //if (off % sizeof(page) != 0) printf("load fail : page offset error\n");
    if (sizeof(page) > pread(fd, load, sizeof(page), off)) {

        return NULL;
    }
    return load;
}

int open_table(char * pathname) {
    //pathname으로 file 열고, 존재하지 않으면 생성. 존재한다면 생성 x
    fd = open(pathname, O_RDWR | O_CREAT | O_EXCL | O_SYNC  , 0775);
    hp = (H_P *)calloc(1, sizeof(H_P));
    //새 file이 생성된 경우 
    if (fd > 0) {
        //printf("New File created\n");
        //free page는 0, page 수는 header page 1개, root page도 지금은 없음. insert해야 생김
        hp->fpo = 0;
        hp->num_of_pages = 1;
        hp->rpo = 0;
        pwrite(fd, hp, sizeof(H_P), 0);
        free(hp);
        hp = load_header(0);
    }
    //기존 file 있어서 생성되지 않았다면 그 file 열어.
    else{
        fd = open(pathname, O_RDWR|O_SYNC);
        if (sizeof(H_P) > pread(fd, hp, sizeof(H_P), 0)) {
            return -1;
        }
        off_t r_o = hp->rpo;
        rt = load_page(r_o);
    }
    
    int table_id = ++table_counter;

    return table_id; 
}
int originopen_table(char * pathname) {
    fd = open(pathname, O_RDWR | O_CREAT | O_EXCL | O_SYNC  , 0775);
    hp = (H_P *)calloc(1, sizeof(H_P));
    //새 file이 생성된 경우 
    if (fd > 0) {
        //printf("New File created\n");
        //free page는 0, page 수는 header page 1개, root page도 지금은 없음. insert해야 생김
        hp->fpo = 0;
        hp->num_of_pages = 1;
        hp->rpo = 0;
        pwrite(fd, hp, sizeof(H_P), 0);
        free(hp);
        hp = load_header(0);
        return 0;
    }
    fd = open(pathname, O_RDWR|O_SYNC);
    if (fd > 0) {
        //printf("Read Existed File\n");
        if (sizeof(H_P) > pread(fd, hp, sizeof(H_P), 0)) {
            return -1;
        }
        off_t r_o = hp->rpo;
        rt = load_page(r_o);
        return 0;
    }
    else return -1;
}

//reset page. reset all values to 0. regardless of page's type.  
//reset한 페이지를 reset하고자 하는 page의 off에 적는다. 기존 page 값을 reset하는 게 아니라 reset한 page를 그 자리에 저장. 
//적고 reset page는 free 시킨다. it is not free page. just reset for storing new data in page
void reset(off_t off) {
    page * reset;
    reset = (page*)calloc(1, sizeof(page));
    reset->parent_page_offset = 0;
    reset->is_leaf = 0;
    reset->num_of_keys = 0;
    reset->next_offset = 0;
    pwrite(fd, reset, sizeof(page), off);
    free(reset);
    return;
}

//reset page and set free page. but it is not controlled by header page. 
//it is used to alloc new page
void freetouse(off_t fpo) {
    page * reset;
    reset = load_page(fpo);
    reset->parent_page_offset = 0;
    reset->is_leaf = 0;
    reset->num_of_keys = 0;
    reset->next_offset = 0;
    pwrite(fd, reset, sizeof(page), fpo);
    free(reset);
    return;
}
//reset page and set free page and controlled by header page.
//it is used to delete page. 
void usetofree(off_t wbf) {
    page * utf = load_page(wbf);
    utf->parent_page_offset = hp->fpo;
    utf->is_leaf = 0;
    utf->num_of_keys = 0;
    utf->next_offset = 0;
    pwrite(fd, utf, sizeof(page), wbf);
    free(utf);
    hp->fpo = wbf;
    pwrite(fd, hp, sizeof(H_P), 0);
    free(hp);
    hp = load_header(0);
    return;
}

off_t new_page() {
    off_t newp;
    page * np;
    off_t prev;
    if (hp->fpo != 0) {
        newp = hp->fpo;
        np = load_page(newp);
        if (np == NULL) {
            perror("Failed to load free page.");
            exit(EXIT_FAILURE);
        }
        //free page's parent_page_offset points to next free page.
        hp->fpo = np->parent_page_offset;
        //store data to disk. 
        pwrite(fd, hp, sizeof(H_P), 0);
        free(hp);
        hp = load_header(0);
        //header's info is changed. so update data to disk and reload data from disk. => header info in memory maintain cur state
        free(np);
        freetouse(newp);
        return newp;
    }
    //change previous offset to 0 is needed
    newp = lseek(fd, 0, SEEK_END);
    //if (newp % sizeof(page) != 0) printf("new page made error : file size error\n");
    reset(newp);
    //if header doesn't have free page then alloc new page and num of pages increases. 
    hp->num_of_pages++;
    pwrite(fd, hp, sizeof(H_P), 0);
    free(hp);
    hp = load_header(0);
    return newp;
}



int cut(int length) {
    if (length % 2 == 0)
        return length / 2;
    else
        return length / 2 + 1;
}


//! disk에 쓰기 작업
//todo disk에 씀  
void start_new_file(record rec) {
    page * root;
    off_t ro;
    ro = new_page();
    rt = load_page(ro);
    hp->rpo = ro;
    pwrite(fd, hp, sizeof(H_P), 0);
    free(hp);
    hp = load_header(0);
    rt->num_of_keys = 1;
    //leaf page.
    rt->is_leaf = 1;
    rt->records[0] = rec;
    pwrite(fd, rt, sizeof(page), hp->rpo);
    free(rt);
    rt = load_page(hp->rpo);
    //printf("new file is made\n");
}

//! next 반영한 상태. 
page * find_leaf(off_t root, int key){
    page * c=load_page(root);
    if(c==NULL){
        return c;
    }
    //root부터 leaf로 쭉 내려가
    while(!c->is_leaf){
        int i=0;
        //internal node라서 일단 key0보다 작으면 next_offset으로 가야 함.
        if(key < c->b_f[0].key){
            off_t child_offset = c->next_offset;
            free(c);
            c = load_page(child_offset);
        }
        //key0보단 크면 이제 key1,key2, key[num_of_keys-1]까지 확인. 
        //[next_offset][v0][p0][v1][p1]...[vn-1][pn-1]
        //v[n-2]보다 크면 i는 n-1. while문 나가. c=[pn-1]이 가리키고 있는 page 
        else{
            while(i < c->num_of_keys - 1 && key >= c->b_f[i].key){
                i++;
            }
            off_t child_offset = c->b_f[i].p_offset;
            free(c);
            c = load_page(child_offset);
        }
        if(c==NULL) return NULL;
    }
    return c;
}

char * db_find(int64_t key) {
    int i=0;
    //c에서 key가 존재하는 page return
    page * c=find_leaf(hp->rpo,key);
    if(c==NULL) return NULL;
    //page에 존재하는 key를 돌면서 일치하는 지 확인. leaf에서 온 거니까 무조건 일치 확인
    //leaf는 record에 저장 
    //[next_offset][v0][p0][v1][p1]...[vn-1][pn-1]
    //v0부터 vn-1까지 보면서 비교... 
    for (i = 0; i < c->num_of_keys; i++) {
        if (c->records[i].key == key) {
            //일치하는 거 찾으면 해당 value 복사
            char *value = strdup(c->records[i].value);
            free(c);
            return value;
        }
    }
    //못 찾으면 null return 
    free(c);
    return NULL;

}

//! next 반영한 상태. 이건 leaf node의 offset을 찾는 역할만.
off_t find_offset(int64_t key){
    off_t current_offset = hp->rpo; // 루트 노드의 오프셋
    page *c = load_page(current_offset);
    if(c == NULL) return 0;
    //[next_offset][v0][p0][v1][p1]...[vn-1][pn-1]
    while(!c->is_leaf){
        int i = 0;
        if(key < c->b_f[0].key){
            // 키가 첫 번째 키보다 작으면 next_offset을 따라감
            off_t child_offset = c->next_offset;
            free(c);
            c = load_page(child_offset);
            current_offset = child_offset; // 현재 오프셋 업데이트
        }
        else{
            // 키가 첫 번째 키보다 크거나 같으면 적절한 p_offset을 따라감
            while(i < c->num_of_keys - 1 && key >= c->b_f[i].key){
                i++;
            }
            off_t child_offset = c->b_f[i].p_offset;
            free(c);
            c = load_page(child_offset);
            current_offset = child_offset; // 현재 오프셋 업데이트
        }
        if (c == NULL) return 0;
    }

    free(c);
    return current_offset;
}





//! record 만들기만 해서 여기선 disk에 쓰기 안 해
record make_record(int64_t key,  char *value) {
    record new_record;
    new_record.key = key;
    strncpy(new_record.value, value, sizeof(new_record.value) - 1);
    new_record.value[sizeof(new_record.value) - 1] = '\0';
    return new_record;
}

//! offset 반영. parent b_f 돌면서 pointer가 left_offset과 동일한 index return
int get_left_index(page* parent, off_t left_offset) {
    //parent에서 left가 존재하는 index를 반환. 
    int left_index = 0;
    // in-memory에서의 get_left_index를 page 구조에 맞게 변형...
    if(parent->next_offset==left_offset) return -1;
    while (left_index < parent->num_of_keys &&  parent->b_f[left_index].p_offset != left_offset) {
        left_index++;
    }
    if(left_index==parent->num_of_keys) printf("out of range!!!!\n");
    return left_index;
}



//! offset 반영. left, right offset 반영해서 disk에 쓰기까지 완. 
//* DISK에 쓰는 행위 포함
page * insert_into_new_root(page* left, off_t left_offset, int key, page* right, off_t right_offset) {
    //새 page 생성
    off_t new_root_offset = new_page();
    page* root = load_page(new_root_offset);
    if (root == NULL) {
        perror("Failed to load new root page.");
        exit(EXIT_FAILURE);
    }
    //root가 split되면서 새로운 root가 생긴 거라서 leaf 아님. key는 새로 받는 거 하나
    //next, v0 p0 v1 p1 
    root->is_leaf = 0;
    root->num_of_keys = 1;
    root->next_offset = left_offset;
    root->b_f[0].key = key;
    root->b_f[0].p_offset = right_offset;

    //자식이 된 root들한테 부모라고 설정
    left->parent_page_offset = new_root_offset;
    right->parent_page_offset = new_root_offset;
     
    pwrite(fd,root, sizeof(page),new_root_offset);
    hp->rpo = new_root_offset;
    pwrite(fd, hp, sizeof(H_P), 0);
    
    free(root);

    return load_page(new_root_offset);
}

//! internal에 넣는 경우... offset 반영 완. page의 left_index+1에 new key랑 new offset 삽입
//TODO root가 바뀐 거 아니고 그냥 page node의 값들만 바뀐 거라 page만 저장해주면 됨 (완료)
page * insert_into_node(page * root, page * page, int left_index, int64_t new_key, off_t new_offset) {
    int i;
    
    //n이라는 page의 v(left_index+1)에 key, p(left_index+1)에 new_node 삽입 
    //next v0 p0 v1 p1 구조에서 left_index가 1이라면 next v0 p0 v1 p1 key new_node의 offset
    //left_index가 0이라면 next v0 p0 key new_node v2 p2
    //new_node는 origin보단 커야 해서 leftmost가 될 수 없음 
    //num key부터 left_index+1까지 밀어
    if(left_index==-1){
        for (i =page->num_of_keys-1; i >=0; i--) {
            page->b_f[i+1].key=page->b_f[i].key;
            page->b_f[i+1].p_offset=page->b_f[i].p_offset;
        }
        page->b_f[0].p_offset=page->next_offset;
        page->next_offset=new_offset;
        page->b_f[0].key=new_key;
        return page;
    }
    else{
    for (i =page->num_of_keys-1; i > left_index; i--) {
       page->b_f[i+1].key=page->b_f[i].key;
       page->b_f[i+1].p_offset=page->b_f[i].p_offset;
    }
    page->b_f[left_index+1].key =new_key;
    page->b_f[left_index+1].p_offset = new_offset;
    page->num_of_keys++;
    return page;}
}

//! leaf에 pair 넣어. key로 넣을 자리 탐색 disk에 쓰는 건 caller에서
//TODO root가 바뀐 거 아니고 그냥 leaf node의 값들만 바뀐 거라 leaf만 저장해주면 됨 (완료)
page * insert_into_leaf(page * leaf, int64_t key, record pair){
    int i, insertion_point=0;
    //leaf node에서 key보다 크거나 같은 값 찾으면 멈춰서 넣어. 
    while (insertion_point < leaf->num_of_keys && leaf->records[insertion_point].key < key)
        insertion_point++;
    //현재 마지막 key 번호에서 들어가야 하는 곳까지 다 옆으로 밀어...     
    for (i = leaf->num_of_keys; i > insertion_point; i--) {
        leaf->records[i] = leaf->records[i - 1];
    }
    //넣을 곳 찾았으니 다 넣으면 끝~ 
    leaf->records[insertion_point]=pair;
    leaf->num_of_keys++;
    return leaf;
}

//! offset 반영... root 호출, index 호출, insert_node, node_split 호출
//todo root는 callee에서, insert_node는 지금 disk에 안 쓰고 있음. disk에 언제 쓸 지 확인, split도 disk 안 쓰고 있음 
page * insert_into_parent(page * root, page * origin_child, off_t origin_child_offset, int64_t key, page* new_child, off_t new_child_offset) {
    //left는 기존 node, right는 새로운 node, key는 parent에 들어갈 방향키
    int left_index;
    page * parent;

    
    off_t parent_offset=origin_child->parent_page_offset;
    parent =load_page(parent_offset);
    if (parent == NULL) //기존 node가 root였기 때문에 root에 넣는다.
    //todo root에서 disk에 썼음
        return insert_into_new_root(origin_child,origin_child_offset,key,new_child,new_child_offset);

    //parent에서 기존 노드의 index를 가져와 
    //next origin_key origin_node key new_node 형식으로 가야 함 
    //left_index는 0. 
    left_index = get_left_index(parent, origin_child_offset);

    //부모에 자리 있으면 parent record의 v(left_index+1)에 key, p(left_index+1)에 new_node 삽입 
    if (parent->num_of_keys < LEAF_MAX - 1){
        parent= insert_into_node(root, parent, left_index, key, new_child_offset);
        pwrite(fd,parent,sizeof(page),parent_offset);
        free(parent);
        return root;
    }

    //todo 여기서 parent 해제는 어떻게 해줘야 하지? 
    return insert_into_node_after_splitting(root, origin_child->parent_page_offset,parent, left_index, key, new_child_offset);
}





//! left_index가 old_node에서 key, right 들어갈 자리. key는 leftmost가 될 수 없다. 새로 들어온 게 부모의 방향키보다 크거나 같아야 하는데
//! 같으면 중복이라 insert 안 되고 크면 leftmost 불가능임. leaf는 dense라서 무조건 방향키랑 동일한 값을 갖고 있어야 함. 
//! 새 page 만들고, old page의 left_index+1에 new key랑 new offset 넣고 남은 건 new, old page에 나눠 저장 
page * insert_into_node_after_splitting(page * root, off_t old_node_offset, page * old_node, int left_index, int64_t new_key, off_t new_pointer) {

    int i, j, split;
    int64_t k_prime;
    int64_t temp_keys[INTERNAL_MAX + 1]; 
    off_t temp_pointers[INTERNAL_MAX + 1];
    off_t temp_parent[INTERNAL_MAX+1];
    off_t temp_next_offset;

    off_t new_node_offset=new_page();
    page *new_node = load_page(new_node_offset);
    if (new_node == NULL) {
        perror("Failed to create new internal node.");
        exit(EXIT_FAILURE);
    }
    new_node->parent_page_offset=old_node->parent_page_offset;
    new_node->num_of_keys=0;
    new_node->is_leaf=0;
     
    memset(new_node->records, 0, sizeof(new_node->records));
    pwrite(fd,new_node,sizeof(page),new_node_offset);
    free(new_node);
    new_node=load_page(new_node_offset);

    //old node의 key, pointer 저장. 
    //old node의 leftmost보다 작은 값은 next_offset에 저장. 
     for (i = 0, j = 0; i < old_node->num_of_keys; i++, j++) {
        if (j == left_index + 1) j++;
        temp_pointers[j] = old_node->b_f[i].p_offset;
        temp_keys[j]=old_node->b_f[i].key;

    }

    temp_pointers[left_index+1] = new_pointer;
    temp_keys[left_index+1] = new_key;

    split = cut(INTERNAL_MAX);
    //0~split-1까지 저장... 
    for (i = 0; i < split; i++) {
        old_node->b_f[i].p_offset = temp_pointers[i];
        old_node->b_f[i].key = temp_keys[i];
        old_node->num_of_keys++;
    }
    //old_node->next_offset은 유지...
    //split+1~internal_max까지는 new에 저장 
    int k = 0;
    for (i=split+1; i <=INTERNAL_MAX ; i++, k++) {
        new_node->b_f[k].p_offset = temp_pointers[i];
        new_node->b_f[k].key = temp_keys[i];
        new_node->num_of_keys++;
    }
    page * child=load_page(old_node->next_offset);
    child->parent_page_offset=old_node_offset;
    free(child);
    child=load_page(new_node->next_offset);
    child->parent_page_offset=new_node_offset;
    free(child);
    for(int i=0; i<old_node->num_of_keys; i++){
        child=load_page(old_node->b_f[i].p_offset);
        child->parent_page_offset=old_node_offset;
        free(child);
    }for(int i=0; i<new_node->num_of_keys; i++){
        child=load_page(new_node->b_f[i].p_offset);
        child->parent_page_offset=new_node_offset;
        free(child);
    }
    //new의 next는 temp[split]의 pointer (부모로 올릴 node가 갖고 있던 pointer)
    new_node->next_offset = temp_pointers[split];
    k_prime = temp_keys[split];
    
    pwrite(fd,old_node,sizeof(page), old_node_offset);
    pwrite(fd,new_node, sizeof(page),new_node_offset);

    return insert_into_parent(root, old_node, old_node_offset, k_prime, new_node, new_node_offset);
}

//! leaf page에 pair 삽입, key로 삽입 장소 탐색 
page * insert_into_leaf_after_splitting(page * root, page * leaf, int64_t key, record pair) {
    
    int split, i,j, insertion_index;
    int64_t new_key;
    record temp_records[LEAF_MAX+1];
    //leaf 넘쳐서 split해야 하니 새로운 leaf page 추가

    off_t new_leaf_offset=new_page();
    page * new_leaf=load_page(new_leaf_offset);
    off_t leaf_offset=find_offset(leaf->records[0].key);
    if (new_leaf == NULL) {
        perror("Failed to load new leaf page.");
        exit(EXIT_FAILURE);
    }
    new_leaf->parent_page_offset = leaf->parent_page_offset; 
    new_leaf->is_leaf = 1;            
    new_leaf->num_of_keys = 0;        
    new_leaf->next_offset = leaf->next_offset;
    leaf->next_offset=new_leaf_offset;

    memset(leaf->records, 0, sizeof(new_leaf->records));

    pwrite(fd, new_leaf, sizeof(page), new_leaf_offset);
    pwrite(fd, leaf, sizeof(page), leaf_offset);

    free(new_leaf);
    new_leaf = load_page(new_leaf_offset);

    if (new_leaf == NULL) {
        perror("Failed to create new leaf node.");
        exit(EXIT_FAILURE);
    }

    insertion_index = 0;
    // 기존 leaf node의 key 계속 돌면서 들어 가 자리 찾는다
    while (insertion_index < LEAF_MAX  && leaf->records[insertion_index].key < key)
        insertion_index++;

    //기존 leaf node의 record들을 임시 record에 저장하는데, 들어 갈 자리는 비워놓고 저장.
    for (i = 0, j = 0; i < leaf->num_of_keys; i++, j++) {
        if (j == insertion_index) j++; 
        temp_records[j] = leaf->records[i];
    }

    //아까 비워둔 자리에 새로 insert하는 record 저장
    temp_records[insertion_index] = pair;
    //반 자를 자리 계산
    split = cut(LEAF_MAX); 
    //기존 leaf node 초기화하고 다시 record 저장해야 해서 0으로 설정하고 절반까지 계속 채워
    leaf->num_of_keys = 0;
    //0~split-1 까지 채워 
    for (i = 0; i <split; i++) {
        leaf->records[i] = temp_records[i];
        leaf->num_of_keys++;
    }
    //split~leaf_max까지 채워. old는 leaf max-1까지 있었지만 temp엔 하나 추가돼서 leaf max까지 있음 
    for (i=split; i <=LEAF_MAX ; i++) {
        new_leaf->records[new_leaf->num_of_keys++] = temp_records[i];
    }
    
    pwrite(fd,leaf,sizeof(page),leaf_offset);        
    pwrite(fd,new_leaf, sizeof(page),new_leaf_offset);

    //새 leaf node의 첫 번째 key가 두 leaf node의 방향key가 된다.
    new_key = new_leaf->records[0].key; 
    //부모에 leaf, new_leaf, new_key 보내. 
    //todo insert_into_parent에서 disk에 썼음...
    root= insert_into_parent(root, leaf,leaf_offset, new_key, new_leaf,new_leaf_offset);
    
    free(new_leaf);

    return root;
}

off_t common_parent(page * left, off_t left_offset, page * right, off_t right_offset){
    off_t leftparent_offset=left->parent_page_offset;
    off_t rightparent_offset=right->parent_page_offset;
    page * leftparent=load_page(leftparent_offset);
    page * rightparent=load_page(rightparent_offset);
    while(leftparent_offset!=rightparent_offset){
       leftparent_offset=leftparent->parent_page_offset;
       leftparent=load_page(leftparent_offset);
       rightparent_offset=rightparent->parent_page_offset;
       rightparent=load_page(rightparent_offset);
    }
    if(leftparent_offset==rightparent_offset){
        return leftparent_offset;
    }
    return -1;
}

//! return root할 때 다 disk에 저장하고, root 다시 load해서 보내야 함. 
page * leaf_right_rotation(page * root,off_t leaf_offset, page * leaf, off_t right_sibling_offset, page* right_sibling,  record pair){
    //todo leaf, right node에 값 저장...
    int64_t key=pair.key;
    record temp_records[LEAF_MAX+1];
    int insertion_index = 0;
    // 기존 leaf node의 key 계속 돌면서 들어 가 자리 찾는다
    while (insertion_index < LEAF_MAX  && leaf->records[insertion_index].key < key)
        insertion_index++;

    //기존 leaf node의 record들을 임시 record에 저장하는데, 들어 갈 자리는 비워놓고 저장.
    int i,j;
    for (i = 0, j = 0; i < leaf->num_of_keys; i++, j++) {
        if (j == insertion_index) j++; 
        temp_records[j] = leaf->records[i];
    }
    temp_records[insertion_index] = pair;

    leaf->num_of_keys = 0;
    //0~num_of_keys-1까지 채워
    for (i = 0; i <LEAF_MAX; i++) {
        leaf->records[i] = temp_records[i];
        leaf->num_of_keys++;
    }

    for (i = right_sibling->num_of_keys; i > 0; i--) {
        right_sibling->records[i] = right_sibling->records[i - 1];
    }
    right_sibling->records[0]=temp_records[leaf->num_of_keys];
    right_sibling->num_of_keys++;

    pwrite(fd,leaf,sizeof(page),leaf_offset);        
    pwrite(fd,right_sibling, sizeof(page),right_sibling_offset);

    //todo leaf, right 부모 조정
    off_t parent=common_parent(leaf, leaf_offset,right_sibling,right_sibling_offset);
    if(parent==-1) printf("공통 부모 없음\n");
    page * common_p=load_page(parent);
    insertion_index=0;
    while(insertion_index<common_p->num_of_keys&&common_p->b_f[insertion_index].key<key){
        insertion_index++;
    }
    common_p->b_f[insertion_index].key=right_sibling->records[0].key;
    
    pwrite(fd,common_p,sizeof(page),parent);
    return root;
}




//! record 만든 거 start_new_file에서 저장 
//todo disk 쓰기 작업 있음!! 다른 것도 다 disk 쓰기 작업 넣어야 함...
int db_insert(int64_t key, char * value) {
    off_t r=hp->rpo;
    page * root=load_page(r);
    // ignore duplication
    if(db_find(key)!=NULL) return -1;
    //if tree does not exist then start a new file.
    //여기서 만들어진 record는 start_new_file이나 insert 그 외 함수들에서 page에 저장된 후 pwrite될 것...
    record pair=make_record(key,value);
    //start a new file and return 0... and insert pair(new record)
    if(r==0){
        start_new_file(pair);
        return 0;
    }
    //if tree already exists then find proper place(node)
    page * leaf=find_leaf(r,key);
    off_t leaf_offset=find_offset(key);
    if(leaf==NULL){
        free(leaf);
        return -1;
    }

    //제일 먼저 그 node에 넣을 공간 있는 지 확인
    //공간 있으면 leaf node니까 leaf에 넣기...
    if(leaf->num_of_keys<LEAF_MAX-1){
        leaf=insert_into_leaf(leaf,key,pair);
        //new page 함수 참고...
        pwrite(fd, leaf, sizeof(page),leaf_offset);
        free(leaf);
        return 0;
    }

    
    //공간 없으면 왼쪽, 오른쪽 sibling 확인해서 공간 있는 지 확인. 
    
    page * right_sibling = load_page(leaf->next_offset);
    off_t right_sibling_offset = leaf->next_offset;

    //todo 왼쪽 존재하면 key rotation 가능한 지 확인 가능하면 수행하고 return, 
    //todo 불가능하고 오른쪽 존재하면 key rotation 가능여부 확인. 가능하면 수행하고 return, 불가능이면 split하러...
    //todo insert node split, insert leaf split에서도 split 전에 key rotation 가능여부 확인해야 함. 
   
    if(right_sibling!=NULL&&right_sibling->num_of_keys<LEAF_MAX-1){
        root=leaf_right_rotation(root, leaf_offset, leaf, right_sibling_offset, right_sibling, pair);
        pwrite(fd,root,sizeof(page),r);
        free(right_sibling);
        return 0;
    }
    
    //공간도 없고 key rotation도 안 되면 split... 
    else{
        root=insert_into_leaf_after_splitting(root, leaf,key,pair);
        pwrite(fd,root,sizeof(page),hp->rpo);
    }
    free(leaf);
    return 0;
}



//todo disk에 쓰기 작업 완료. 
page * remove_entry_from_node(page * n, off_t n_offset,int key) {
    int i, num_pointers;
    //leaf, internal 나눠서 제거해야 함
    if(n->is_leaf){
        i=0;
        //지워질 record가 있는 page에서 record 있는 자리 찾고 뒤에서부터 하나씩 땡겨.
        while(n->records[i].key!=key) i++;
        for(++i; i<n->num_of_keys; i++){
            n->records[i-1]=n->records[i];
        }
        n->num_of_keys--;
        for(i=n->num_of_keys; i<LEAF_MAX; i++){
            n->records[i].key=-1;
            n->records[i].value[0]='\0';
        }
        pwrite(fd,n,sizeof(page),n_offset);
        return n;
    }
    else{
        //n이 internal일 때는 지워지는 자식이 leftmost인지 아닌지가 중요하다... next offset 때문에 근데 나는 key값만 알기에... 다른 방법 필요
        //재분배와 달리 merge해서 한 node가 사라질 때만 이 함수가 호출되니까 이 함수 호출 시점엔 child page가 삭제되어 있음. 즉 child 가리키는 pointer가 NULL
        //노노 아직 가리키는 pointer는 유효함. dangling pointer가 된 거지... 그니까 load page해서 NULL인지를 확인ㄱㄱ 
        if(load_page(n->next_offset)==NULL){
            //만약 지운 애가 leftmost였다면 next offset에 0번째 pointer 저장하고 나머지는 앞으로 땡기면 됨 
             n->next_offset=n->b_f[0].p_offset;
            for(++i; i<n->num_of_keys; i++){
                n->b_f[i-1].key=n->b_f[i].key;
                n->b_f[i-1].p_offset=n->b_f[i].p_offset;
            }
            n->num_of_keys--;
            for(i=n->num_of_keys; i<INTERNAL_MAX; i++){
                n->b_f[i].key=-1;
                n->b_f[i].p_offset=-1;
            }
            pwrite(fd,n,sizeof(page),n_offset);
            return n;
        }
        //지운 애가 leftmost가 아니라면 그냥 알아서 자리 찾고... 한 칸씩 땡기면 됨. 
        //만약 지운 애가 두 번째였다면 i가 0이 될 거고... 그럼 next offset 빼고 다 바뀌겠지요. 원하는 거 만족. 
        else{
            i=0;
            while(n->b_f[i].key!=key) i++;
            for(++i; i<n->num_of_keys; i++){
                n->b_f[i-1].key=n->b_f[i].key;
                n->b_f[i-1].p_offset=n->b_f[i].p_offset;
            }
            n->num_of_keys--;
            for(i=n->num_of_keys; i<INTERNAL_MAX; i++){
                n->b_f[i].key=-1;
                n->b_f[i].p_offset=-1;
            }
            pwrite(fd,n,sizeof(page),n_offset);
            return n;
        }
    }
}


//이 함수는 삭제 작업 수행할 때 leaf에서 지워지고 재귀적으로 root까지 타고 가서 root에서도 지워야 할 일이 생길 때
//그 때 호출됨. 그거 아니면 처음부터 root가 leaf인 상태일 때... 
//todo disk에 쓰기 작업 완료 근데 애매쓰
page * adjust_root(page * root) {
    
    //root에서 지운 후에 key 남아 있으면 ㄱㅊ 진행시켜
    off_t root_offset=hp->rpo;
    if (root->num_of_keys > 0)
        return root;

    //root에서 지운 후에 key가 안 남아 있다면 일단
    //root가 자식이 있었다면?? 첫 번째 자식한테 물려줘 root 직위를
    
    if (!root->is_leaf) {
        //todo new_root에 root의 첫 번째 자식 덮어 씌우고, 첫 번째 자식 자리에 update한 new_root 저장하고, 헤더에 root를 첫 번째 자식으로 지정
        page * new_root=load_page(root->next_offset);
        new_root->parent_page_offset= 0;
        hp->rpo = root->next_offset;
        pwrite(fd,new_root, sizeof(page),root->next_offset);
        usetofree(root_offset);
        pwrite(fd, hp, sizeof(H_P), 0);
        free(root);
        free(new_root);
        new_root=load_page(root->next_offset);
        return new_root;
    }

    //만약 자식이 없었다면 root를 없애

    else{
        hp->rpo=0;
        root=NULL;
        usetofree(root_offset);
        pwrite(fd, hp, sizeof(H_P), 0);
        return root;
    }
}


/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
 */

page * coalesce_nodes(page * root, page * n, off_t n_offset, page * neighbor, off_t neighbor_offset, int neighbor_index, int k_prime) {
    //n이 leftmost일 때, neighbor가 leftmost일 때, leaf, internal일 때 고려해야 함 
    //근데 swap해서 무조건 n이 오른쪽에 가게 해놨으니... leaf, internal만 고려하면 됨. 
    //만약 오른쪽 형제와 merge하는 거면 n이 right, neighbor가 left로 고정. 
    int i, j, neighbor_insertion_index, n_end;
    page * tmp;

    //n이 leftmost일 때. n과 neighbor를 swap해서 n을 오른쪽으로 보내! 
    if (neighbor_index == -2) {
        tmp = n;
        n = neighbor;
        neighbor = tmp;
    }
    //왼쪽으로 merge할 거라서 neighbor의 마지막에 들어간다. 
    neighbor_insertion_index = neighbor->num_of_keys;

    
    if (!n->is_leaf) {
        //n이 internal이라면, 일단 neigbor의 가장 오른쪽 key가 빈 상태니까 부모의 방향키인인 k_prime 넣어주기
        //그리고 n에서 하나씩 갖고 와...

        neighbor->b_f[neighbor_insertion_index].key = k_prime;
        neighbor->num_of_keys++;
        n_end = n->num_of_keys;

        for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
            neighbor->b_f[i].key = n->b_f[j].key;
            neighbor->b_f[i].p_offset = n->b_f[j].p_offset;
            neighbor->num_of_keys++;
            n->num_of_keys--;
        }
        //internal 수정한 거라 부모 다 바꿔줘야 함. n의 자식들에게 부모가 neighbor로 바꼈다고 알려줘. 
        page * child;
        for(int i=0; i<neighbor->num_of_keys; i++){
            child=load_page(neighbor->b_f[i].p_offset);
            child->parent_page_offset=neighbor_offset;
            free(child);
            pwrite(fd,child,sizeof(page),neighbor->b_f[i].p_offset);
        }
        pwrite(fd,neighbor,sizeof(page),neighbor_offset);
    }

    
    else {
        for (i = neighbor_insertion_index, j = 0; j < n->num_of_keys; i++, j++) {
            neighbor->records[i]=n->records[j];
            neighbor->num_of_keys++;
        }
        neighbor->next_offset=n->next_offset;
        
        pwrite(fd,neighbor,sizeof(page),neighbor_offset);
    }
    off_t temp_n_parent=n->parent_page_offset;
    //여기서 n 초기화
    usetofree(n_offset);
    root = delete_entry(root, temp_n_parent, k_prime);
    return root;
}


/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum
 */
page * redistribute_nodes(page * root, page * n, off_t n_offset, page * neighbor, off_t neighbor_offset, int neighbor_index, int k_prime_index, int k_prime) {  
    //여기선 node가 삭제된 게 아니라서 직속 부모 외에는 key값 수정 안 해줘도 됨. 재귀적으로 수행할 필요 없음. 
    int i;
    page * tmp;
    page * parent=load_page(n->parent_page_offset);
    /* Case: n has a neighbor to the left. 
     * Pull the neighbor's last key-pointer pair over
     * from the neighbor's right end to n's left end.
     */
    //n이 leftmost가 아닐 때. 
    if (neighbor_index != -2) {
        //n이 leaf라면 n을 오른쪽으로 한 칸씩 밀고, neighbor의 마지막 record 갖고 오고 parent의 방향키를 n의 0번째 key로
        if(n->is_leaf){
            for(int i=n->num_of_keys; i>0; i--){
                n->records[i]=n->records[i-1];
            }
            n->records[0]=neighbor->records[neighbor->num_of_keys-1];
            for(int i=n->num_of_keys-1; i<LEAF_MAX; i++){
                neighbor->records[i].key=-1;
                neighbor->records[i].value[0]='\0';
            }
            //이 때는 바꾼 후 n의 0번 째가 필요
            parent->b_f[k_prime_index].key=n->records[0].key;
        }
        //n이 internal이라면? 
        else{
            for(int i=n->num_of_keys; i>0; i--){
                n->b_f[i].key=n->b_f[i-1].key;
                n->b_f[i].p_offset=n->b_f[i].p_offset;
            }
            n->b_f[0].p_offset=n->next_offset;
            n->b_f[0].key=k_prime;
            parent->b_f[k_prime_index].key=neighbor->b_f[neighbor->num_of_keys-1].key;
            n->next_offset=neighbor->b_f[neighbor->next_offset-1].p_offset;
            for(int i=n->num_of_keys-1; i<INTERNAL_MAX; i++){
                n->b_f[i].key=-1;
                n->b_f[i].p_offset=-1;
            }
        }
    }

    /* Case: n is the leftmost child.
     * Take a key-pointer pair from the neighbor to the right.
     * Move the neighbor's leftmost key-pointer pair
     * to n's rightmost position.
     */
    //n이 leftmost일 때
    else {  
        //n이 leaf라면
        if (n->is_leaf) {
            n->records[n->num_of_keys]=neighbor->records[0];
            for(int i=0; i<neighbor->num_of_keys-1; i++){
                neighbor->records[i]=neighbor->records[i+1];
            }
            for(int i=n->num_of_keys-1; i<LEAF_MAX; i++){
                neighbor->records[i].key=-1;
                neighbor->records[i].value[0]='\0';
            }
            //이 때는 n을 바꾼 후 0번째가 필요함.
            parent->b_f[k_prime_index].key=neighbor->records[0].key;
        }
        //n이 internal일 때
        else {
            
            n->b_f[n->num_of_keys].key = k_prime;
            n->b_f[n->num_of_keys].p_offset = neighbor->next_offset;
            //이 때는 바꾸기 전의 neighbor 0 번째가 필요함... 바꾸러 갈 때 주고 가~
            parent->b_f[k_prime_index].key=neighbor->b_f[0].key;
            neighbor->next_offset=neighbor->b_f[0].p_offset;
            for(int i=0; i<neighbor->num_of_keys-1; i++){
                neighbor->b_f[i].key=neighbor->b_f[i+1].key;
                neighbor->b_f[i].p_offset=neighbor->b_f[i].p_offset;
            }
            for(int i=neighbor->num_of_keys-1; i<INTERNAL_MAX; i++){
                neighbor->b_f[i].key=-1;
                neighbor->b_f[i].p_offset=-1;
            }
        }
    }
    n->num_of_keys++;
    neighbor->num_of_keys--;
    pwrite(fd,n,sizeof(page),n_offset);
    pwrite(fd,neighbor,sizeof(page),neighbor_offset);
    pwrite(fd,parent,sizeof(page),n->parent_page_offset);
    return root;
}


/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
 */
page * delete_entry(page * root, off_t n_offset, int key) {
    //merge, redistribute 둘 다 무조건 부모 동일한 형제만 검사한다고 가정. 

    int min_keys;
    page * neighbor;
    int neighbor_index, k_prime_index;
    int64_t k_prime;
    int capacity;
    page * n=load_page(n_offset);
    page * parent=load_page(n->parent_page_offset);
    n = remove_entry_from_node(n, n_offset, key);
 
    if (n == root) 
        return adjust_root(root);

    min_keys = n->is_leaf ? cut(LEAF_MAX-1) : cut(INTERNAL_MAX);


    if (n->num_of_keys >= min_keys)
        return root;
    
    //! n이 leaf든 internal이든 이건 부모 기준으로 나누는 거라서 case 안 나눠도 됨
    //이건 왼쪽 형제 보는 코드

    //아 여기서 말하는 neighbor index는 n이 leftmost라면 -1이어야 함 
    neighbor_index =get_left_index(parent, n_offset )-1;
    //n이 leftmost라면 neighbor_index는 -2... neighbor가 leftmost라면 neighbor_most는 -1
    //neighbor index가 -1이라면 k_prime은 0번째, -2여도 0 번째, 그 외는 neighbor+1
    k_prime_index = neighbor_index==-2 ? 0 : neighbor_index+1;
    k_prime = parent->b_f[k_prime_index].key;
    //n이 leftmost라면 neighbor는 0번째, neighbor가 leftmost라면 neighbor는 next offset, 그 외는 neighbor index
    off_t neighbor_offset=parent->b_f[neighbor_index].p_offset;
    if(neighbor_index==-2) neighbor=load_page(parent->b_f[0].p_offset);
    else if(neighbor_index==-1) neighbor=load_page(parent->next_offset);
    else neighbor = load_page(parent->b_f[neighbor_index].p_offset);
    
    capacity = n->is_leaf ? LEAF_MAX : INTERNAL_MAX;


    if (neighbor->num_of_keys + n->num_of_keys < capacity)
        return coalesce_nodes(root, n, n_offset,neighbor, neighbor_offset,neighbor_index, k_prime);
    //왼쪽이랑 안 된다면 오른쪽 형제 봐야 함. 근데 이 때 merge 안 돼서 재분배한다고 해도 무조건 왼쪽 형제랑 수행하도록... 
    //어차피 왼, 오 둘 다 merge 안 되는 거면 둘 다 재분배는 가능함. 그니까 그냥 왼쪽 형제로 통일. 어차피 n이 leftmost일 때는 재분배 함수 안에서 처리함 
    else{
        page * right_neighbor;
        int right_neighbor_index, right_k_prime_index;
        int64_t right_k_prime;
        //만약 n이 rightmost라면 neighbor가 out of range... 이 경우엔 이미 왼쪽 형제랑 비교한 상황이라서 무시해도 됨. 
        //n이 leftmost인 상황도 이미 오른쪽 형제랑 비교한 상황이라 무시해도 됨
        //n이 leftmost일 때는 0번째랑 비교 완료... n이 rightmost일 때는 오른쪽 형제 없고, 왼쪽 형제랑은 이미 비교한 상태
        right_neighbor_index =get_left_index( parent,n_offset )+1;
        if(right_neighbor_index==parent->num_of_keys || right_neighbor_index==0) return  redistribute_nodes(root, n,n_offset, neighbor, neighbor_offset,neighbor_index, k_prime_index, k_prime);
        //여기서 나올 수 있는 right_neighbor_index의 값은 1부터 num_of_keys-1까지... 
        right_k_prime_index = right_neighbor_index;
        right_k_prime = parent->b_f[k_prime_index].key;
        right_neighbor=load_page(parent->b_f[right_neighbor_index].p_offset);
        off_t right_neighbor_offset=parent->b_f[right_neighbor_index].p_offset;
        if (right_neighbor->num_of_keys + n->num_of_keys < capacity) 
            //이러면 무조건 n이 왼쪽으로 가서... 저 함수 내부에서 case 나눠서 swap하기가 힘듦... 그냥 여기서 swap해서 보내는 걸로.
            return coalesce_nodes(root, right_neighbor, right_neighbor_offset, n, n_offset,right_neighbor_index, right_k_prime);
        else return redistribute_nodes(root, n,n_offset, neighbor, neighbor_offset, neighbor_index, k_prime_index, k_prime);
    }

    
        
}






// void destroy_tree_nodes(node * root) {
//     int i;
//     if (root->is_leaf)
//         for (i = 0; i < root->num_of_keys; i++)
//             free(root->pointers[i]);
//     else
//         for (i = 0; i < root->num_of_keys + 1; i++)
//             destroy_tree_nodes((node *)root->pointers[i]);
//     free(root->pointers);
//     free(root->keys);
//     free(root);
// }


// page * destroy_tree(page * root) {
//     destroy_tree_nodes(root);
//     return NULL;
// }


int db_delete(int64_t key) {
    page * root=load_page(hp->rpo);
    page * leaf=find_leaf(hp->rpo,key);
    off_t leaf_offset=find_offset(key);
    if(leaf!=NULL){
        root=delete_entry(root,leaf_offset,key);
        return 0;
    }
    return -1;
}//fin







