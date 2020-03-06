#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include"my_malloc.h"

void* memstart;//the start of the heap
void* lastaddr;//the end of the heap
mcb_addr lastnode;//the last node in the heap
mcb_addr freehead;// the free list's head
mcb_addr freetail;//the free list's tail
int has_init=0;//whether we have initialize the gloabl variables

//initialize global variables
void init(){
  memstart=sbrk(0);
  lastaddr=memstart;
  lastnode=NULL;
  freehead=NULL;
  freetail=NULL;
  has_init=1;
}

//make size the multiples of 8
size_t align_8(size_t size){
   size=(((size-1)>>3)<<3)+8;
return size;
}

//if we find the free block suitable for malloc, enter split function
//to determine whether we should split the free block or not 
void split_block(mcb_addr pcurrent,size_t size){
  remove_free_block(pcurrent);
  if(pcurrent->size>sizeof(mcb)+size+8){
    mcb_addr temp=pcurrent->next;
    mcb_addr current=(void*)pcurrent+size;
    if(pcurrent!=lastnode){
      temp->prev=current;
    }
    else{
      lastnode=current;
    }
    pcurrent->next=current;
    current->prev=pcurrent;
    current->size=pcurrent->size-size;
    current->available=1;
    current->next=temp;
    pcurrent->size=size;
    add_free_block(current);
    }
}

//first fit find, if we find the free block suitable for the required size,
//set ret to be the block's address. Else, ret=NULL
void* ff_find(size_t size){
  size=size+sizeof(mcb);
  mcb_addr pcurrent=freehead;
  void* ret=NULL;
  while(pcurrent!=NULL){
    if(pcurrent->size>=size){
      pcurrent->available=0;
      ret=pcurrent;
      split_block(pcurrent,size);
      break;
    }
    pcurrent=pcurrent->fnext;
  }
  if(ret!=NULL){
    ret=ret+sizeof(mcb);
  }
  return ret;
}

//if we cannot find the suitable free block in heap, we need to extend heap
//thus, we also need to update lastaddr and lastnode
void* extend_heap(size_t size){
  size=size+sizeof(mcb);
  void* ret=lastaddr;
  sbrk(size);
  lastaddr=lastaddr+size;
  mcb_addr pmcb=(mcb_addr)ret;
  pmcb->prev=lastnode;
  pmcb->size=size;
  pmcb->available=0;
  pmcb->next=lastaddr;
  pmcb->fprev=NULL;
  pmcb->fnext=NULL;
  lastnode=pmcb;
  ret=ret+sizeof(mcb);
  return ret;
}

//best fit find, loop to find the best place to put the malloc space
//if we cannot find such block, return NULL
void* bf_find(size_t size){
  size=size+sizeof(mcb);
  mcb_addr pcurrent=freehead;
  void* ret=NULL;
  size_t smallest_diff=0xffffffffffffffff;
  while(pcurrent!=NULL){
     if(pcurrent->size>=size){
      size_t diff=pcurrent->size-size;
      if(diff==0){
	ret=(void*)pcurrent;
	break;
	}
      if(diff<smallest_diff){
	ret=(void*)pcurrent;
        smallest_diff=diff;
      }
    }
    pcurrent=pcurrent->fnext;
  }
  if(ret!=NULL){
  mcb_addr add=(mcb_addr)ret;
  add->available=0;
  //remove_free_block(add);
  split_block(add,size);
  ret=ret+sizeof(mcb);
  }
  return ret;
}

//using helper functions to realize first fit malloc
void* ff_malloc(size_t size){
  if(has_init!=1){
    init();
  }
  size=align_8(size);
  void* ret=ff_find(size);
  if(ret==NULL){
    ret=extend_heap(size);
  }
  return ret;
}

//using helper functions to realize best fit malloc
void* bf_malloc(size_t size){
  if(has_init!=1){
    init();
  }
  size=align_8(size);
  void* ret=bf_find(size);
  if(ret==NULL){
    ret=extend_heap(size);
  }
  return ret;
}

//if the block we need to free is next to any freed blocks
//we need to merge them to form a larger free block
void merge_block(mcb_addr pmcb){
  if(pmcb->prev!=NULL){
    if(pmcb->prev->available==1){
      remove_free_block(pmcb->prev);
      pmcb->prev->size=pmcb->prev->size+pmcb->size;
      pmcb->prev->next=pmcb->next;
      if(pmcb->next!=lastaddr){
      pmcb->next->prev=pmcb->prev;
      }
      else{
	lastnode=pmcb->prev;
      }
      pmcb=pmcb->prev;
    }
  }
  if(pmcb->next!=lastaddr){
    if(pmcb->next->available==1){
      remove_free_block(pmcb->next);
      pmcb->size=pmcb->size+pmcb->next->size;
      pmcb->next=pmcb->next->next;
      if(pmcb->next!=lastaddr){
      pmcb->next->prev=pmcb;
      }
      else{
	lastnode=pmcb;
      }
    }
  }
  add_free_block(pmcb);
}

//add free block to the freelist 
void add_free_block(mcb_addr pmcb){
  if(freehead==NULL){
    freehead=pmcb;
    freetail=pmcb;
    pmcb->fprev=NULL;
    pmcb->fnext=NULL;
  }
  else{
    freetail->fnext=pmcb;
    pmcb->fprev=freetail;
    freetail=pmcb;
    freetail->fnext=NULL;
  }
}

//remove occupied blocks from the freelist 
void remove_free_block(mcb_addr pmcb){
  if(pmcb->fprev==NULL&&pmcb->fnext==NULL){
    freehead=NULL;
    freetail=NULL;
  }
 else if(pmcb->fprev==NULL){
    freehead=pmcb->fnext;
    freehead->fprev=NULL;
 }
 else if(pmcb->fnext==NULL){
    freetail=pmcb->fprev;
    freetail->fnext=NULL;
  }
 else{
   pmcb->fprev->fnext=pmcb->fnext;
   pmcb->fnext->fprev=pmcb->fprev;
 }
 pmcb->fprev=NULL;
 pmcb->fnext=NULL;
}

//free function for first fit malloc
void ff_free(void *ptr){
  mcb_addr pmcb=(mcb_addr)(ptr-sizeof(mcb));
  pmcb->available=1;
  merge_block(pmcb);
}
//free function for best fit malloc
void bf_free(void *ptr){
  mcb_addr pmcb=(mcb_addr)(ptr-sizeof(mcb));
  pmcb->available=1;
  merge_block(pmcb);
}

//calculate the size of the entire heap
unsigned long get_data_segment_size(){
  if(has_init!=1){
    return 0;
  }
  unsigned long res=lastaddr-memstart;
  return res;
}

//calculate the size of free blocks
unsigned long get_data_segment_free_space_size(){
  unsigned long free_space=0;
  void* current=memstart;
  while(current!=lastaddr){
    mcb_addr pcurrent=current;
    if(pcurrent->available==1){
      free_space=free_space+pcurrent->size;
    }
    current=pcurrent->next;
    }
  return free_space;
}

