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
	// USE MEMORY ALLOCATING FUNCTION TO ALLOCATE MEMORY HERE

  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
	if ((Scheduler::count_of_threads==0) || (Scheduler::head==NULL)){
		Console::puts("No threads in queue. error\n");
		assert(false);		
	}

	Threadnode* current_head = Scheduler::head;
	Threadnode* next_thread_node=Scheduler::head->next;
	Thread* next_thread = next_thread_node->thr;
	
	Scheduler::head = next_thread_node;
	//delete current_head;
	

	//Thread* current_thread = Scheduler::current_thr_node->thr;
	//Scheduler::current_thr_node = next_thread_node;
	//Console::puts("removing the node previously running.");
	//Scheduler::terminate(current_thread);

	Thread::dispatch_to(next_thread);

	Console::puts(" current thread yielded. cpu went to the next thread.\n");

}

void Scheduler::resume(Thread * _thread) {

  Scheduler::add(_thread);
}

void Scheduler::add(Thread * _thread) {
	// add a node just before the current thread pointer.
	
	Threadnode* new_thread_node = new Threadnode;
	new_thread_node->thr = _thread;
	new_thread_node->next = NULL;
	//Console::puts("interrupts???????????????????=================");Console::puti(Machine::interrupts_enabled());
	//Machine::disable_interrupts();

	if (Scheduler::head == NULL){
		
		Scheduler::head=new_thread_node;
		Scheduler::tail=new_thread_node;
		
		
	} else{
	Scheduler::tail->next = new_thread_node;
	Scheduler::tail = new_thread_node;
	}

	Scheduler::count_of_threads=Scheduler::count_of_threads+1;
	//Machine::enable_interrupts();

	return;

}

void Scheduler::terminate(Thread * _thread) {
  
  
  if (Scheduler::head->thr->ThreadId()==_thread->ThreadId()){
  	Scheduler::yield();
  }
  Threadnode* new_thread_node= Scheduler::head;
  Threadnode* problem_node=NULL;
  //Machine::disable_interrupts();
  while(new_thread_node->next!=NULL){
  
  Thread* next_thread = new_thread_node->next->thr;
  if (next_thread->ThreadId()==_thread->ThreadId()){
  problem_node = new_thread_node->next;
  new_thread_node->next = problem_node->next;
  delete problem_node;
  //break;
  }
  //Machine::enable_interrupts();
  
  }
  
  
  

 


 Scheduler::count_of_threads=Scheduler::count_of_threads-1;
  
}
