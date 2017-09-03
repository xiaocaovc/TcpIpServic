#ifndef _POOLX_H_INCLUDED_
#define _POOLX_H_INCLUDED_

typedef struct plx_pool_s        plx_pool_t;

typedef struct {
	unsigned char         *last;     // ��ǰ pool ����������ݵĽ�βָ�룬���������ݵĿ�ʼָ��
	unsigned char         *end;      // ��ǰ pool ���ݿ�Ľ�βָ��
	plx_pool_t            *next;     // ָ����һ�� pool ��ָ��
	size_t                user;     // �ѷ������
	unsigned int          failed;   // ��ǰ pool �ڴ治���Է���Ĵ���
} plx_pool_data_t;
struct plx_pool_s {
	plx_pool_data_t       d;        // ���� pool ��������ָ��Ľṹ��
	size_t                max;      // ��ǰ pool ���ɷ�����ڴ��С��Bytes��
	plx_pool_t           *current;  // pool ��ǰ����ʹ�õ�pool��ָ��
	plx_pool_t           *large;    // pool ��ָ������ݿ��ָ�루�����ݿ���ָ size > max �����ݿ飩
	plx_pool_t           *largecurrent;  // pool ��ǰ����ʹ�õ�pool��ָ��
};
// ����һ��pool
plx_pool_t *plx_create_pool(size_t size);
// �����ڴ��
void plx_destroy_pool(plx_pool_t *pool);
// ���ڴ���л�ȡһ���ڴ�(�ڴ����)
void *plx_palloc(plx_pool_t *pool, size_t size);
// ���ڴ���л�ȡһ���ڴ�(�ڴ����),������Ϊ0
void * plx_pcalloc(plx_pool_t *pool, size_t size);
// �ͷ�
void plx_ralloc(plx_pool_t *pool, void *lpdata);

#endif /* _POOLX_H_INCLUDED_ */