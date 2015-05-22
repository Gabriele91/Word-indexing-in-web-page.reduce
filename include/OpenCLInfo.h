//
//  OpenCLDevice.h
//  Word-indexing-in-web-page
//
//  Created by Gabriele Di Bari on 10/04/15.
//  Copyright (c) 2015 Gabriele Di Bari. All rights reserved.
//
#pragma once

#include <Config.h>
#include <StringUtils.h>


class OpenCLInfo
{
    
public:

	struct InfoPlatform
	{
		std::string m_name;

		std::string to_string() const
		{
			return "Platform: " + m_name;
		}
	};
    
    struct InfoDevice
    {
        std::string    m_name;
        cl_uint        m_compute_units;
        size_t         m_work_group_size;
        cl_device_type m_type;

        std::string to_string() const
        {
            return "\tDevice: " + m_name + "\n"
                   "\tCompute Units: " + std::to_string(m_compute_units) + "\n"
                   "\tWork group size: " + std::to_string(m_work_group_size) + "\n"
                   "\tType device: " + to_string(m_type) ;
        }
        
        static std::string to_string(cl_device_type type)
        {
            switch(type)
            {
                case CL_DEVICE_TYPE_CPU: return "CPU";
                case CL_DEVICE_TYPE_GPU: return "GPU";
                case CL_DEVICE_TYPE_ACCELERATOR: return "ACCELERATOR";
                case CL_DEVICE_TYPE_DEFAULT: return "DEFAUL";
                default: return "";
            };
        }
    };
    
    static std::vector<cl_platform_id> get_id_platforms()
    {
        //get count of platforms
        cl_uint platform_id_count = 0;
        clGetPlatformIDs (0, nullptr, &platform_id_count);
        //get id of devices
        std::vector<cl_platform_id> platform_ids (platform_id_count);
        clGetPlatformIDs (platform_id_count, platform_ids.data (), nullptr);
        //return
        return platform_ids;
    }
    
    static std::vector<cl_device_id> get_id_devices(cl_platform_id platforms_id)
    {
        //get count of devices
        cl_uint device_id_count = 0;
        clGetDeviceIDs(platforms_id, CL_DEVICE_TYPE_ALL, 2, nullptr, &device_id_count);
        //get id of devices
        std::vector<cl_device_id> device_ids (device_id_count);
        clGetDeviceIDs(platforms_id, CL_DEVICE_TYPE_ALL, 2, device_ids.data(), nullptr);
        //return
        return device_ids;
    }

    static InfoPlatform get_platform_info(cl_platform_id platform)
	{
		//get name
		char buffer[255];
		std::memset(buffer, 0, 255);
		clGetPlatformInfo(platform, CL_PLATFORM_NAME, 254, buffer, NULL);
        //ouput value
        InfoPlatform out_value;
		out_value.m_name = buffer;
		out_value.m_name = StringUtils::trim(out_value.m_name);
        //return value
        return out_value;
	}
    
    static InfoDevice get_device_info(cl_device_id device)
    {
        //get name
		char buffer[255];
		std::memset(buffer, 0, 255);
        clGetDeviceInfo(device, CL_DEVICE_NAME, 254, buffer, NULL);
        //output object 
        InfoDevice out_value;
		out_value.m_name = buffer;
        out_value.m_name = StringUtils::trim(out_value.m_name);
        //get units count
        clGetDeviceInfo(device,
                        CL_DEVICE_MAX_COMPUTE_UNITS,
                        sizeof(out_value.m_compute_units),
                        &out_value.m_compute_units, NULL);
        //get work group size
        clGetDeviceInfo(device,
                        CL_DEVICE_MAX_WORK_GROUP_SIZE,
                        sizeof(out_value.m_work_group_size),
                        &out_value.m_work_group_size, NULL);
        //get type
        clGetDeviceInfo(device,
                        CL_DEVICE_TYPE,
                        sizeof(out_value.m_type),
                        &out_value.m_type, NULL);

        //return
        return out_value;
    }
    
    static std::string to_string()
    {
        std::stringstream stream;
        
        auto p_list = OpenCLInfo::get_id_platforms();
        
        for(auto platform_id : p_list)
        {
            auto d_list = OpenCLInfo::get_id_devices(platform_id);
            stream << OpenCLInfo::get_platform_info(platform_id).to_string() << "\n";
            
            for(auto device_id : d_list)
            {
                stream << std::endl;
                stream << OpenCLInfo::get_device_info(device_id).to_string() << "\n";
            }
            
            stream << std::endl;
        }
        
        return stream.str();
    }
    
    static const char *get_error_string(cl_int error)
    {
        switch(error)
        {
            // run-time and JIT compiler errors
            case 0: return "CL_SUCCESS";
            case -1: return "CL_DEVICE_NOT_FOUND";
            case -2: return "CL_DEVICE_NOT_AVAILABLE";
            case -3: return "CL_COMPILER_NOT_AVAILABLE";
            case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
            case -5: return "CL_OUT_OF_RESOURCES";
            case -6: return "CL_OUT_OF_HOST_MEMORY";
            case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
            case -8: return "CL_MEM_COPY_OVERLAP";
            case -9: return "CL_IMAGE_FORMAT_MISMATCH";
            case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
            case -11: return "CL_BUILD_PROGRAM_FAILURE";
            case -12: return "CL_MAP_FAILURE";
            case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
            case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
            case -15: return "CL_COMPILE_PROGRAM_FAILURE";
            case -16: return "CL_LINKER_NOT_AVAILABLE";
            case -17: return "CL_LINK_PROGRAM_FAILURE";
            case -18: return "CL_DEVICE_PARTITION_FAILED";
            case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
                
            // compile-time errors
            case -30: return "CL_INVALID_VALUE";
            case -31: return "CL_INVALID_DEVICE_TYPE";
            case -32: return "CL_INVALID_PLATFORM";
            case -33: return "CL_INVALID_DEVICE";
            case -34: return "CL_INVALID_CONTEXT";
            case -35: return "CL_INVALID_QUEUE_PROPERTIES";
            case -36: return "CL_INVALID_COMMAND_QUEUE";
            case -37: return "CL_INVALID_HOST_PTR";
            case -38: return "CL_INVALID_MEM_OBJECT";
            case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
            case -40: return "CL_INVALID_IMAGE_SIZE";
            case -41: return "CL_INVALID_SAMPLER";
            case -42: return "CL_INVALID_BINARY";
            case -43: return "CL_INVALID_BUILD_OPTIONS";
            case -44: return "CL_INVALID_PROGRAM";
            case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
            case -46: return "CL_INVALID_KERNEL_NAME";
            case -47: return "CL_INVALID_KERNEL_DEFINITION";
            case -48: return "CL_INVALID_KERNEL";
            case -49: return "CL_INVALID_ARG_INDEX";
            case -50: return "CL_INVALID_ARG_VALUE";
            case -51: return "CL_INVALID_ARG_SIZE";
            case -52: return "CL_INVALID_KERNEL_ARGS";
            case -53: return "CL_INVALID_WORK_DIMENSION";
            case -54: return "CL_INVALID_WORK_GROUP_SIZE";
            case -55: return "CL_INVALID_WORK_ITEM_SIZE";
            case -56: return "CL_INVALID_GLOBAL_OFFSET";
            case -57: return "CL_INVALID_EVENT_WAIT_LIST";
            case -58: return "CL_INVALID_EVENT";
            case -59: return "CL_INVALID_OPERATION";
            case -60: return "CL_INVALID_GL_OBJECT";
            case -61: return "CL_INVALID_BUFFER_SIZE";
            case -62: return "CL_INVALID_MIP_LEVEL";
            case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
            case -64: return "CL_INVALID_PROPERTY";
            case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
            case -66: return "CL_INVALID_COMPILER_OPTIONS";
            case -67: return "CL_INVALID_LINKER_OPTIONS";
            case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";
                
            // extension errors
            case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
            case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
            case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
            case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
            case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
            case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
            default: return "Unknown OpenCL error";
        }
    }
    
};