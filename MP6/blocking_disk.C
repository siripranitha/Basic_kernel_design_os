/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "scheduler.H"
#include "thread.H"

extern Scheduler * SYSTEM_SCHEDULER;



/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
    this->diskqueue_tail=NULL;
    this->diskqueue_head=NULL;
    this->disk_queue_size=0;
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  Console::puts("is this accessing? write ");
  SimpleDisk::read(_block_no, _buf);

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  Console::puts("is this accessing? write");
  SimpleDisk::write(_block_no, _buf);
}

void BlockingDisk::add_disk_thread(Thread* _thread){
  Threadnode* new_thread_node = new Threadnode;
  new_thread_node->thr = _thread;
  new_thread_node->next = NULL;
  //Console::puts("interrupts???????????????????=================");Console::puti(Machine::interrupts_enabled());
  //Machine::disable_interrupts();

  if (this->diskqueue_head == NULL){
    
    this->diskqueue_head=new_thread_node;
    this->diskqueue_tail=new_thread_node;
    
    
  } else{
  this->diskqueue_head->next = new_thread_node;
  this->diskqueue_head = new_thread_node;
  }

  this->disk_queue_size+=1;
  //Machine::enable_interrupts();

  return;


}

void BlockingDisk::wait_until_ready() {
	Console::puts("is this accessing? ");
    if (!BlockingDisk::is_ready()) {
        
        Thread *current_thread = Thread::CurrentThread();
        
        //this->add_disk_thread(current_thread);
        
        SYSTEM_SCHEDULER->yield();
    }
	
    return;
}

bool BlockingDisk::is_ready() {
    return SimpleDisk::is_ready();
}

