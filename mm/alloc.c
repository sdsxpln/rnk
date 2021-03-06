/*
 * Copyright (C) 2014  Raphaël Poggi <poggi.raph@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stddef.h>
#include <printk.h>
#include <string.h>
#include "memory.h"

#ifdef CONFIG_TLSF
tlsf_t tlsf_mem_kernel_pool;
tlsf_t tlsf_mem_user_pool;
#endif /* CONFIG_TLSF */

#ifdef CONFIG_CUSTOM_MALLOC

int mem_alloc = 0;

static void *ralloc(struct memory_block *heap, unsigned int heap_size, unsigned int chunks, void *base)
{
	void *ret = NULL;
	int i = 0;
	int j = 0;
	int off;
	unsigned int mask;
	unsigned free_blocks = 0;
	unsigned int blocks;
	struct alloc_header *header = NULL;

	if (chunks < CHUNK_PER_BLOCK) {
		mask = MASK(chunks);

		for (i = 0; i < heap_size; i++) {
			if (heap[i].free_chunks >= chunks) {
				for (off = 0; off <= (CHUNK_PER_BLOCK - chunks); off++) {
					if (is_free(heap[i].free_mask, (mask << off))) {
						heap[i].free_mask |= (mask << off);
						heap[i].free_chunks -= chunks;
						ret = to_addr(i, off, base);
						goto out;
					}
				}
			} else {
				verbose_printk("not enough free space\r\n");
			}

		}
	} else {
		blocks = chunks / CHUNK_PER_BLOCK;
		chunks = chunks % CHUNK_PER_BLOCK;
		mask = MASK(chunks);

		for (i = 0; i < heap_size; i++) {
			if (free_blocks < blocks) {
				if (!heap[i].free_mask)
					free_blocks++;
				else
					free_blocks = 0;
			} else if (free_blocks == blocks) {
				if (!(heap[i].free_mask & mask)) {
					for (j = 1; j < free_blocks; j++) {
						heap[i - j].free_mask = -1;
						heap[i - j].free_chunks = 0;
					}
					heap[i].free_mask |= mask;
					heap[i].free_chunks -= chunks;

					ret = to_addr(i - free_blocks, 0, base);
					goto out;
				}
			} else {
				free_blocks = 0;
			}
			
		}	
	}

out:
	if (!ret)
		return ret;

	header = (struct alloc_header *)ret;
	header->magic = MAGIC;
	header->chunks = chunks;
	ret = (void *)((unsigned int *)ret + sizeof(struct alloc_header));

	return ret;
}

#endif /* CONFIG_CUSTOM_MALLOC */

#ifdef CONFIG_TLSF
static void alloc(size_t size, unsigned int *m, tlsf_t pool)
#else
static void alloc(size_t size, unsigned int *m)
#endif
{
	void *mem = NULL;
#ifdef CONFIG_CUSTOM_MALLOC
	int chunks;

	if (size > MAX_KERNEL_HEAP_SIZE)
		return NULL;

	size += sizeof(struct alloc_header);

	chunks = size + CHUNK_SIZE - 1;
	chunks = chunks / CHUNK_SIZE;

	mem = ralloc(kernel_heap, KERNEL_NUM_BLOCKS, chunks, (void *)KERNEL_HEAP_START);

	if (mem)
		mem_alloc += size;
#elif defined(CONFIG_TLSF)
	mem = tlsf_malloc(pool, size);
#endif
	*m = mem;
}

void umalloc(size_t size, void *m)
{
	unsigned int mem;
#ifdef CONFIG_TLSF
	alloc(size, &mem, tlsf_mem_kernel_pool);
#else
	alloc(size, &mem);
#endif

	*(unsigned int *)m = mem;
}

void *kmalloc(size_t size)
{
	unsigned int mem;

#ifdef CONFIG_TLSF
	alloc(size, &mem, tlsf_mem_kernel_pool);
#else
	alloc(size, &mem);
#endif

	return (void *)mem;
}
