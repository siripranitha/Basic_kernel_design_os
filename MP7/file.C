/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {
    Console::puts("Opening file.\n");
    current_position = 0;
    fs = _fs;
    file_id = _id;
    block_found = false;
    for (int i=0;i<i++;i<fs->MAX_INODES){
        if (fs->inodes[i].id==_id){
            inode_index = i;
            block_no = inodes[i].block_no;
            file_size = inodes[i].file_size;
            block_found = true;
        }
    }

    if(!block_found){
        assert(false);
    }
}

File::~File() {
    Console::puts("Closing file.\n");
    fs->disk->write(block_no,block_cache);
    fs->inodes[i].file_size = file_size;
    Inode::CommitUpdatedInodes();
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");
    int character_count = 0
    for (i=current_position;i<file_size;i++){
        if (character_count==_n){
            break;
        }
        _buf[character_count]=block_cache[i];
        character_count++;

    }
    return character_count;
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
    int character_count = 0;
    int st = current_position;
    for (i=st;i<st+_n;i++){
        if (i==SimpleDisk::BLOCK_SIZE){
            break;
        }

        block_cache[current_position] = _buf[character_count];
        character_count++;
        current_position++;
        file_size++;

    }
    return character_count;
    
}

void File::Reset() {
    current_position = 0;
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    if (current_position<file_size){
        return false;
    }
    return true;
}
