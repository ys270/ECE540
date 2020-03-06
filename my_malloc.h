#include<unistd.h>
#include<stdio.h>
#include<sys/types.h>
void* ff_malloc(size_t size);
void ff_free(void *ptr);
void* bf_malloc(size_t size);
void bf_free(void *ptr);

//two calculation function
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();

//the structure of the block
typedef struct Meta_Control_Block* mcb_addr;
struct Meta_Control_Block{
  size_t size;
  size_t available;
  mcb_addr next;
  mcb_addr prev;
  mcb_addr fnext;
  mcb_addr fprev;
};
typedef struct Meta_Control_Block mcb; 

//helper functions
void init();//initialize global variables
size_t align_8(size_t size);
void merge_block(mcb_addr pmcb);
void split_block(mcb_addr pcurrent,size_t size);
void* ff_find(size_t size);
void* extend_head(size_t size);
void* bf_find(size_t size);
void remove_free_block(mcb_addr pmcb);
void add_free_block(mcb_addr pmcb);
