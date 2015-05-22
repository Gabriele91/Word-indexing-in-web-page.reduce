//
//  OpenCLProgram.h
//  Word-indexing-in-web-page
//
//  Created by Gabriele Di Bari on 10/04/15.
//  Copyright (c) 2015 Gabriele Di Bari. All rights reserved.
//
#pragma once

#include <Config.h>
#include <GCPointer.h>
#include <OpenCLKernel.h>
#include <OpenCLErrors.h>

//Forward declaration
class OpenCLInfo;
class OpenCLContext;
class OpenCLMemory;
class OpenCLQueue;

//Class declaration
class OpenCLProgram : public GCPointer< OpenCLProgram >
{
    //attributes
    cl_program m_program{ 0 };
    //private contructor
    OpenCLProgram(cl_program program)
    {
        m_program = program;
    }
    //friend class
    friend class OpenCLContext;
    friend class GCPointer< OpenCLProgram >;
    
public:
    
    ~OpenCLProgram()
    {
        clReleaseProgram(m_program);
    }
    
    cl_program get_program() const
    {
        return m_program;
    }
    
    operator cl_program () const
    {
        return get_program();
    }
    
    cl_int build(const std::vector< cl_device_id >& device_list,
                 const char *options = NULL,
                 void (CL_CALLBACK *pfn_notify)(cl_program, void *user_data) = NULL,
                 void *user_data = NULL)
    {
        return clBuildProgram(m_program,
                              (cl_uint)device_list.size(),
                              (const cl_device_id*)device_list.data(),
                              (const char*)options,
                              pfn_notify,
                              user_data);
    }
    
    cl_int build(const cl_device_id device,
                 const char *options = NULL,
                 void (CL_CALLBACK *pfn_notify)(cl_program, void *user_data) = NULL,
                 void *user_data = NULL)
    {
        return clBuildProgram(m_program,1,&device,options,pfn_notify,user_data);
    }
    
    cl_int build(cl_uint num_devices,
                 const cl_device_id *device_list,
                 const char *options = NULL,
                 void (CL_CALLBACK *pfn_notify)(cl_program, void *user_data) = NULL,
                 void *user_data = NULL)
    {
        return clBuildProgram(m_program,num_devices,device_list,options,pfn_notify,user_data);
    }
    
    static std::string debug_option(const std::string& path_source);
    
    static std::string include_option(const std::string& path_dir);
    
    std::string get_last_build_errors(cl_device_id  device) const
    {
        const size_t c_str_size = 4096;
        char  c_str[c_str_size] = "";
        clGetProgramBuildInfo(m_program,device,CL_PROGRAM_BUILD_LOG, c_str_size, c_str, NULL);
        return c_str;
    }
    
    cl_kernel raw_create_kernel(const std::string& kernel_name)
    {
        //error variable
        cl_int error;
        //create kernel
        cl_kernel kernel=clCreateKernel(m_program, kernel_name.c_str(), &error);
        //assert
        ASSERTCL( error );
        //return
        return kernel;
    }
    
    OpenCLKernel::ptr create_kernel(const std::string& kernel_name)
    {
        return (new OpenCLKernel(raw_create_kernel(kernel_name)))->shared();
    }
    
    
};