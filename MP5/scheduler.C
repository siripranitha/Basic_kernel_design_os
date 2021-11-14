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
Threadnode* Scheduler::current_thr_node; 
int Scheduler::count_of_threads; 


Scheduler::Scheduler() {
	Scheduler::count_of_threads = 0;
	// add an idle thread here.
	Scheduler::current_thr_node = NULL;
	// USE MEMORY ALLOCATING FUNCTION TO ALLOCATE MEMORY HERE

  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
	if ((Scheduler::count_of_threads==0) || (Scheduler::current_thr_node==NULL)){
		Console::puts("No threads in queue. error\n");
		assert(false);		
	}

	Threadnode* next_thread_node;
	Thread* next_thread;
	next_thread_node = Scheduler::current_thr_node->next;
	next_thread = next_thread_node->thr;

	Thread* current_thread = Scheduler::current_thr_node->thr;
	Scheduler::current_thr_node = next_thread_node;
	Console::puts("removing the node previously running.");
	//Scheduler::terminate(current_thread);

	Thread::dispatch_to(next_thread);

	Console::puts(" current thread yielded. cpu went to the next thread.\n");

}

void Scheduler::resume(Thread * _thread) {
  Console::puts("Resume function called. number of threads -is ");Console::puti(Scheduler::count_of_threads);

  Scheduler::add(_thread);
}

void Scheduler::add(Thread * _thread) {
	// add a node just before the current thread pointer.
	
	Threadnode* new_thread_node = new Threadnode;
	new_thread_node->thr = _thread;

	if (Scheduler::current_thr_node == NULL){
		Scheduler::current_thr_node=new_thread_node;
		new_thread_node->next = new_thread_node;
		new_thread_node->prev = new_thread_node;
		// assigning its own thread to itself.
		
	} else{

	Threadnode* node_before_current = Scheduler::current_thr_node->prev;

	new_thread_node->next = Scheduler::current_thr_node;
	new_thread_node->prev = node_before_current;

	node_before_current->next = new_thread_node;
	current_thr_node->prev = new_thread_node;
	}

	Scheduler::count_of_threads=Scheduler::count_of_threads+1;


	return;

}

void Scheduler::terminate(Thread * _thread) {
  Console::puts("	THREADS ARE GETTING TERMINATED YOOOO");
  
  if (Scheduler::count_of_threads<2){
  	Console::puts("no threads or one thread to terminate");
  	assert(false);
  }
  
  //Console::puts("current Thread: "); Console::puti(Thread::CurrentThread()->ThreadId());
  //Console::puts("terminate Thread: "); Console::puti(_thread->ThreadId());
  
  if (Scheduler::current_thr_node->thr->ThreadId()==_thread->ThreadId()){
  	Scheduler::yield();
  }

  Threadnode* new_node = Scheduler::current_thr_node;
  Threadnode* node_to_be_terminated =NULL;

  for (int i=0;i<Scheduler::count_of_threads;i++){
  	if (new_node->thr->ThreadId() == _thread->ThreadId()){
  		node_to_be_terminated = new_node;
  		break;
  	}
  	new_node = new_node->next;
  }

  if (node_to_be_terminated==NULL){
  	Console::puts(" thread to be terminated not found ");
  	return;
  }

  Threadnode* term_prev = node_to_be_terminated->prev;
  Threadnode* term_next = node_to_be_terminated->next;

  term_prev->next = term_next;
  term_next->prev = term_prev;
  Scheduler::count_of_threads-=1;
  //delete node_to_be_terminated;

  //thread_shutdown(_thread);
  // delete the  terminated node!! dont know how.




  
}
