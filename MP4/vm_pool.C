/*
 File: vm_pool.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

#define MAX_COUNT  200 //PRONE TO CHANGE, MAY BE DERIVE FROM PAGE SIZE ITSELF
#define PS  Machine::PAGE_SIZE



VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;
    page_table->register_pool(this);

    allocated_region = (struct allocated_region_*) (base_address); 
    // pointer starts from base addr, that means in first page i guess.
    region_iterator = 0;

    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
  // used lk -> address->, base,base+_size
  //page table ->
  if (size == 0){
        Console::puts("0 size invalid for allocate");
        return 0;
    }

    unsigned int page_count = _size/PS;
    unsigned rem = _size%PS;

    if (rem>0){
      page_count = page_count+1; 
    }
    unsigned long final_size = page_count*PS;
    
    assert((region_iterator+page_count)<=MAX_COUNT-1);

    if (region_iterator==0){
      allocated_region[region_iterator].start_address_region = base_address+PS;
      }
    else{
      allocated_region[region_iterator].start_address_region = allocated_region[region_iterator-1].start_address_region+allocated_region[region_iterator-1].size_of_region;
      }
    
    allocated_region[region_iterator].size_of_region = final_size;

    region_iterator = region_iterator+1;
    return allocated_region[region_iterator-1].start_address_region+final_size;
    
    Console::puts("Allocated region of memory.\n");
}

void VMPool::release(unsigned long _start_address) {
 // mark those invalid
  unsigned int index ;
    for (int i=0;i<MAX_COUNT;i++){
      if (allocated_region[i].start_address_region == _start_address){
        index = i;
        break;
      }
    }

  unsigned int page_count = allocated_region[index].size_of_region/PS;
  unsigned long addr ;
    for (int i = 0;i<page_count;i++){
      addr = allocated_region[index].start_address_region+i*PS;
      page_table->free_page(addr);
    }

  for (int i =index;i<region_iterator-1;i++){
    allocated_region[i] = allocated_region[i+1];
  }
  region_iterator = region_iterator -1;
  page_table->load();

    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
  // is it bw/w base and base+size or should it be present in allocated list as well?
    if ((_address>=base_address)&&((base_address+size)>_address)){
      return true;
    }else{
      return false;
    }
    
}

