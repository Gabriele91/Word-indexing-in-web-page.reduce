//
//  OpenCLContext.h
//  Word-indexing-in-web-page
//
//  Created by Gabriele Di Bari on 10/04/15.
//  Copyright (c) 2015 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Config.h>
#include <OpenCLBuffer.h>
#include <OpenCLQueue.h>
#include <OpenCLProgram.h>
#include <OpenCLErrors.h>

class OpenCLContext
{
    
    bool                        m_init   { false };
    cl_context                  m_context{ 0     };
    cl_platform_id              m_platform;
    std::vector< cl_device_id > m_devices;

    //get plathform from context
    cl_platform_id get_context_platform()
    {
        size_t error = 0;
        size_t property_size = 0; 
        //get properties size
        error = clGetContextInfo(m_context, CL_CONTEXT_PROPERTIES, 0, NULL, &property_size);
        //assert
        assert(error == CL_SUCCESS);
        //get properties
        std::vector< cl_context_properties > properties(property_size / sizeof(cl_context_properties));
        //get properties
        error = clGetContextInfo(m_context, CL_CONTEXT_PROPERTIES, property_size, properties.data(), NULL);
        //assert
        assert(error == CL_SUCCESS);
        //search platform id
        for (size_t i = 0; properties[i]; ++i)
        {
            if (properties[i] == CL_CONTEXT_PLATFORM)
            {
                return (cl_platform_id) properties[i + 1];
            }
        } 
        assert( 0 );
        return 0;
    }

    //error context callback
    static void CL_CALLBACK cl_error_callback(const char* error,
                                              const void* private_info,
                                              size_t size_data,
                                              void* data)
    {
        DEBUGCODE
        (
              MESSAGE( "OpenCL Error callback: \n" << error );
        );
    }
    
public:

    enum DeviceType
    {
        TYPE_GPU = CL_DEVICE_TYPE_GPU,
        TYPE_CPU = CL_DEVICE_TYPE_CPU,
        TYPE_ACCELERATOR = CL_DEVICE_TYPE_ACCELERATOR,
        TYPE_DEFAULT     = CL_DEVICE_TYPE_DEFAULT,
        TYPE_ALL = CL_DEVICE_TYPE_ALL,
    };

    OpenCLContext()
    {
    }
    
    OpenCLContext(cl_platform_id platform_id,const std::vector< cl_device_id >& list_device)
    {
        init(platform_id,list_device);
    }

    OpenCLContext(cl_platform_id platform_id, const cl_device_id device)
    {
        init(platform_id, device);
    }
    OpenCLContext(cl_platform_id platform_id, DeviceType type)
    {
        init(platform_id, type);
    }
    OpenCLContext(DeviceType type)
    {
        init(type);
    }
    
    virtual ~OpenCLContext()
    {
        release();
    }
    
    bool init(cl_platform_id platform_id,const std::vector< cl_device_id >& list_device )
    {
        //init params
        const cl_context_properties  properties[] =
        {
            CL_CONTEXT_PLATFORM, (cl_context_properties) platform_id,
            NULL
        };
        //error variable
        cl_int error;
        //create context
         m_context = clCreateContext(properties,
                                    (cl_uint)list_device.size(),
                                    (const cl_device_id*)list_device.data(),
                                    (void(CL_CALLBACK *)(const char *, const void *, size_t, void *))cl_error_callback,
                                    (void*)NULL,
                                    ( cl_int*)&error);
        //assert
        ASSERTCL( error );
        //save
        m_init = true;
        m_platform = platform_id;
        m_devices = list_device;
        //resturn status
        return error == CL_SUCCESS;
    }
    
    bool init(cl_platform_id platform_id,const cl_device_id device)
    {
        //init params
        const cl_context_properties properties[] =
        {
            CL_CONTEXT_PLATFORM, (cl_context_properties) platform_id,
            NULL
        };
        //error variable
        cl_int error;
        //create context
        m_context = clCreateContext(properties,
                                    (cl_uint)1,
                                    (const cl_device_id*)&device,
                                    (void(CL_CALLBACK *)(const char *, const void *, size_t, void *))cl_error_callback,
                                    (void*)NULL,
                                    (cl_int*)&error);
        //assert
        ASSERTCL( error );
        //save
        m_init = true;
        m_platform = platform_id;
        m_devices.resize(1);
        m_devices[0] = device;
        //resturn status
        return error == CL_SUCCESS;
    }
    
    bool init(cl_platform_id platform_id, DeviceType type)
    {
        //init params
        const cl_context_properties properties[] =
        {
            CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id,
            NULL
        };
        //error variable
        cl_int error;
        //create context
        m_context = clCreateContextFromType(properties,
                                            (cl_device_type)type,
                                            (void(CL_CALLBACK *)(const char *, const void *, size_t, void *))cl_error_callback,
                                            (void*)NULL,
                                            (cl_int*)&error);
        //assert
        ASSERTCL( error );
        //get devices size
        size_t n_byte_devices = 0;
        error = clGetContextInfo(m_context, CL_CONTEXT_DEVICES, 0, NULL, &n_byte_devices);
        ASSERTCL( error );
        //get devices
        m_devices.resize(n_byte_devices / sizeof(cl_device_id));
        error = clGetContextInfo(m_context, CL_CONTEXT_DEVICES, n_byte_devices, m_devices.data(), NULL);
        ASSERTCL( error );
        //save
        m_init = true;
        m_platform = platform_id;
        //resturn status
        return error == CL_SUCCESS;
    }

    bool init(DeviceType type)
    {
        //error variable
        cl_int error;
        //create context
        //try all platform
        auto platforms = OpenCLInfo::get_id_platforms();
        for (auto platform_id : platforms)
        {
            cl_context_properties properties[] =
            {
                CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id,
                NULL
            };
            //try
            m_context = clCreateContextFromType(properties,
                                                (cl_device_type)type,
                                                (void(CL_CALLBACK *)(const char *, const void *, size_t, void *))cl_error_callback,
                                                (void*)NULL,
                                                (cl_int*)&error);
            //if no error...
            if (error == CL_SUCCESS) break;
        }
        //assert
        assert(m_context != NULL);
        //get platform
        m_platform = get_context_platform();
        //get devices size
        size_t n_byte_devices = 0;
        error = clGetContextInfo(m_context, CL_CONTEXT_DEVICES, 0, NULL, &n_byte_devices);
        ASSERTCL( error );
        //get devices
        m_devices.resize(n_byte_devices / sizeof(cl_device_id));
        error = clGetContextInfo(m_context, CL_CONTEXT_DEVICES, n_byte_devices, m_devices.data(), NULL);
        ASSERTCL( error );
        //save
        m_init = true;
        //resturn status
        return error == CL_SUCCESS;
    }

    const std::vector< cl_device_id >& get_devices() const
    {
        return m_devices;
    }
    cl_platform_id get_platform() const
    {
        return m_platform;
    }

    void release()
    {
        assert( m_init );
        //delete
        clReleaseContext(m_context);
        //save
        m_init = false;
    }
    
    cl_command_queue raw_create_queue(cl_device_id device, cl_command_queue_properties properties) const
    {
        cl_int error;
        cl_command_queue queue= clCreateCommandQueue(m_context, device, properties, &error);
        ASSERTCL( error );
        return queue;
    }
    OpenCLQueue::ptr create_queue(cl_device_id device, cl_command_queue_properties properties) const
    {
        return (new OpenCLQueue(raw_create_queue(device,properties)))->shared();
    }
    
    cl_mem raw_create_buffer(cl_mem_flags flags, size_t size, void* data) const
    {
        cl_int error;
        cl_mem alloc= clCreateBuffer(m_context, flags, size, data, &error);
        ASSERTCL( error );
        return alloc;
    }
    
    OpenCLMemory::ptr create_buffer(cl_mem_flags flags, size_t size, void* data) const
    {
        return OpenCLMemory::ptr(new OpenCLMemory(raw_create_buffer(flags, size, data)));
    }
    
    cl_program raw_create_program(const std::string& source) const
    {
        const size_t source_size[]=
        {
            source.size()
        };
        const char *sources[]=
        {
            &source[0]
        };
        cl_int error;
        cl_program program = clCreateProgramWithSource(m_context, 1, sources, source_size, &error);
        ASSERTCL( error );
        return program;
    }
    
    OpenCLProgram::ptr create_program(const std::string& source) const
    {
        return (new OpenCLProgram(raw_create_program(source)))->shared();
    }
    
    const cl_context& get_context() const
    {
        return m_context;
    }
    
    
    
    
};
