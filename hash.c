#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define SIZE 64

struct DataItem {
   int key;
   int data;
   int target_addr;  
   
};

struct DataItem* hashArray[SIZE]; 
struct DataItem* dummyItem;
struct DataItem* item;

int hashCode(int key) {
   return key % SIZE;
}

struct DataItem *search(int key) {
   //get the hash 
   int hashIndex = hashCode(key);  
	
   //move in array until an empty 
   if(hashArray[hashIndex] != NULL) {
      return hashArray[hashIndex];  
   }        
   else return NULL;
     
}

void insert(int key,int data, int target_addr ) {

   struct DataItem *item = (struct DataItem*) malloc(sizeof(struct DataItem));
   item->key = key;
   item->data = data;
   item->target_addr = target_addr;  
   

   //get the hash 
   int hashIndex = hashCode(key);

   //move in array until an empty or deleted cell	
   hashArray[hashIndex] = item;
}


void display() {
   int i = 0;
	
   for(i = 0; i<SIZE; i++) {
	
      if(hashArray[i] != NULL)
         printf(" (%d,%d)",hashArray[i]->key,hashArray[i]->data);
      else
         printf(" ~~ ");
   }
	
   printf("\n");
}

