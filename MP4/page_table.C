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
  
  page_directory = ( unsigned long*)(process_mem_pool->get_frames(1)*PAGE_SIZE);
  unsigned long *page_table = ( unsigned long*)(process_mem_pool->get_frames(1)*PAGE_SIZE);
   
    
  for(i=0; i<1024; i++){
    page_table[i] = address | WRITE_BIT | PRESENT_BIT ; 
    // attribute set to: supervisor level, read/write, present(011 in binary)
    address = address + PAGE_SIZE; // 4096 = 4kb
  };
  
  page_directory[0] = (unsigned long) page_table; 
  // entry of page directory set to page table
  //attribute set to: supervisor level, read/write, present(011 in binary)
  page_directory[0] = page_directory[0] | WRITE_BIT | PRESENT_BIT;

  page_directory[1023] = (unsigned long)page_directory;
  page_directory[1023] = page_directory[1023] | WRITE_BIT | PRESENT_BIT;
  


  for(i=1; i<1023; i++){
  page_directory[i] = 0 | WRITE_BIT ; 
  // attribute set to: supervisor level, read/write, not present(010 in binary)
  };

  
  
  vmpool_count = 0;
  for (i = 0;i<10;i++){
  vmpool_list[i]=NULL;
  }

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
  if(_r->err_code&1==1){
    Console::puts("protection fault");
    assert(false);
    return;
  }
  else{
  
  bool flag=false;
  unsigned int cnt;
  cnt = current_page_table->vmpool_count;
  
  for (int i=0;i<=cnt;i++){
        if (current_page_table->vmpool_list[i]!=NULL){

   	            if  (current_page_table->vmpool_list[i]->is_legitimate(fault_addr)){
   	            flag = true;
   	            break;
   	          }
            }
        }

  
  if(!flag){
        Console::puts("Page Not legitimate");
        assert(!flag);
    }
    


  
  
  unsigned long* cur_page_dir = PageTable::current_page_table->PDE_address(fault_addr);
  unsigned long pde_index = (fault_addr & 0xFFC00000) >> 22;
  unsigned long pte_index = (fault_addr & 0x003FF000) >> 12;
  
  bool frame_flag = false;
  if((*cur_page_dir & 0x1) != 0x1)                                                                        //PDE is not valid
	  {
		*cur_page_dir = (process_mem_pool->get_frames(1) << 12);                      //Allocating frame for a new page table
		*cur_page_dir = *cur_page_dir | 3;                                                                     // attribute set to: supervisor level, read/write, present(011 in binary) 
		frame_flag = true;
	  }
	  
	 unsigned long* page_table = (unsigned long *)(0xFFC00000 | (pde_index << 12));            //computing the logical address of page table; it is pte address multiple
  
  if(frame_flag)                                                                                               //If a new page table was created init the entries
	  {
		  for(unsigned int i=0; i<1024; i++)                                                             //Intialize every entry of page table
		  {
			*(page_table + i) = 0 | 2;                                                          // attribute set to: supervisor level, read/write, not present(010 in binary)
		  } 
	  }      
	  unsigned long fr = (process_mem_pool->get_frames(1)) << 12;       
	  *(page_table + pte_index) =  fr | 3;                                                     //Set the PTE entry to point to the actual physical frame
	  

  
  
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
    unsigned long pde_index = (addr & 0xFFC00000) >> 22;
    return (unsigned long *)(0xFFFFF000 + (pde_index << 2)); 
}

unsigned long* PageTable::PTE_address(unsigned long addr)
{
    unsigned long pte_entry = addr>>12;
    pte_entry = 0x03FF0000|pte_entry;
    
    unsigned long* pte_ptr = (unsigned long*) pte_entry;
    return pte_ptr;
    
}

void PageTable::free_page(unsigned long _page_no) {
    //unsigned long page_dir_bits = _page_no>>22;
    //unsigned long page_table_entry_number = (_page_no>>12)& 0x000003FF;
        //get the page no which is to be released using recursive page table lookup;
    //unsigned long* page_table = (unsigned long*)((page_dir_bits*PAGE_SIZE)|0xffc00000);
    //process_mem_pool->release_frames(page_table[page_table_entry_number]);
    //write_cr3(read_cr3());

    // page table entry = page table number, left shift by 12 bits and make & operator with 0x000003FF;

  unsigned long pde_index = (_page_no & 0xFFC00000) >> 22;
  unsigned long pte_index = (_page_no & 0x003FF000) >> 12;
  unsigned long *pt_entry = (unsigned long *)(0xFFC00000 | (pde_index << 12) | (pte_index << 2));
    
  if(*pt_entry & 0x1)                                                         
    {
      ContFramePool::release_frames(*pt_entry >> 12);     
      *pt_entry = *pt_entry & 0xFFFFFFFE;                 
      write_cr3((unsigned long)page_directory);           
    }
    Console::puts("freed page\n");

    
}
