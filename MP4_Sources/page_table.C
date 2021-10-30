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
#define USER_LEVEL_BIT  4
// referenced http://www.osdever.net/tutorials/view/implementing-basic-paging
// implementing a linked list in c


void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
// sets up global param, that is kernel memory pool and process memory pool on paging initailization
  PageTable::shared_size = _shared_size;
  PageTable::kernel_mem_pool = _kernel_mem_pool;
  PageTable::process_mem_pool = _process_mem_pool;
  
   Console::puts("Initialized Paging System\n");

}

PageTable::PageTable()
{
  unsigned long address=0; 
  unsigned int i;
  
  page_directory = ( unsigned long*)(kernel_mem_pool->get_frames(1)*PAGE_SIZE);
  unsigned long *page_table = ( unsigned long*)(process_mem_pool->get_frames(1)*PAGE_SIZE);
   
    
  for(i=0; i<1024; i++){
    page_table[i] = address | WRITE_BIT | PRESENT_BIT ; 
    // attribute set to: supervisor level, read/write, present(011 in binary)
    address = address + 4096; // 4096 = 4kb
  };
  //page_table[1023] = (unsigned long) page_table;
  //page_table[1023] = page_table[1023] | PRESENT_BIT;
  // attribute should be supervisor level , read only , present - 001

  page_directory[0] = (unsigned long) page_table; 
  // entry of page directory set to page table
  //attribute set to: supervisor level, read/write, present(011 in binary)
  page_directory[0] = page_directory[0] | WRITE_BIT | PRESENT_BIT;


  for(i=1; i<1024; i++){
  page_directory[i] = 0 | WRITE_BIT ; 
  // attribute set to: supervisor level, read/write, not present(010 in binary)
  };

  page_directory[1023] = (unsigned long)page_directory;
  page_directory[1023] = page_directory[1023] | 0x3; //????
  vmpool_current = NULL;
  vmpool_count = 0;



  // writing it for recursive loop  
    Console::puts("Constructed Page Table object\n");
}
    

void PageTable::load()
{
    PageTable::current_page_table = this;
    write_cr3((unsigned long)page_directory); 
  
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    // this function basically enables paging . that is it sets the value of paging_enabled variable to 1
  PageTable::paging_enabled=1;
  write_cr0(read_cr0() | 0x80000000); // set the paging bit in CR0 to 1
  
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  unsigned long fault_addr = read_cr2();
  if(_r->err_code&1==1){//if the last bit is set to zero that means it is protection fault do nothing
    Console::puts("protection fault");
    return;
  }else{
  //get  the current page table directory
  VMPool* pool=current_page_table->vm_pool;
  bool flag=false;
    if(pool!=NULL&& !(pool->is_legitimate(fault_addr))){
    flag=true;}
    if(flag){
        Console::puts("Page Not legitimate");
    }

  unsigned long* cur_page_dir = current_page_table->page_directory;
  unsigned long* page_table;
  //get the entry in page directory where page fault has occurred
  unsigned long page_dir_bits = fault_addr>>22;

  page_table=(unsigned long*)(cur_page_dir[page_dir_bits]&0xFFFFF000);
  //if the page fault is due to invalid page table then this segment will run
  if((cur_page_dir[page_dir_bits]& 1) == 0){
    //get a frame from kernel pool for page table
    page_table=(unsigned long*)((kernel_mem_pool->get_frames(1)*PAGE_SIZE)|3);
    cur_page_dir[page_dir_bits]=(unsigned long)page_table;
    //set all the page table entries to zero with R/W bit set and valid bit not set in supervisor mode
    for(int i =0;i<1024;i++){
        page_table[i] = 0|2;
    }
  }
  //the below code is executed every time if a page fault occurred
  unsigned long page_table_bits = (fault_addr>>12)&0x3ff;//get the page table entry where fault occurs
  page_table[page_table_bits]=(unsigned long)((process_mem_pool->get_frames(1) * PAGE_SIZE)|3);//set page table entry by getting frame from process pool

  }
  Console::puts("handled page fault\n");

}

void PageTable::register_pool(VMPool * _vm_pool)
{
    vmpool_list[vmpool_count] = _vm_pool;
    vmpool_count = vmpool_count+1;

    Console::puts("registered VM pool\n");
}

unsigned long* PageTable::PDE_address(unsigned long addr)
{
    //unsigned long pde_entry = addr>>22;
    //	return 0xFFFFF000 |(pde_entry);
    assert(false);
    return 0;
}

unsigned long* PageTable::PTE_address(unsigned long addr)
{
    unsigned long pte_entry = addr>>12;
    pte_entry = 0x03FF0000|pte_entry;
    
    unsigned long* pte_ptr = (unsigned long*) pte_entry;
    return pte_ptr;
    
}

void PageTable::free_page(unsigned long _page_no) {
    unsigned long pde_bits = _page_no>>22;
    unsigned long page_table_entry_number = (_page_no>>12)& 0x000003FF;
        //get the page no which is to be released using recursive page table lookup;
    unsigned long* page_table = (unsigned long*)((page_dir_bits*PAGE_SIZE)|0xffc00000);
    process_mem_pool->release_frames(page_table[page_table_entry_number]);
    write_cr3(read_cr3());

    // page table entry = page table number, left shift by 12 bits and make & operator with 0x000003FF;

    Console::puts("freed page\n");

}
