#include "stdafx.h"
#include "Poolx.h"

#define PLX_MAX_ALLOC_FROM_POOL 4095 // x86 页-> 4K
#define plx_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

static void *plx_palloc_block(plx_pool_t *pool, size_t size);
static void *plx_palloc_large(plx_pool_t *pool, size_t size);

plx_pool_t * plx_create_pool(size_t size)
{
	plx_pool_t  *p;

	p = (plx_pool_t*)malloc( size);
	if (p == NULL) 
	{
		return NULL;
	}
	p->d.last = (u_char *)p + sizeof(plx_pool_t);
	p->d.end = (u_char *)p + size;
	p->d.next = p;
	p->d.failed = 0;
	p->d.user = 0;

	size = size - sizeof(plx_pool_t);
	p->max = (size < PLX_MAX_ALLOC_FROM_POOL) ? size : PLX_MAX_ALLOC_FROM_POOL;
	p->current = p;
	p->large = NULL;
	p->largecurrent = NULL;

	return p;
}
void plx_destroy_pool(plx_pool_t *pool)
{
	plx_pool_t *p, *n;
	for (p=pool->largecurrent; p; p = n)
	{
		n = p->d.next;
		free(p);	
		if (n == pool->largecurrent)
		{
			break;
		}
	}
	for (p = pool; p; p = n)
	{
		n = p->d.next;
		free(p);
		if (n==pool)
		{
			break;
		}
	}
}

// 释放
void plx_ralloc(plx_pool_t *pool, void *lpdata)
{
	if (!lpdata)
	{
		return;
	}
	u_char* m = (u_char*)lpdata;
	m -= sizeof(u_char*);
	plx_pool_t* p = (plx_pool_t*)m;
	p->d.user -= 1;
	if (p->d.user == 0)
	{
		if (p->d.end - p->d.last <= pool->max)
		{
			p->d.last = (u_char *)p + sizeof(plx_pool_data_t);
			p->d.failed = 0;
			pool->current = p;
		}
		else
		{
			pool->largecurrent = p;
		}
	}
}

// 从内存池中获取一块内存(内存对齐)
void * plx_palloc(plx_pool_t *pool, size_t size)
{
	u_char      *m;
	plx_pool_t  *p,*n;
	size += sizeof(plx_pool_t*);
	if (size <= pool->max) 
	{
		for (p = pool->current; p&&p->d.failed < 4;)
		{
			m = plx_align_ptr(p->d.last, sizeof(unsigned long));
			if ((size_t)(p->d.end - m) >= size)
			{
				p->d.last = m + size;
				p->d.user += 1;
				*m = *(u_char*)p;
				//*((int*)m) = (int)p;
				m += sizeof(u_char*);
				return m;
			}
			p->d.failed++;
			p = p->d.next;
			if (p == pool->current)
			{
				break;
			}
		}
		return plx_palloc_block(pool, size);
	}
	return plx_palloc_large(pool, size);
}
// 加入新的data块
static void * plx_palloc_block(plx_pool_t *pool, size_t size)
{
	u_char      *m;
	size_t       psize;
	plx_pool_t   *newp;
	psize = (size_t)(pool->d.end - (u_char *)pool);
	m = (u_char*)malloc( psize);
	if (m == NULL)
	{
		return NULL;
	}
	newp = (plx_pool_t *)m;
	newp->d.end = m + psize;
	newp->d.next = NULL;
	newp->d.failed = 0;
	m += sizeof(plx_pool_data_t);
	newp->d.last = m + size;
	newp->d.user = 1;
	newp->d.next = pool->current->d.next;
	pool->current->d.next = newp;
	pool->current = newp;
	// m = plx_align_ptr(m, sizeof(unsigned long));
	*m = *(u_char*)newp;
	// *((int*)m) = (int)newp;
	m += sizeof(u_char*);
	return m;
}

// 进行大内存分配
static void * plx_palloc_large(plx_pool_t *pool, size_t size)
{
	TRACE(_T("----plx_palloc_large----\n！"));
	u_char              *p;
	plx_pool_t        *large;
	for (large = pool->largecurrent; large;)
	{
		if (0 == large->d.user)
		{
			if (size <= (size_t)(large->d.end - (u_char *)large)-sizeof(plx_pool_data_t))
			{
				large->d.user = 1;
				p = large->d.last;
				p = (u_char*)large;
				// *((int*)p) = (int)large;
				p += sizeof(plx_pool_t*);
				return p;
			}
		}
		large = large->d.next;
		if (large==pool->largecurrent)
		{
			break;
		}
	}
	size += sizeof(plx_pool_data_t);
	p = (u_char*)malloc(size);
	if (p == NULL)
	{
		return NULL;
	}
	large = (plx_pool_t*)p;
	large->d.end = p + size;
	p += sizeof(plx_pool_data_t);
	large->d.last = p ;
	large->d.next = NULL;
	large->d.failed = 0;
	large->d.user = 1;
	// 挂接
	if (pool->largecurrent)
	{
		large->d.next = pool->largecurrent->d.next;
		pool->largecurrent->d.next = large;
		pool->largecurrent= large->d.next;
	}
	else
	{
		pool->large = large;
		pool->largecurrent = large;
		large->d.next = large;
	}
	// *((int*)large->d.last) = (int)p;
	*p = *(u_char*)large;
	p += sizeof(u_char*);
	return p;
}
// 从内存池中获取一块内存(内存对齐),并设置为0
void * plx_pcalloc(plx_pool_t *pool, size_t size)
{
	void *p;
	p = plx_palloc(pool, size);
	if (p)
	{
		// zeromemory
		memset(p, 0, size);
	}
	return p;
}