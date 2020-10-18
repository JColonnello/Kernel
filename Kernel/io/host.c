#include <bmfs/host.h>
#include <lib.h>

struct BMFSHostData
{
	/* Nothing required here yet. */
};

struct BMFSHostData *host_init(void)
{
	struct BMFSHostData *host_data = kmalloc(sizeof(*host_data));
	if (host_data == NULL)
		return BMFS_NULL;

	return host_data;
}

void host_done(struct BMFSHostData *host_data)
{
	if (host_data == BMFS_NULL)
		return;

	kfree(host_data);
}

static void *host_malloc(struct BMFSHostData *host_data,bmfs_uint64 size)
{
	if (host_data == BMFS_NULL)
		return BMFS_NULL;

	void *addr = kmalloc(size);
	if (addr == NULL)
		return BMFS_NULL;

	return addr;
}

static void host_free(struct BMFSHostData *host_data,void *addr)
{
    if (host_data == BMFS_NULL)
		return;
    kfree(addr);
}

const struct BMFSHost bmfs_host = 
{
    .Init = host_init,
    .Done = host_done,
    .Malloc = host_malloc,
    .Free = host_free
};