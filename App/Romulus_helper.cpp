/**
 * Author: xxx xxxx
 */

/* This module does mmaps/munmaps, and initializes/creates the persistent header and passes this to sgx-remus */

#include "Romulus_helper.h"

PersistentHeader *per_out{nullptr};

uint8_t *base_addr = (uint8_t *)0x7fdd40000000;
uint8_t *real_base_addr = (uint8_t *)(((size_t)(base_addr + 128) << 6) >> 6);
static int fd = -1;



/* Create file and do mmap */
void rom_init()
{

    struct stat buf;
    if (stat(MMAP_FILENAME, &buf) == 0)
    {
        printf("File exists..doing mmap\n");
        // File exists
        //std::cout << "Re-using memory region\n";
        fd = open(MMAP_FILENAME, O_RDWR | O_CREAT, 0755);
        assert(fd >= 0);
        // mmap() memory range
        uint8_t *got_addr = (uint8_t *)mmap(base_addr, MAX_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        if (got_addr == MAP_FAILED)
        {
            printf("got_addr = %p  %p\n", got_addr, MAP_FAILED);
            perror("ERROR: mmap() is not working !!! ");
            assert(false);
        }
        per_out = reinterpret_cast<PersistentHeader *>(real_base_addr - sizeof(PersistentHeader));
        if (per_out->id != MAGIC_ID){           
            create_file();
        } 
    }
    else
    {
        create_file();
    }
    //printf("mmapped file created\n");
}

void create_file()
{
    // File doesn't exist
    printf("File does not exist..create + mmap\n");
    fd = open(MMAP_FILENAME, O_RDWR | O_CREAT, 0755);
    assert(fd >= 0);
    if (lseek(fd, MAX_SIZE - 1, SEEK_SET) == -1)
    {
        perror("lseek() error");
    }
    if (write(fd, "", 1) == -1)
    {
        perror("write() error");
    }
    // mmap() memory range
    uint8_t *got_addr = (uint8_t *)mmap(base_addr, MAX_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
    if (got_addr == MAP_FAILED)
    {
        printf("got_addr = %p  %p\n", got_addr, MAP_FAILED);
        perror("ERROR: mmap() is not working !!! ");
        assert(false);
    }
    //per_out = new ((real_base_addr - sizeof(PersistentHeader))) PersistentHeader;
    per_out = (PersistentHeader*)(real_base_addr - sizeof(PersistentHeader));

    //the rest is done inside the enclave.
}

void close_file()
{
    printf("Closing mmapped file...\n");
    assert(fd >= 0);
    munmap(base_addr, MAX_SIZE);
    close(fd);
}