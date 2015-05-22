//
//  OpenCLKernel.h
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
class OpenCLProgram;

//Class declaration
class OpenCLKernel : public GCPointer< OpenCLKernel >
{
    //attributes
    cl_kernel m_kernel{ 0 };
    //private contructor
    OpenCLKernel(cl_kernel kernel)
    {
        m_kernel = kernel;
    }
    //info about kernel/device
    template< typename T >
    T get_info(cl_device_id device, cl_kernel_work_group_info info)
    {
        T out_value = 0;

        cl_int error = clGetKernelWorkGroupInfo
            (
            m_kernel,
            device,
            info,
            sizeof(T),
            (void*)&out_value,
            NULL
            );
        ASSERTCL(error);

        return out_value;
    }
    //friend class
    friend class OpenCLContext;
    friend class OpenCLProgram;
    friend class GCPointer< OpenCLKernel >;
    
public:
    
    ~OpenCLKernel()
    {
        clReleaseKernel(m_kernel);
    }
    
    cl_kernel get_kernel() const
    {
        return m_kernel;
    }
    
    operator cl_kernel () const
    {
        return get_kernel();
    }
    
    cl_int set_arg(size_t index,size_t size, const void* data)
    {
        return  clSetKernelArg( m_kernel, (cl_uint)index, size, data );
    }
    
    cl_int set_arg(size_t index, cl_mem data)
    {
        return  clSetKernelArg( m_kernel, (cl_uint)index, sizeof(cl_mem), &data );
    }
    
    template< typename T >
    cl_int set_arg_value(size_t index, T& value)
    {
        return  clSetKernelArg( m_kernel, (cl_uint)index, sizeof(T), &value );
    }


    size_t get_work_goup_max_size(cl_device_id device)
    {
        return get_info<size_t>(device, CL_KERNEL_WORK_GROUP_SIZE);
    }

    size_t get_prefered_work_goup_max_size_multiple(cl_device_id device)
    {
        return get_info<size_t>(device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE);
    }

    cl_uint get_private_mem_size(cl_device_id device)
    {
        return get_info<cl_ulong>(device, CL_KERNEL_PRIVATE_MEM_SIZE);
    }

    cl_uint get_local_mem_size(cl_device_id device)
    {
        return get_info<cl_ulong>(device, CL_KERNEL_LOCAL_MEM_SIZE);
    }
        
    std::vector< cl_int > set_args(const std::vector< cl_mem >& args)
    {
        std::vector< cl_int >  out_errors;
        out_errors.resize(args.size());
        
        for(size_t i=0; i!=args.size(); ++i)
        {
            out_errors[i]=clSetKernelArg( m_kernel, (cl_uint)i, sizeof(cl_mem), &args[i] );
        }
        
        return out_errors;
    }
};