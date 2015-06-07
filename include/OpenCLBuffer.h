//
//  OpenCLBuffer.h
//  Word-indexing-in-web-page
//
//  Created by Gabriele Di Bari on 10/04/15.
//  Copyright (c) 2015 Gabriele Di Bari. All rights reserved.
//
#pragma once

#include <Config.h>
#include <GCPointer.h>
#include <OpenCLErrors.h>

//Forward declaration
class OpenCLInfo;
class OpenCLContext;
class OpenCLMemory;

//Class declaration
class OpenCLMemory : public GCPointer< OpenCLMemory >
{
    //attributes
    cl_mem m_memory{ 0 };
    //private contructor
    OpenCLMemory(cl_mem memory)
    {
        m_memory = memory;
    }
    //friend class
    friend class OpenCLContext;
    friend class GCPointer< OpenCLMemory >;

    
public:
    
    enum TypeMemory
    {
        //BUFFER MODE
        READ_ONLY  = CL_MEM_READ_ONLY,
        WRITE_ONLY = CL_MEM_WRITE_ONLY,
        READ_WRITE = CL_MEM_READ_WRITE,
        //STORAGE
        USE_HOST_PTR   = CL_MEM_USE_HOST_PTR,
        ALLOC_HOST_PTR = CL_MEM_ALLOC_HOST_PTR,
        //INIT
        COPY_HOST_PTR  = CL_MEM_COPY_HOST_PTR
    };
    
    ~OpenCLMemory()
    {
        clReleaseMemObject(m_memory);
    }
    
    cl_mem get_memory() const
    {
        return m_memory;
    }
    
    operator cl_mem () const
    {
        return get_memory();
    }
    
    
};