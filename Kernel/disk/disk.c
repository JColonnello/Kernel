#include "disk.h"
#include "bmfs/errno.h"
#include "bmfs/fs.h"
#include <naiveConsole.h>
#include <stddef.h>
#include <bmfs/bmfs.h>
#include <bmfs/file.h>
#include "host.h"
#include "lib.h"

#define SECTOR_SIZE 512

void disk_read(void *dest, uint32_t lba, size_t count)
{
    int i=0;
    while(count > 0)
    {
        size_t n = (count > 255) ? 255 : count;
        ata_lba_read(dest + 512 * i, lba + i, n);
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
        ata_lba_write(buf + 512 * i, lba + i, n);
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
} AtaDisk;

static AtaDisk data = (AtaDisk)
{
    .size = 6291456,
    .offset = 512,
    .pos = 0
};


int ata_seek(void *disk_ptr, bmfs_uint64 offset, int whence)
{
	AtaDisk *disk = (AtaDisk*)(disk_ptr);

    if (whence == BMFS_SEEK_SET)
	{
		if (offset > disk->size)
			disk->pos = disk->size;
		else
			disk->pos = disk->offset + offset;
	}
	else if (whence == BMFS_SEEK_END)
	{
		if (offset > disk->size - disk->offset)
			disk->pos = disk->offset;
		else
			disk->pos = disk->size - offset;
	}
	else
	{
		return BMFS_EINVAL;
	}

	return 0;
}

int ata_tell(void *disk_ptr, bmfs_uint64 *offset)
{
	AtaDisk *disk = (AtaDisk*)(disk_ptr);
    *offset = disk->pos;
    return 0;
}

int ata_read(void *disk_ptr, void *buf, bmfs_uint64 len, bmfs_uint64 *read_len)
{
	AtaDisk *disk = (AtaDisk*)(disk_ptr);

    if ((disk->pos + len) > disk->size)
		len = disk->size - disk->pos;

    size_t remain = len;
    
    //If current position is not sector alligned
    if((disk->pos % SECTOR_SIZE) != 0)
    {
        disk_read(disk->buffer, disk->pos / SECTOR_SIZE, 1);
        size_t segment = disk->pos % SECTOR_SIZE;
        size_t toRead = SECTOR_SIZE - segment;
        if(toRead > remain)
            toRead = remain;

        memcpy(buf, disk->buffer + segment, toRead);
        disk->pos += toRead;
        buf += toRead;
        remain -= toRead;
    }
    while(remain >= SECTOR_SIZE)
    {
        size_t nSectors = remain / SECTOR_SIZE;
        if(nSectors > 255)
            nSectors = 255;

        disk_read(buf, disk->pos / SECTOR_SIZE, nSectors);
        size_t bytes = nSectors * SECTOR_SIZE;
        disk->pos += bytes;
        remain -= bytes;
        buf += bytes;
    }
    //Read remaining
    if(remain)
    {
        disk_read(disk->buffer, disk->pos / SECTOR_SIZE, 1);
        memcpy(buf, disk->buffer, remain);
        disk->pos += remain;
    }
    
    *read_len = len;
	return 0;
}

static int ata_write(void *disk_ptr, const void *buf, bmfs_uint64 len, bmfs_uint64 *write_len);

struct BMFS bmfs;
struct BMFSDisk disk;

void diskInit()
{
    bmfs_disk_init(&disk);
    data.buffer = kmalloc(512);
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

int show_file(const char *path) 
{
   
    /* Initialize disk here. */

    /* Initialize file system header. */

    struct BMFSFile file;

    bmfs_file_init(&file);

    int err = bmfs_open_file(&bmfs, &file, path);
    if (err == BMFS_ENOENT) {
        ncPrint("Entry does not exist.\n");
    } else if (err == BMFS_EISDIR) {
        ncPrint("Entry is a directory.\n");
    } else if (err != 0) {
        ncPrint("Failed to open.\n");
        return -1;
    }

    bmfs_file_set_mode(&file, BMFS_FILE_MODE_READ);

    char buf[512];

    while (!bmfs_file_eof(&file)) 
    {

        bmfs_uint64 read_count = 0;

        err = bmfs_file_read(&file, buf, 512, &read_count);
        if (err != 0)
        {
            ncPrint("Failed to read.\n");
            ncPrintHex(err);
            break;
        }

        ncWrite(buf, read_count);
    }
    ncPrint("Done.\n");
    return 0;
}