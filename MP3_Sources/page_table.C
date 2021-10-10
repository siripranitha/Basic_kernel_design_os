#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;

#define PRESENT_BIT  1  // bit 0 should be set to 1
#define WRITE_BIT    2  // bit 1 ,the r/w bit to be set to 1

// referenced http://www.osdever.net/tutorials/view/implementing-basic-paging


void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
// sets up global param, that is kernel memory pool and process memory pool on paging initailization
  PageTable::shared_size = _shared_size;
  PageTable::kernel_mem_pool = _kernel_mem_pool;
  PageTable::process_mem_pool = _process_mem_pool

   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
  unsigned long address=0; 
  unsigned int i;
  
  unsigned long *page_directory = ( unsigned long*)(kernel_mem_pool->get_frames(1)*PAGE_SIZE);
  unsigned long *page_table = ( unsigned long*)(kernel_mem_pool->get_frames(1)*PAGE_SIZE);
   
    
  for(i=0; i<1024; i++){
    page_table[i] = address | WRITE_BIT | PRESENT_BIT; 
    // attribute set to: supervisor level, read/write, present(011 in binary)
    address = address + 4096; // 4096 = 4kb
  };

  page_directory[0] = page_table; 
  // entry of page directory set to page table
  //attribute set to: supervisor level, read/write, present(011 in binary)
  page_directory[0] = page_directory[0] | WRITE_BIT | PRESENT_BIT;


  for(i=1; i<1024; i++){
  page_directory[i] = 0 | WRITE_BIT; 
  // attribute set to: supervisor level, read/write, not present(010 in binary)
  };
   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{

  PageTable::current_page_table = this;
  write_cr3(page_directory); 
  Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
  // this function basically enables paging . that is it sets the value of paging_enabled variable to 1
  PageTable::paging_enabled=1
  write_cr0(read_cr0() | 0x80000000); // set the paging bit in CR0 to 1
  
  Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  assert(false);
  Console::puts("handled page fault\n");
}

