#include <syncro/wait.h>
#include <stdarg.h>
#include <pid.h>

struct __PDNode {
	unsigned pid;
	struct __PDNode *next;
	struct __PDNode *prev;
};

struct __WaitHandle {
	struct __PDNode *first;
	struct __PDNode *last;
	bool disposed;
};

struct __PDNode *__initWaitEntry(WaitHandle *handle)
{
	struct __PDNode *node = kmalloc(sizeof(struct __PDNode));
	node->pid = getCurrentPid();
	node->next = NULL;
	return node;
}

void __addWaitEntry(WaitHandle *handle, struct __PDNode *node)
{
	if(handle->disposed)
		return;
	
	node->prev = handle->last;
	while(!__atomic_compare_exchange_n(&handle->last, &node->prev, node, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) ;
	if(node->prev != NULL)
		node->prev->next = node;
	else
		handle->first = node;
}

void __freeWaitEntry(WaitHandle *handle, struct __PDNode *node)
{
	if(node->prev != NULL)
		node->prev->next = node->next;
	if(node->next != NULL)
		node->next->prev = node->prev;
	struct __PDNode *tmp = node;
	__atomic_compare_exchange_n(&handle->last, &tmp, node->prev, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	tmp = node;
	__atomic_compare_exchange_n(&handle->first, &tmp, node->next, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	kfree(node);
}

void __freeWaitEntries(struct __PDNode *node, ...)
{
	va_list args;
	va_start(args, node);

	WaitHandle *handle;
	while((handle = va_arg(args, WaitHandle*)) != NULL)
		__freeWaitEntry(handle, node);
	va_end(args);
}
void __addWaitEntries(struct __PDNode *node, ...)
{
	va_list args;
	va_start(args, node);

	WaitHandle *handle;
	while((handle = va_arg(args, WaitHandle*)) != NULL)
		__addWaitEntry(handle, node);
	va_end(args);
}

void releaseOne(WaitHandle *handle)
{
	struct __PDNode *node = handle->first;
	while(node != NULL && !__atomic_compare_exchange_n(&handle->first, &node, node->next, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) ;
	if(node != NULL)
	{
		struct __PDNode *tmp = node;
		__atomic_compare_exchange_n(&handle->last, &tmp, node->next, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
		setProcessState(node->pid, PROCESS_RUNNING);
	}
}

void releaseAll(WaitHandle *handle)
{
	for(struct __PDNode *node = handle->first; node != NULL; node = node->next)
	{
		setProcessState(node->pid, PROCESS_RUNNING);
	}
}


WaitHandle *WaitHandle_Create()
{
	WaitHandle *handle = kmalloc(sizeof(struct __WaitHandle));
	*handle = (struct __WaitHandle){ 0 };
	return handle;
}

//Disposing a wait handle with processes waiting on it is invalid
void WaitHandle_Dispose(WaitHandle *handle)
{
	handle->disposed = true;
	releaseAll(handle);
	kfree(handle);
}