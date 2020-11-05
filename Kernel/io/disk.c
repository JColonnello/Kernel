#include "disk.h"
#include "bmfs/errno.h"
#include "bmfs/fs.h"
#include <naiveConsole.h>
#include <stddef.h>
#include <bmfs/bmfs.h>
#include <bmfs/file.h>
#include "host.h"
#include "lib.h"
#include "syncro/semaphore.h"
#include "syslib.h"
#include <io/files.h>

#define SECTOR_SIZE 512

void disk_read(void *dest, uint32_t lba, size_t count)
{
    int i=0;
    while(count > 0)
    {
        size_t n = (count > 255) ? 255 : count;
        ata_lba_read((char*)dest + 512 * i, lba + i, n);
        i += n;
        count -= n;
    }
}

void disk_write(const void *buf, uint32_t lba, size_t count)
{
    int i=0;
    while(count > 0)
    {
        size_t n = (count > 255) ? 255 : count;
        ata_lba_write((char*)buf + 512 * i, lba + i, n);
        i += n;
        count -= n;
    }
}

typedef struct
{
    bmfs_uint64 size;
    bmfs_uint64 pos;
    bmfs_uint64 offset;
    void *buffer;
    Semaphore *lock;
} AtaDisk;

static AtaDisk data =
{
    .size = 67108864,
    .offset = 512,
    .pos = 0
};


int ata_seek(void *disk_ptr, bmfs_uint64 offset, int whence)
{
	AtaDisk *disk = (AtaDisk*)(disk_ptr);
    lock(disk->lock);

    if (whence == BMFS_SEEK_SET)
	{
		if (offset > disk->size)
			disk->pos = disk->size;
		else
			disk->pos = offset;
	}
	else if (whence == BMFS_SEEK_END)
	{
		if (offset > disk->size)
			disk->pos = 0;
		else
			disk->pos = disk->size - offset;
	}
	else
	{
		return BMFS_EINVAL;
	}

    unlock(disk->lock);
	return 0;
}

int ata_tell(void *disk_ptr, bmfs_uint64 *offset)
{
	AtaDisk *disk = (AtaDisk*)(disk_ptr);
    *offset = disk->pos;
    return 0;
}

int ata_read(void *disk_ptr, void *buffer, bmfs_uint64 len, bmfs_uint64 *read_len)
{
	AtaDisk *disk = (AtaDisk*)(disk_ptr);
    char *buf = buffer;
    lock(disk->lock);

    if ((disk->pos + len) > disk->size)
		len = disk->size - disk->pos;

    size_t remain = len;
    
    //If current position is not sector alligned
    if((disk->pos % SECTOR_SIZE) != 0)
    {
        disk_read(disk->buffer, (disk->pos + disk->offset) / SECTOR_SIZE, 1);
        size_t segment = disk->pos % SECTOR_SIZE;
        size_t toRead = SECTOR_SIZE - segment;
        if(toRead > remain)
            toRead = remain;

        memcpy(buf, (char*)disk->buffer + segment, toRead);
        disk->pos += toRead;
        buf += toRead;
        remain -= toRead;
    }
    if(remain >= SECTOR_SIZE)
    {
        size_t nSectors = remain / SECTOR_SIZE;

        disk_read(buf, (disk->pos + disk->offset) / SECTOR_SIZE, nSectors);
        size_t bytes = nSectors * SECTOR_SIZE;
        disk->pos += bytes;
        remain -= bytes;
        buf += bytes;
    }
    //Read remaining
    if(remain)
    {
        disk_read(disk->buffer, (disk->pos + disk->offset) / SECTOR_SIZE, 1);
        memcpy(buf, disk->buffer, remain);
        disk->pos += remain;
    }
    
    unlock(disk->lock);
    *read_len = len;
	return 0;
}

//static int ata_write(void *disk_ptr, const void *buf, bmfs_uint64 len, bmfs_uint64 *write_len);

struct BMFS bmfs;
struct BMFSDisk disk;

void diskInit()
{
    bmfs_disk_init(&disk);
    data.buffer = kmalloc(SECTOR_SIZE);
    data.lock = sem_create(1);
    disk.DiskPtr = &data;
    disk.seek = ata_seek;
    disk.read = ata_read;
    disk.tell = ata_tell;

    bmfs_init(&bmfs);
    bmfs_set_disk(&bmfs, &disk);
    bmfs_set_host(&bmfs, &bmfs_host);

    int err = bmfs_import(&bmfs);
    if (err != 0) 
    {
        ncPrint("Failed to import BMFS file system.\n");
        return;
    }
}

static int readFile(FileDescriptorData *data, void *buf, size_t count)
{
    struct BMFSFile *file = (void*)data;
    bmfs_uint64 read_count;
    int err = bmfs_file_read(file, buf, count, &read_count);
    if(err)
        return err;
    return read_count;
}

static int writeFile(FileDescriptorData *data, const void *buf, size_t count)
{
    struct BMFSFile *file = (void*)data;
    bmfs_uint64 read_count;
    int err = bmfs_file_write(file, buf, count, &read_count);
    if(err)
        return err;
    return read_count;
}

static int closeFile(FileDescriptorData *data)
{
    struct BMFSFile *file = (void*)data;
    bmfs_file_close(file);
    kfree(data);
    return 0;
}


int openFile(FileDescriptor *fd, const char *path, int mode)
{
    struct BMFSFile *file = kmalloc(sizeof(struct BMFSFile));
    bmfs_file_init(file);
    int err = bmfs_open_file(&bmfs, file, path);
    if(err != 0)
    {
        kfree(file);
        return err;
    }
    bmfs_file_set_mode(file, BMFS_FILE_MODE_RW);


    *fd = (FileDescriptor)
    {
        .data = (FileDescriptorData*)file,
        .read = readFile,
        .write = writeFile,
        .close = closeFile
    };
    return 0;
}
