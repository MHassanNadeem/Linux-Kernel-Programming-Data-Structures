/* Linux Kernel Programming
 * Project 2
 * 
 * Hassan Nadeem
 * hnadeem@vt.edu
 * */

#pragma GCC diagnostic ignored "-Wdeclaration-after-statement" /* Supress warning */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/kfifo.h>
#include <linux/slab.h>
#include <linux/hashtable.h>
#include <linux/rbtree.h>
#include <linux/radix-tree.h>

#define DBG(var, type)          printk(KERN_INFO #var" = %"#type"\n", var)
#define DBGM(var, type, desc)   printk(KERN_INFO desc" = %"#type"\n", var)
#define PRINT(msg, ...)         printk(KERN_INFO msg, ##__VA_ARGS__)

static char *int_str = "-1 0 1";
module_param(int_str, charp, 0);
MODULE_PARM_DESC(int_str, "Arbitrary list of space separated integers");

/*---------------------------------------------------------------------------*/
/* Array */
/*---------------------------------------------------------------------------*/

int print_array(const int *array, const int size){
	int i;
	
	PRINT("=== ARRAY ===");
	
	for(i=0; i<size; i++){
		PRINT("array[%02d] = %d\n", i, array[i]);	
	}
	
	return 0;
}
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Linked List */
/*---------------------------------------------------------------------------*/

struct IntList{
	int data;
	struct list_head list;
};

int linked_list_append(struct list_head *head, int data){
	struct IntList *tmp;

	tmp = (struct IntList*)kmalloc(sizeof(struct IntList), GFP_KERNEL);
	tmp->data = data;
	list_add_tail(&tmp->list, head);
	
	return 0;
}

int linked_list_appendArray(struct list_head *head, int *array, int size){
	int i;

	for(i=0; i<size; i++){
		linked_list_append(head, array[i]);
	}
	
	return 0;
}

int linked_list_destroy(struct list_head *head){
	struct IntList *tmp;
	struct list_head *pos, *pos2;
	
	list_for_each_safe(pos, pos2, head){
		tmp = list_entry(pos, struct IntList, list);
		list_del(pos);
		kfree(tmp);
	}
	
	return 0;
}

int linked_list_print(struct list_head *head){
	struct IntList *listNode;
	struct list_head *pos;
	
	list_for_each(pos, head){
		listNode = list_entry(pos, struct IntList, list);
		DBG(listNode->data, d);
	}
	
	return 0;
}

int linked_list_task(int *array, int size){
	LIST_HEAD(myList);
	
	PRINT("=== LINKED LIST TASK ===");
	
	linked_list_appendArray(&myList, array, size);
	linked_list_print(&myList);
	linked_list_destroy(&myList);
	
	return 0;
}

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Red-Black tree */
/*---------------------------------------------------------------------------*/

struct int_rbnode{
    struct rb_node node;
    int data;
};

int rbtree_insert(struct rb_root *root, struct int_rbnode *data){
    struct rb_node **new = &(root->rb_node), *parent = NULL;

    /* Figure out where to put new node */
    while (*new){
        struct int_rbnode *this = container_of(*new, struct int_rbnode, node);

        parent = *new;
        if (data->data < this->data)
            new = &((*new)->rb_left);
        else
            new = &((*new)->rb_right);
    }

    /* Add new node and rebalance tree. */
    rb_link_node(&data->node, parent, new);
    rb_insert_color(&data->node, root);

    return 0;
}

struct int_rbnode *rbtree_search(struct rb_root *root, int val){
    struct rb_node *node = root->rb_node;

    while (node) {
        struct int_rbnode *data = container_of(node, struct int_rbnode, node);

        if (data->data == val)
            return data;
		else if (data->data > val)
  			node = node->rb_left;
		else
  			node = node->rb_right;
	}
	return NULL;
}

int rbtree_task(int *array, int size){
	struct int_rbnode* tmp;
    int i;
    
	PRINT("=== RBTREE TASK ===");
	
    /* Create rbtree */
    struct rb_root root = RB_ROOT;
    
    /* Insert numbers */
    for(i = 0; i<size; i++){
        tmp = (struct int_rbnode*) kmalloc(sizeof(struct int_rbnode), GFP_KERNEL);
        tmp->data = array[i];
        rbtree_insert(&root, tmp);
    }
    
    /* Lookup and print */
    for(i = 0; i<size; i++){
        tmp = rbtree_search(&root, array[i]);
        if(tmp == NULL){
            PRINT("node not found");
            return -1;
        }
        DBG(tmp->data, d);
    }
    
    /* Remove numbers */
    for(i = 0; i<size; i++){
        tmp = rbtree_search(&root, array[i]);
        if(tmp != NULL){
            rb_erase(&tmp->node, &root);
            kfree(tmp);
        }
    }
    
	return 0;
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Hash Table */
/*---------------------------------------------------------------------------*/

struct int_hashtableEntry{
    struct hlist_node hnode;
    int data;
};

/* Hashtable is too big to be declared on the stack */
DEFINE_HASHTABLE(int_hashtable, 14); /* 2^14 buckets */ /////////////////////////////////////// FIX THIS

struct int_hashtableEntry *hashtable_search(int val){
    struct int_hashtableEntry *ret;
    
    hash_for_each_possible(int_hashtable, ret, hnode, val){
        if (val == ret->data)
            return ret;
    }
        
    return NULL;
}

int hashtable_task(int *array, int size){
    int i;
    struct int_hashtableEntry *tmp;
    struct hlist_node *tmp_hlist_node;
    
    
    PRINT("=== HASHTABLE TASK ===");
    
    /* Create and init*/
    hash_init(int_hashtable);
    
    /* Insert */
    for(i=0; i<size; i++){
        tmp = (struct int_hashtableEntry*) kmalloc(sizeof(struct int_hashtableEntry), GFP_KERNEL);
        tmp->data = array[i];
        hash_add(int_hashtable, &tmp->hnode, tmp->data);
    }
    
    /* Iterate */
    PRINT("- hashtable iteration");
    hash_for_each(int_hashtable, i, tmp, hnode){
        DBG(tmp->data, d);
    }
    
    /* Lookup */
    PRINT("- hashtable lookup");
    for(i=0; i<size; i++){
        tmp = hashtable_search(array[i]);
        if(tmp == NULL){
            PRINT("Ops Something is wrong. Key not found");
            return -1;
        }else{
            DBG(tmp->data, d);
        }
    }
    
    /* Remove and Destroy */
    PRINT("- hashtable remove and free");
    hash_for_each_safe(int_hashtable, i, tmp_hlist_node, tmp, hnode){
        hash_del(&tmp->hnode);
        kfree(tmp);
    }
    
    return 0;
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Radix Tree */
/*---------------------------------------------------------------------------*/

#define isEven(x) (x%2 == 0)
#define isOdd(x) (!isEven(x))
#define TAG_ODD 1 /* tag for radix tree */

int radixtree_task(int *array, int size){
    int ret, i;
    void *lookupResult;
    void *voidPtrArray[size];
    
    PRINT("=== RADIX TREE TASK ===");
    
    /* Create a radix tree, which is indexed by integer numbers */
    struct radix_tree_root root;
    INIT_RADIX_TREE(&root, GFP_KERNEL);
    
    /* Insert integer numbers in int_str to the radix tree */
    for(i=0; i<size; i++){
        ret = radix_tree_insert(&root, array[i], (void*)&array[i]);
        if(ret == -EEXIST){
            PRINT("Error inserting key already exists");
            return -1;
        }else if(ret != 0){
            PRINT("Error inserting number into radix tree");
            return -1;
        }
    }
    
    /* Look up the inserted numbers and print them out */
    PRINT("- radixtree lookup");
    for(i=0; i<size; i++){
        lookupResult = radix_tree_lookup(&root, array[i]);
        DBG(*(int*)lookupResult, d);
    }
    
    /* Tag all odd numbers in the radix tree */
    for(i=0; i<size; i++){
        if(isOdd(array[i]))
            radix_tree_tag_set(&root, array[i], TAG_ODD);
    }
    
    /* Look up all tagged odd number using radix_tree_gang_lookup_tag */
    PRINT("- radixtree odd number lookup");
    ret = radix_tree_gang_lookup_tag(&root, voidPtrArray, 0, size, TAG_ODD);
    for(i=0; i<ret; i++){
        DBG(*(int*)voidPtrArray[i], d);
    }
        
    
    /* Remove all inserted numbers in the radix tree */
    for(i=0; i<size; i++){
        radix_tree_delete(&root, array[i]);
    }
    
    return 0;
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* BitMap */
/*---------------------------------------------------------------------------*/

int bitmap_task(int *array, int size){
    
    PRINT("=== BITMAP TASK ===");
    
    return 0;
}
/*---------------------------------------------------------------------------*/

static int getNumInts(const char *int_str){
    int num_ints = 1, i;
    
    if(int_str == 0 || *int_str == 0) return 0;
    
    for(i=0; i<strlen(int_str); i++){
        if(int_str[i] == ' '){
            num_ints++;
            while(int_str[i] == ' ') i++;
        }
    }
    
    DBG(num_ints, d);
    
    return num_ints;
}

static int parseInts(char *int_str, int *int_array, int size){
    int i;
    char *int_string;
    int ret;
    
    for(i=0; i<size; i++){
        int_string = strsep(&int_str, " ");
        if( int_string == 0 ) return -1;
        ret = kstrtoint(int_string, 10, &int_array[i]);
        if(ret != 0) return -1;
    }
    
    print_array(int_array, size);
    
    return 0;
}

int *int_array;

int init_module(void){
    DBG(int_str, s);
    int num_ints;
    
    /* Parse the integers */
    num_ints = getNumInts(int_str);
    if(num_ints < 1){
        PRINT("Error Parsing Input");
        return -1;
    }
    
    int_array = (int*) kmalloc(sizeof(int)*num_ints, GFP_KERNEL);
    
    if(parseInts(int_str, int_array, num_ints)){
        PRINT("Error Parsing Input");
        return -1;
    }
    
    
	PRINT("========== PROJECT 2 ==========\n");

	if(linked_list_task(int_array, num_ints))   PRINT("ERROR in linked list task 2\n");
	if(rbtree_task(int_array, num_ints))        PRINT("ERROR in rbTree task\n");
    if(hashtable_task(int_array, num_ints))     PRINT("ERROR in hash table task\n");
    if(radixtree_task(int_array, num_ints))     PRINT("ERROR in radix tree task\n");
    if(bitmap_task(int_array, num_ints))        PRINT("ERROR in bitmap task\n");

	return 0;
}

void cleanup_module(void)
{
    kfree(int_array);
	printk(KERN_INFO "-----------------------------\n");
}

MODULE_LICENSE("GPL");
