/*
 File: scheduler.C
 
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

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
//#include "blocking_disk.H"




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
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/
Threadnode* Scheduler::head;
Threadnode* Scheduler::tail; 
int Scheduler::count_of_threads; 


Scheduler::Scheduler() {
	Scheduler::count_of_threads = 0;
	// add an idle thread here.
	Scheduler::head = NULL;
	Scheduler::tail = NULL;
	this->disk = NULL;
	// USE MEMORY ALLOCATING FUNCTION TO ALLOCATE MEMORY HERE

  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::update_disk(BlockingDisk * _disk){
	this->disk = _disk;
	return;
}

void Scheduler::yield() {
	if ((Scheduler::count_of_threads==0) || (Scheduler::head==NULL)){
		Console::puts("No threads in queue. error\n");
		assert(false);		
	}
	
	//Console::puts("is disk ready --");Console::puti(this->disk->is_ready());
	
	
	
	if ((this->disk->disk_queue_size>0)&&(this->disk->is_ready())){
	   Console::puts("*********************************************************************************");
	   Console::puts("checking if the disk is ready and dispatching next thread in disk queue ");Console::puts("\n");
	   Thread::dispatch_to(this->disk->return_first_disk_thread());
	   return;		
	} else{
	
	
	
	
	Thread* next_thread;
	
	if (Thread::CurrentThread()->ThreadId()== Scheduler::head->thr->ThreadId()){

	Threadnode* current_head = Scheduler::head;
	Threadnode* next_thread_node=Scheduler::head->next;
	next_thread = next_thread_node->thr;
	
	Scheduler::head = next_thread_node;
	//delete current_head;
	Scheduler::count_of_threads=Scheduler::count_of_threads-1;
	}else{
	next_thread=Scheduler::head->thr;
	}
	

	//Thread* current_thread = Scheduler::current_thr_node->thr;
	//Scheduler::current_thr_node = next_thread_node;
	//Console::puts("removing the node previously running.");
	//Scheduler::terminate(current_thread);
	
	
	
	Console::puts("scheduler yielding to thread id :");Console::puti(next_thread->ThreadId()+1); Console::puts("\n");


	Thread::dispatch_to(next_thread);
	return;}
	
	
	
}

void Scheduler::resume(Thread * _thread) {

  Scheduler::add(_thread);
}

void Scheduler::add(Thread * _thread) {
	// add a node just before the current thread pointer.
	
	Threadnode* new_thread_node = new Threadnode;
	new_thread_node->thr = _thread;
	new_thread_node->next = NULL;
	
	if (Scheduler::head == NULL){
		
		Scheduler::head=new_thread_node;
		Scheduler::tail=new_thread_node;
		
		
	} else{
	Scheduler::tail->next = new_thread_node;
	Scheduler::tail = new_thread_node;
	}

	Scheduler::count_of_threads=Scheduler::count_of_threads+1;
	Console::puts("added the thread back to ready queue :");Console::puti(_thread->ThreadId()+1); Console::puts("\n");

	
	return;

}

void Scheduler::terminate(Thread * _thread) {
  
  
  if (Scheduler::head->thr->ThreadId()==_thread->ThreadId()){
  	Scheduler::yield();
  }
  Threadnode* new_thread_node= Scheduler::head;
  Threadnode* problem_node=NULL;
  while(new_thread_node->next!=NULL){
  
  Thread* next_thread = new_thread_node->next->thr;
  if (next_thread->ThreadId()==_thread->ThreadId()){
  problem_node = new_thread_node->next;
  new_thread_node->next = problem_node->next;
  delete problem_node;
  //break;
  }
  
  }
  
  
  

 


 Scheduler::count_of_threads=Scheduler::count_of_threads-1;
  
}
