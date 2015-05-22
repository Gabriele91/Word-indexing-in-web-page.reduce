//
//  OpenCLQueue.h
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
class OpenCLQueue;

//Class declaration
class OpenCLQueue : public GCPointer< OpenCLQueue >
{
    //attributes
    cl_command_queue m_cmd_queue{ 0 };
    //private contructor
    OpenCLQueue(cl_command_queue cmd_queue)
    {
        m_cmd_queue = cmd_queue;
    }
    //friend class
    friend class OpenCLContext;
    friend class GCPointer< OpenCLQueue >;
    
public:
    
    enum TypeQueue
    {
        OUT_OF_ORDER_EXEC_MODE_ENABLE  = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
        PROFILING_ENABLE               = CL_QUEUE_PROFILING_ENABLE
    };
    
    ~OpenCLQueue()
    {
        clReleaseCommandQueue(m_cmd_queue);
    }
    
    cl_command_queue get_queue() const
    {
        return m_cmd_queue;
    }
    
    operator cl_command_queue () const
    {
        return get_queue();
    }
    
    cl_int write_buffer(cl_mem buffer,
                        cl_bool blocking_write,
                        size_t offset,
                        size_t cb,
                        const void *ptr,
                        cl_uint num_events_in_wait_list = 0,
                        const cl_event *event_wait_list = NULL,
                        cl_event *event = NULL)
    {
        return clEnqueueWriteBuffer
        (
            m_cmd_queue,
            buffer,
            blocking_write,
            offset,
            cb,
            ptr,
            num_events_in_wait_list,
            event_wait_list,
            event
        );
    }
    
    cl_int read_buffer(cl_mem buffer,
                       cl_bool blocking_read,
                       size_t offset,
                       size_t cb,
                       void *ptr,
                       cl_uint num_events_in_wait_list = 0,
                       const cl_event *event_wait_list = NULL,
                       cl_event *event = NULL)
    {
        return clEnqueueReadBuffer
        (
             m_cmd_queue,
             buffer,
             blocking_read,
             offset,
             cb,
             ptr,
             num_events_in_wait_list,
             event_wait_list,
             event
         );
    }
    
    
    cl_int copy_buffer(cl_mem src_buffer,
                       cl_mem dst_buffer,
                       size_t src_offset,
                       size_t dst_offset,
                       size_t cb,
                       cl_uint num_events_in_wait_list = 0,
                       const cl_event *event_wait_list = NULL,
                       cl_event *event = NULL)
    {
        return clEnqueueCopyBuffer
        (
             m_cmd_queue,
             src_buffer,
             dst_buffer,
             src_offset,
             dst_offset,
             cb,
             num_events_in_wait_list,
             event_wait_list,
             event
        );
    }
    
    cl_int enqueue_ndrange_kernel(cl_kernel kernel,
                                  cl_uint work_dim,
                                  const size_t *global_work_offset,
                                  const size_t *global_work_size,
                                  const size_t *local_work_size,
                                  cl_uint num_events_in_wait_list = 0,
                                  const cl_event *event_wait_list = NULL,
                                  cl_event *event = NULL)
    {
        return clEnqueueNDRangeKernel
        ( 
           m_cmd_queue, 
           kernel,
           work_dim,
           global_work_offset,
           global_work_size,
           local_work_size,
           num_events_in_wait_list,
           event_wait_list,
           event
        );
    }
    
    cl_int flush()
    {
        return clFlush(m_cmd_queue);
    }
    
    cl_int finish()
    {
        return clFinish(m_cmd_queue);
    }
    
};