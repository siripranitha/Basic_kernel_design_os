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
  vmpool_count = 0;

   Console::puts("Initialized Paging System\n");

}

PageTable::PageTable()
{
  unsigned long address=0; 
  unsigned int i;
  
  page_directory = ( unsigned long*)(process_mem_pool->get_frames(1)*PAGE_SIZE);
  unsigned long *page_table = ( unsigned long*)(process_mem_pool->get_frames(1)*PAGE_SIZE);
   
    
  for(i=0; i<1024; i++){
    page_table[i] = address | WRITE_BIT | PRESENT_BIT | USER_LEVEL_BIT; 
    // attribute set to: supervisor level, read/write, present(011 in binary)
    address = address + 4096; // 4096 = 4kb
  };
  page_table[1023] = (unsigned long) page_table;
  page_table[1023] = page_table[1023] | PRESENT_BIT;
  // attribute should be supervisor level , read only , present - 001

  page_directory[0] = (unsigned long) page_table; 
  // entry of page directory set to page table
  //attribute set to: supervisor level, read/write, present(011 in binary)
  page_directory[0] = page_directory[0] | WRITE_BIT | PRESENT_BIT;


  for(i=1; i<1024; i++){
  page_directory[i] = 0 | WRITE_BIT |USER_LEVEL_BIT; 
  // attribute set to: supervisor level, read/write, not present(010 in binary)
  };

  page_directory[1023] = (unsigned long)page_directory;
  page_directory[1023] = page_directory[1023] | PRESENT_BIT;

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
  unsigned long error_code = _r->err_code;
  unsigned long page_fault_address = read_cr2();
  
  unsigned long *page_directory_current = PageTable::PDE_address(page_fault_address);
  unsigned long *page_table_containing_the_page = PageTable::PTE_address(page_fault_address);

  unsigned long page_table_entry_number = (_page_no>>12)& 0x000003FF;
  unsigned long page_directory_entry_number = (_page_no>>22)& 0x000003FF;

    
// is the address registered in vmpools? check..
  for (int i=0;i<=vmpool_count;i++){
    assert(vmpool_list[i]->is_legitimate(page_fault_address));
  }

  if (error_code & PRESENT_BIT == 0){
    // that means the page is not presennt. now to check whether the problem lies at page table level or page dir level
    // first check is to see whether the page directory entry corresponding to page address is valid.
    if (page_directory_current[page_directory_entry_number]&PRESENT_BIT==1){
      // page directory entry is present, that is page table is initialised, but page table entry is missing.
      page_table_containing_the_page[page_table_entry_number] = (PageTable::process_mem_pool->get_frames(1)*PAGE_SIZE)| WRITE_BIT | PRESENT_BIT|USER_LEVEL_BIT;
    } 
    
    }

    else{
      // we have to create a page dir entry and corresponding page table entry for the faulty address
        page_directory_current[page_directory_entry_number] = (unsigned long)(process_mem_pool->get_frames(1)*PAGE_SIZE)| WRITE_BIT | PRESENT_BIT|USER_LEVEL_BIT;
        //page_table_containing_the_page = (unsigned long *)(page_directory_current[fault_addr_page_dir_entry] & 0xFFFFF000);
        //IS THIS REQUIRED ? I DONT THINK SO??

        for(int i=0; i<1024; i++){
            page_table_containing_the_page[i] =  USER_LEVEL_BIT ;  // write bit not used bcoz directory?
              // attribute set to: user level
              };
        page_table_containing_the_page[1023] = (unsigned long) page_table_containing_the_page;
        page_table_containing_the_page[1023] = page_table_containing_the_page[1023] | PRESENT_BIT; //put it to supervisor level?

      page_table_containing_the_page[page_table_entry_number] = (PageTable::process_mem_pool->get_frames(1)*PAGE_SIZE)| WRITE_BIT | PRESENT_BIT;

    }

    return;



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
    unsigned long pde_entry = addr>>22;
    return 0xFFFFF000 |(pde_entry);
}

unsigned long* PageTable::PTE_address(unsigned long addr)
{
    unsigned long pte_entry = addr>>12;
    return 0x03FF0000|pte_entry;
    
}

void PageTable::free_page(unsigned long _page_no) {
    unsigned long * pte = PageTable::PTE_address(_page_no);
    unsigned long page_table_entry_number = (_page_no>>12)& 0x000003FF;
    // page table entry = page table number, left shift by 12 bits and make & operator with 0x000003FF;
    pte[page_table_entry_number] = 0|WRITE_BIT|USER_LEVEL_BIT //110(user level,can write, not present)
    // release frames in frame pool?

    //PageTable::load();
    // flush tlb here or in vm pool?

}
