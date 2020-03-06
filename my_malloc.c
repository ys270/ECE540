#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include <limits.h>
#include <pthread.h>
#include"my_malloc.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
__thread block * fhead_nolock = NULL;
unsigned long heap_size = 0;
block * fhead_lock = NULL;
int lock_version=0;

void* ts_malloc_lock(size_t size){
  lock_version=0;
  pthread_mutex_lock(&mutex);
  void* ret = bf_malloc(size);
  pthread_mutex_unlock(&mutex);
  return ret;
}

void ts_free_lock(void *ptr){
  pthread_mutex_lock(&mutex);
  bf_free(ptr);
  pthread_mutex_unlock(&mutex);
}

void* ts_malloc_nolock(size_t size){
  lock_version=1;
  void* ret=bf_malloc(size);
  return ret;
}
void ts_free_nolock(void *ptr){
  bf_free(ptr);
}

void split_block(block * ptr, size_t size) {
  remove_free_block(ptr);
  block * add = (block *)((char *)ptr + size + sizeof(block));
  add->size = ptr->size - sizeof(block) - size;
  ptr->size = size;
  add_free_block(add);
}

void remove_free_block(block * ptr) {
  block* fhead=NULL;
  if(lock_version==0){
    fhead=fhead_lock;
  }
  else{
    fhead=fhead_nolock;
  }
  if (ptr == fhead) {
    if (ptr->fnext == NULL) {
      fhead = NULL;
    }
    else {
      fhead = ptr->fnext;
    }
  }
  else {
    block * curr = fhead;
    while (curr->fnext != ptr) {
      curr = curr->fnext;
    }
    curr->fnext = ptr->fnext;
  }
  if(lock_version==0){
    fhead_lock=fhead;
  }
  else{
    fhead_nolock=fhead;
  }

}

void add_free_block(block * ptr) {
  block* fhead=NULL;
  if(lock_version==0){
    fhead=fhead_lock;
  }
  else{
    fhead=fhead_nolock;
  }
  if (fhead == NULL) {
    fhead = ptr;
    fhead->fnext = NULL;
  }
  else if (ptr < fhead) {
    ptr->fnext = fhead;
    fhead = ptr;
  }
  else{
    block * current = fhead;
    while (current->fnext != NULL) {
    if ((current < ptr) && (ptr < current->fnext)) {
      block * temp = current->fnext;
      current->fnext = ptr;
      ptr->fnext = temp;
      break;
    }
    current = current->fnext;
  }
  if(current->fnext==NULL){
    current->fnext = ptr;
    ptr->fnext = NULL;
  }
  }
  if(lock_version==0){
    fhead_lock=fhead;
  }
  else{
    fhead_nolock=fhead;
  }
}

void * ff_malloc(size_t size) {
  /* if (isInit==0) {
    block * temp = sbrk(size + sizeof(block));
    temp->size = size;
    heap_size +=(size + sizeof(block)); 
    isInit = 1;
    void * ret = (void *)((char *)temp + sizeof(block));
    return ret;
    }*/
  void * ans= ff_find(size);
  if (ans==NULL) {
    ans = extend_heap(size);
  }
  return ans;
  }

void * extend_heap(size_t size){
  block* temp=NULL;
  if(lock_version==0){
    temp = sbrk(size + sizeof(block));
  }
  else{
    pthread_mutex_lock(&mutex);
    temp = sbrk(size + sizeof(block));
    pthread_mutex_unlock(&mutex);
  }
    temp->size = size;
    heap_size += (size + sizeof(block));
    void * ret = (void *)((char *)temp + sizeof(block));
    return ret;
}

void * ff_find(size_t size){
  block* fhead=NULL;
  if(lock_version==0){
    fhead=fhead_lock;
  }
  else{
    fhead=fhead_nolock;
  }
  block * current = fhead;
  void * ret = NULL;
  while (current != NULL) {
    if (current->size >= size) {
      if (current->size > size + sizeof(block)) {
        split_block(current, size);
	ret = (void *)((char *)current + sizeof(block));
        return ret;
      }
      remove_free_block(current);
      ret = (void *)((char *)current + sizeof(block));
      return ret;
    }
    current = current->fnext;
  }
  return NULL;
  }

void * bf_malloc(size_t size){
  /*  if (isInit==0) {
    block * temp = sbrk(size + sizeof(block));
    temp->size = size;
    heap_size += (size + sizeof(block)); 
    isInit = 1;
    void * ret = (void *)((char *)temp + sizeof(block));
    return ret;
    }*/
  void * ret = bf_find(size);
  if (ret==NULL) {
    ret = extend_heap(size);
  }
  return ret;
}

void * bf_find(size_t size) {
  block* fhead=NULL;
  if(lock_version==0){
    fhead=fhead_lock;
  }
  else{
    fhead=fhead_nolock;
  }
  block * ret = NULL;
  block * current = fhead;
  long long int smallest_diff = 0x7fffffffffffffff; 
  while (current) {
    if ((current->size >= size)) { 
      int diff = (long long int)(current->size - size);
      if (diff == 0) { 
        ret = current;
        break;
      }
      if (diff < smallest_diff) {
        smallest_diff = diff;
        ret = current;
      }
    }
    current = current->fnext;
  }
  if(ret==NULL){
    return ret;
  }
  else{
      if ((long long int)(ret->size - size - sizeof(block)) < 0) {
        remove_free_block(ret);
	ret = (void *)((char *)ret + sizeof(block));
        return ret;
      }
      else {
        split_block(ret, size);
	ret = (void *)((char *)ret + sizeof(block));
        return ret;
      }
    remove_free_block(ret);
    ret = (void *)((char *)ret + sizeof(block));
    return ret;
  }
}

void ff_free(void * ptr) {
  block * temp = (void *)((char *)ptr - sizeof(block));
  add_free_block(temp);
  merge_free_block(temp);
  }

void bf_free(void * ptr) {
   block * temp = (void *)((char *)ptr - sizeof(block));
  add_free_block(temp);
  merge_free_block(temp);
}

void merge_free_block(block * ptr){
  block* fhead=NULL;
  if(lock_version==0){
    fhead=fhead_lock;
  }
  else{
    fhead=fhead_nolock;
  }
  block * current = fhead;
  while (current != ptr->fnext) {
    if ((block *)(current->fnext) == (block *)((char *)current + current->size + sizeof(block))) {
      current->size = current->size + sizeof(block) + current->fnext->size;
      current->fnext = current->fnext->fnext;
      if (!current->fnext) {
        break;
      }
      if ((block *)(current->fnext) == (block *)((char *)current + current->size + sizeof(block))) {
        current->size = current->size + sizeof(block) + current->fnext->size;
        current->fnext = current->fnext->fnext;
        break;
      }
      break;
    }
    current = current->fnext;
  }
}

unsigned long get_data_segment_size() {
  return heap_size;
}

unsigned long get_data_segment_free_space_size() {
  unsigned long ans = 0;
  block* fhead=NULL;
  if(lock_version==0){
    fhead=fhead_lock;
  }
  else{
    fhead=fhead_nolock;
  }
  block * p = fhead;
  while (p != NULL) {
    ans += (p->size + sizeof(block));
    p = p->fnext;
  }
  return ans;
  }
