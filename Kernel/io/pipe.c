#include "collections/pool.h"
#include <io/pipe.h>
#include <syncro/semaphore.h>
#include <wait.h>
#include <lib.h>

#define PIPE_SIZE 4096

struct PipeStream
{
	size_t start;
	size_t end;
	int readrefs;
	int writerefs;
	int count;
	int id;
	Semaphore *lock;
	Semaphore *available;
	Semaphore *freeSpace;
	char text[PIPE_SIZE];
};

struct FileDescriptorData
{
	bool isReadEnd;
	struct PipeStream *stream;
};

static Pool *pool;
static Semaphore *lock;

void pipe_init()
{
	pool = Pool_Create(sizeof(struct PipeStream));
	lock = sem_create(1);
}

static int readPipe(FileDescriptorData *data, void *buf, size_t count)
{
	char *out = buf;
	struct PipeStream *stream = data->stream;
	int i;

	sem_wait(stream->available);
	lock(stream->lock);
	for(i = 0; i < count && stream->count > 0; )
	{
		out[i++] = stream->text[stream->start++];
		stream->start %= PIPE_SIZE;
		stream->count--;
		if(stream->count == PIPE_SIZE-1)
			sem_release(stream->freeSpace);
	}
	if(stream->count > 0 || stream->writerefs == 0)
		sem_release(stream->available);
	unlock(stream->lock);

	return i;
}

static int writePipe(FileDescriptorData *data, const void *buf, size_t count)
{
	struct PipeStream *stream = data->stream;
	const char *in = buf;
	int i;

	sem_wait(stream->freeSpace);
	lock(stream->lock);
	if(stream->readrefs == 0)
	{
		unlock(stream->lock);
		sem_release(stream->freeSpace);
		return -1;
	}
	for(i = 0; i < count; )
	{
		if(stream->count == PIPE_SIZE)
		{
			if(stream->readrefs == 0)
			{
				unlock(stream->lock);
				return i == 0 ? i : -1;
			}
			unlock(stream->lock);
			sem_wait(stream->freeSpace);
			lock(stream->lock);
			continue;
		}
		stream->text[stream->end++] = in[i++];
		stream->end %= PIPE_SIZE;
		stream->count++;
		if(stream->count == 1)
			sem_release(stream->available);
	}
	if(stream->count < PIPE_SIZE)
		sem_release(stream->freeSpace);
	unlock(stream->lock);

	return i;
}

static int closePipe(FileDescriptorData *data)
{
	struct PipeStream *stream = data->stream;

	lock(stream->lock);
	if(data->isReadEnd)
	{
		stream->readrefs--;
		if(stream->readrefs == 0 && stream->count == PIPE_SIZE)
			sem_release(stream->freeSpace);
	}
	else
	{
		stream->writerefs--;
		if(stream->writerefs == 0 && stream->count == 0)
			sem_release(stream->available);
	}
	bool dispose = stream->writerefs == 0 && stream->readrefs == 0;
	unlock(stream->lock);
	kfree(data);

	if(dispose)
	{
		sem_close(stream->freeSpace);
		sem_close(stream->available);
		sem_close(stream->lock);
		lock(lock);
		Pool_Remove(pool, stream->id);
		unlock(lock);
	}
	return 0;
}

static bool dupPipe(const struct FileDescriptor *fd, struct FileDescriptor *newfd)
{
	FileDescriptorData *newdata = kmalloc(sizeof(struct FileDescriptorData));
	if(newdata == NULL)
        return false;

	memcpy(newdata, fd->data, sizeof(FileDescriptorData));
    memcpy(newfd, fd, sizeof(struct FileDescriptor));
	newfd->data = newdata;

	lock(newdata->stream->lock);
	if(newdata->isReadEnd)
		newdata->stream->readrefs++;
	else
		newdata->stream->writerefs++;
	unlock(newdata->stream->lock);
	
	return true;
}

bool openPipe(FileDescriptor *readfd, FileDescriptor *writefd)
{
	struct PipeStream *stream;

	lock(lock);
	{
		struct PipeStream tmp;
		int index = Pool_Add(pool, &tmp);
		stream = Pool_GetRef(pool, index);
		stream->id = index;
	}
	unlock(lock);

	if(stream == NULL)
		return false;
	stream->lock = sem_create(1);
	if(stream->lock == NULL)
		goto free_stream;
	stream->available = sem_create(0);
	if(stream->available == NULL)
		goto close_lock;
	stream->freeSpace = sem_create(1);
	if(stream->freeSpace == NULL)
		goto close_available;

	struct FileDescriptorData *readData = kmalloc(sizeof(struct FileDescriptorData));
	if(readData == NULL)
		goto close_space;
	*readData = (FileDescriptorData)
	{
		.isReadEnd = true,
		.stream = stream
	};

	struct FileDescriptorData *writeData = kmalloc(sizeof(struct FileDescriptorData));
	if(writeData == NULL)
		goto free_read;
	*writeData = (FileDescriptorData)
	{
		.isReadEnd = false,
		.stream = stream
	};

	*readfd = (FileDescriptor)
    {
        .data = readData,
        .read = readPipe,
        .close = closePipe,
        .dup = dupPipe,
        .isOpen = true,
    };

	*writefd = (FileDescriptor)
    {
        .data = writeData,
        .write = writePipe,
        .close = closePipe,
        .dup = dupPipe,
        .isOpen = true,
    };

	stream->writerefs = 1;
	stream->readrefs = 1;

	return true;

	free_read:
	kfree(readData);
	close_space:
	sem_close(stream->freeSpace);
	close_available:
	sem_close(stream->available);
	close_lock:
	sem_close(stream->lock);
	free_stream:
	kfree(stream);
	return false;
}

int pipe_list(PipeInfo *out, int n)
{
	int count = Pool_Count(pool);
	if(out == NULL)
		return count;
	
	lock(lock);
	int indexes[count];
	Pool_ToIndexArray(pool, indexes);
	for(int i = 0; i < count && i < n; i++)
	{
		struct PipeStream *stream = Pool_GetRef(pool, indexes[i]);
		out[i] = (PipeInfo)
		{
			.id = stream->id,
			.count = stream->count,
		};
		int size = sizeof(out[i].blocked) / sizeof(*out[i].blocked);
		SemaphoreStatus sem;
		sem_info(stream->available, &sem);
		out[i].blockedCount = sem.blockedCount < size ? sem.blockedCount : size;
		memcpy(out[i].blocked, sem.blocked, out[i].blockedCount);
	}
	unlock(lock);

	return count;
}