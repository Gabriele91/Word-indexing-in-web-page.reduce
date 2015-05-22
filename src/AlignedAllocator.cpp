//
//  AlignedAllocator.cpp
//  Word-indexing-in-web-page
//
//  Created by Gabriele Di Bari on 29/04/15.
//  Copyright (c) 2015 Gabriele Di Bari. All rights reserved.
//

#include <AlignedAllocator.h>
#include <memory>

#if _WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#endif

void* Aligned::aligned_alloc(std::size_t n, std::size_t alignment)
{
#ifdef _WIN32
	void * const pv = _mm_malloc(n, alignment);
#else
    void * pv = NULL;
    posix_memalign(&pv, alignment, n);
#endif
    return (void*)pv;
}
void Aligned::aligned_free(void * p)
{
#if _WIN32
    _mm_free(p);
#else
    free(p);
#endif
}