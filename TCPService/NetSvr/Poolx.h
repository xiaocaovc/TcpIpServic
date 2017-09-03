#ifndef _POOLX_H_INCLUDED_
#define _POOLX_H_INCLUDED_

typedef struct plx_pool_s        plx_pool_t;

typedef struct {
	unsigned char         *last;     // 当前 pool 中用完的数据的结尾指针，即可用数据的开始指针
	unsigned char         *end;      // 当前 pool 数据库的结尾指针
	plx_pool_t            *next;     // 指向下一个 pool 的指针
	size_t                user;     // 已分配块数
	unsigned int          failed;   // 当前 pool 内存不足以分配的次数
} plx_pool_data_t;
struct plx_pool_s {
	plx_pool_data_t       d;        // 包含 pool 的数据区指针的结构体
	size_t                max;      // 当前 pool 最大可分配的内存大小（Bytes）
	plx_pool_t           *current;  // pool 当前正在使用的pool的指针
	plx_pool_t           *large;    // pool 中指向大数据快的指针（大数据快是指 size > max 的数据块）
	plx_pool_t           *largecurrent;  // pool 当前正在使用的pool的指针
};
// 创建一个pool
plx_pool_t *plx_create_pool(size_t size);
// 销毁内存池
void plx_destroy_pool(plx_pool_t *pool);
// 从内存池中获取一块内存(内存对齐)
void *plx_palloc(plx_pool_t *pool, size_t size);
// 从内存池中获取一块内存(内存对齐),并设置为0
void * plx_pcalloc(plx_pool_t *pool, size_t size);
// 释放
void plx_ralloc(plx_pool_t *pool, void *lpdata);

#endif /* _POOLX_H_INCLUDED_ */