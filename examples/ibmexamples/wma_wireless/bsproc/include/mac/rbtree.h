
enum rbtree_node_color { RED, BLACK };

typedef struct rbtree_node_t {
    void* key;
    void* value;
    struct rbtree_node_t* left;
    struct rbtree_node_t* right;
    struct rbtree_node_t* parent;
    enum rbtree_node_color color;
} rbtree_node;

typedef struct rbtree_t {
    rbtree_node* root;
} rbtree;


rbtree* rbtree_create();
void rbtree_destroy(rbtree*);
void* rbtree_lookup(rbtree* t, void* key);
void rbtree_insert(rbtree* t, void* key, void* value);
void rbtree_delete(rbtree* t, void* key);


