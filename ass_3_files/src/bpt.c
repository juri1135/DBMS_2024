#include "bpt.h"
//header page. root, free, number of pages
H_P * hp;

page * rt = NULL; //root is declared as global

int fd = -1; //fd is declared as global
static int table_counter = 0; 

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
    pwrite(fd, hp, sizeof(hp), 0);
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
        pwrite(fd, hp, sizeof(hp), 0);
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
    
    while (left_index < parent->num_of_keys &&  parent->b_f[left_index].p_offset != left_offset) {
        left_index++;
    }
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
    for (i =page->num_of_keys-1; i > left_index; i--) {
       page->b_f[i+1].key=page->b_f[i].key;
       page->b_f[i+1].p_offset=page->b_f[i].p_offset;
    }
    page->b_f[left_index+1].key =new_key;
    page->b_f[left_index+1].p_offset = new_offset;
    page->num_of_keys++;
    return page;
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
    temp_next_offset=old_node->next_offset;

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
    int i=0, j=0;
    for (i = 0, j = 0; i < leaf->num_of_keys; i++, j++) {
        if (j == insertion_index) j++; 
        temp_records[j] = leaf->records[i];
    }
    temp_records[insertion_index] = pair;

    leaf->num_of_keys = 0;
    //0~num_of_keys-1까지 채워
    for (i = 0; i <leaf->num_of_keys; i++) {
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
    off_t leftparent_offset=leaf->parent_page_offset;
    off_t rightparent_offset=right_sibling->parent_page_offset;
    page * leftparent=load_page(leftparent_offset);
    page * rightparent=load_page(rightparent_offset);
    while(leftparent_offset!=rightparent_offset){
       leftparent_offset=leftparent->parent_page_offset;
       leftparent=load_page(leftparent_offset);
       rightparent_offset=rightparent->parent_page_offset;
       rightparent=load_page(rightparent_offset);
    }
    if(leftparent_offset==rightparent_offset){
        insertion_index=0;
        while(insertion_index<rightparent->num_of_keys&&rightparent->b_f[insertion_index].key<key){
            insertion_index++;
        }
        rightparent->b_f[insertion_index].key=right_sibling->records[0].key;
    }
    pwrite(fd,rightparent,sizeof(page),rightparent_offset);
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


int db_delete(int64_t key) {

}//fin








