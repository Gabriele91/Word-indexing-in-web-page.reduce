//
//  OpenCLInvertexIndex.h
//  Word-indexing-in-web-page
//
//  Created by Gabriele Di Bari on 19/05/15.
//  Copyright (c) 2015 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Config.h>
#include <SiteMapping.h>
#include <OpenCLBuffer.h>
#include <OpenCLQueue.h>
#include <OpenCLProgram.h>
#include <OpenCLKernel.h>
#include <OpenCLContext.h>
#define TYPE_COUNTER_IIMAP cl_uchar


//implementation
ASPACKED(struct MapsInfo
{

    cl_ushort m_row;
    cl_uint   m_size;
    cl_uint   m_offset;
    cl_uint   m_page;

    MapsInfo()
    {
        m_row = 0;
        m_size = 0;
        m_offset = 0;
        m_page = 0;
    }
});

class WordMapInvertedIndex : public GCPointer< WordMapInvertedIndex >
{
public:
    //class declaretrion
    class InvertedIndexMap;
    
    //implementation
    class InvertedIndexMap : public GCPointer< InvertedIndexMap >
    {
        //maps
        size_t m_count_maps = 0;
        //max size word
        size_t m_max_word_size = 0;
        //number of rows
        size_t m_count_row     = 0;
        //real number of row
        size_t m_real_count_row   = 0;
        //buffer
        std::vector< unsigned char, AlignedAllocator< unsigned char, ALIGNED_BIT > > m_bytes;
        //friend class
        friend class WordMapInvertedIndex;
        //void buffer
        void all_to_zero()
        {
            std::memset(m_bytes.data(), 0, m_bytes.size());
        }
    
    public:

        const unsigned char* data() const
        {
            return m_bytes.data();
        }
                
        size_t count_row() const
        {
            return  m_count_row;
        }
        
        size_t count_maps() const
        {
            return  m_count_maps;
        }
        
        size_t word_capacity() const
        {
            return  m_max_word_size;
        }
        
        const char* at(cl_uint i) const;
        
        const char* operator[](cl_uint i) const;
        
        void compue_real_size()
        {
            for( m_real_count_row=0;
                 m_real_count_row!=m_count_row && *at(m_real_count_row);
                 ++m_real_count_row);
        }
        
        size_t real_count_row() const
        {
            return  m_real_count_row;
        }
        
        OpenCLMemory::ptr create_buffer(OpenCLContext& context)
        {
            compue_real_size();
            return context.create_buffer(OpenCLMemory::READ_ONLY | OpenCLMemory::COPY_HOST_PTR,
                                         real_count_row()*word_capacity(),
                                         (void*)m_bytes.data());
        }
    };
    
    void init
    (
       OpenCLContext& context,
       OpenCLQueue::ptr queue,
       size_t start,
       size_t end,
       size_t word_len,
       const std::vector<  PageMapping::Map::ptr  >& maps,
       InvertedIndexMap::ptr last_iimap
    )
    {
        m_iimap=InvertedIndexMap::shared_new();
        //size of page
        m_iimap->m_count_maps= end - start;
        //alloc
        m_info.resize(m_iimap->m_count_maps);
        //size of maps
        cl_uint maps_size     =0;
        //max size word
        m_iimap->m_max_word_size=word_len;
        //row
        m_iimap->m_count_row  =0;
        //mapping
        for(size_t i=start; i!=end; ++i)
        {
            m_info[i - start].m_row = (cl_ushort)maps[i]->row_size();//size of a row
            m_info[i - start].m_size = (cl_uint)maps[i]->size();    //count rows
            m_info[i - start].m_offset = (cl_uint)maps_size;
            m_info[i - start].m_page = (cl_uint)i;
            //max count words
            m_iimap->m_count_row += maps[i]->size();
            //size of maps
            maps_size            += maps[i]->byte_size();
            //assert(maps[i]->byte_size());
        }
        //alloc buffer
        if NOT(last_iimap)
        {
            m_iimap->m_bytes.resize(m_iimap->word_capacity() * m_iimap->count_row());
            m_iimap->all_to_zero();
        }
        else
        {
            //inc size of row
            m_iimap->m_count_row += last_iimap->m_count_row;
            //alloc
            m_iimap->m_bytes.resize(m_iimap->word_capacity() * m_iimap->count_row());
            //copy old buffer
            std::memcpy(m_iimap->m_bytes.data(),
                        last_iimap->m_bytes.data(),
                        last_iimap->m_bytes.size() );
            //set to 0 new buffer
            std::memset( m_iimap->m_bytes.data()+last_iimap->m_bytes.size(),
                         0,
                         m_iimap->m_bytes.size()-last_iimap->m_bytes.size() );
        }
        //create buffer
        m_cl_info = context.create_buffer(OpenCLMemory::READ_WRITE | OpenCLMemory::COPY_HOST_PTR, m_info.size() * sizeof(MapsInfo), m_info.data());
        m_cl_res  = context.create_buffer(OpenCLMemory::READ_WRITE | OpenCLMemory::COPY_HOST_PTR, m_iimap->m_bytes.size(), m_iimap->m_bytes.data());
        m_cl_maps = context.create_buffer(OpenCLMemory::READ_WRITE, maps_size, NULL);
        //write all maps
        maps_size=0;
        //mapping
        for(size_t i=start; i!=end; ++i)
        {
            if (maps[i]->byte_size())
            {
                queue->write_buffer(*m_cl_maps,
                                    true,
                                    maps_size,
                                    maps[i]->byte_size(),
                                    maps[i]->data());
                maps_size+=maps[i]->byte_size();
            }
            DEBUGCODE(else
            {
                MESSAGE("Page["<<i<<"] Size 0");
            });
        }
        //params
        m_args[0] = (cl_uint)m_iimap->count_maps();
        m_args[1] = (cl_uint)m_iimap->word_capacity();
        m_args[2] = (cl_uint)m_iimap->count_row();
        m_cl_args  = context.create_buffer(OpenCLMemory::READ_WRITE | OpenCLMemory::COPY_HOST_PTR,sizeof(cl_uint)*3,m_args);
        //send values
        queue->flush();
    }

    
    void execute_task(OpenCLQueue::ptr  queue,
                      OpenCLKernel::ptr kernel)
    {
        const size_t cl_local_work_size = 1;
        const size_t cl_global_work_size = m_iimap->count_maps();
        //
        ASSERTCL(  kernel->set_arg(0, *m_cl_info) );
        ASSERTCL(  kernel->set_arg(1, *m_cl_maps) );
        ASSERTCL(  kernel->set_arg(2, *m_cl_res)  );
        ASSERTCL(  kernel->set_arg(3, *m_cl_args)  );
        //ouput params
        ASSERTCL_MSG(
                     queue->enqueue_ndrange_kernel(*kernel,
                                                   1,
                                                   0,
                                                   &cl_global_work_size,
                                                   &cl_local_work_size), "Error: Failed to execute");
    }
    
    InvertedIndexMap::ptr read_map(OpenCLQueue::ptr  queue)
    {
        queue->read_buffer(*m_cl_res, true, 0, m_iimap->m_bytes.size(), m_iimap->m_bytes.data());
        return m_iimap;
    }
    
    InvertedIndexMap::ptr get_map()
    {
        return m_iimap;
    }
    
protected:
    
    //big map
    OpenCLMemory::ptr m_cl_maps;
    OpenCLMemory::ptr m_cl_info;
    OpenCLMemory::ptr m_cl_res;
    OpenCLMemory::ptr m_cl_args;
    //kernel info
    cl_uint m_args[3]={ 0,0,0 };
    //info map
    InvertedIndexMap::ptr m_iimap;
    //last vector of info
    std::vector < MapsInfo > m_info;

    
};

class ColumMapInvertedIndex : public GCPointer< ColumMapInvertedIndex >
{
    
public:
    
    void init
    (
     OpenCLContext& context,
     OpenCLQueue::ptr queue,
     size_t start,
     size_t end,
     size_t word_len,
     const std::vector<  PageMapping::Map::ptr  >& maps,
     WordMapInvertedIndex::InvertedIndexMap::ptr& iimap
     )
    {
        assert( iimap );
        //size of page
        m_count_maps= end - start;
        //alloc
        m_info.resize(m_count_maps);
        //size of maps
        cl_uint maps_size     =0;
        //page start
        m_page_start = start ;
        //mapping
        for(size_t i=start; i!=end; ++i)
        {
            m_info[i - start].m_row    = (cl_ushort)maps[i]->row_size();//size of a row
            m_info[i - start].m_size   = (cl_uint)maps[i]->size();    //count rows
            m_info[i - start].m_offset = (cl_uint)maps_size;
            m_info[i - start].m_page   = (cl_uint)i;
            //size of maps
            maps_size            += maps[i]->byte_size();
            //assert(maps[i]->byte_size());
        }
        //alloc buffer
        m_value.resize(iimap->real_count_row()*m_count_maps);
        std::memset(m_value.data(), 0, m_value.size()*sizeof(cl_ushort));
        //create buffer
        m_cl_value = context.create_buffer(OpenCLMemory::READ_WRITE | OpenCLMemory::COPY_HOST_PTR, m_value.size()* sizeof(cl_ushort), m_value.data());
        m_cl_info  = context.create_buffer(OpenCLMemory::READ_WRITE | OpenCLMemory::COPY_HOST_PTR, m_info.size() * sizeof(MapsInfo),  m_info.data());
        m_cl_maps  = context.create_buffer(OpenCLMemory::READ_WRITE, maps_size, NULL);
        //write all maps
        maps_size=0;
        //mapping
        for(size_t i=start; i!=end; ++i)
        {
            if(maps[i]->byte_size())
            {
                queue->write_buffer(*m_cl_maps,
                                    true,
                                    maps_size,
                                    maps[i]->byte_size(),
                                    maps[i]->data());
                maps_size+=maps[i]->byte_size();
            }
            DEBUGCODE(else
            {
                MESSAGE("Page["<<i<<"] Size 0");
            });
        }
        //params
        m_args[0] = (cl_uint)m_count_maps;
        m_args[1] = (cl_uint)m_page_start;
        m_args[2] = (cl_uint)iimap->word_capacity();
        m_args[3] = (cl_uint)iimap->real_count_row();
        m_cl_args = context.create_buffer(OpenCLMemory::READ_WRITE | OpenCLMemory::COPY_HOST_PTR,sizeof(cl_uint)*4,m_args);
        //send values
        queue->flush();
    }
    
    void execute_task(OpenCLQueue::ptr  queue,
                      OpenCLKernel::ptr kernel,
                      OpenCLMemory::ptr cl_words,
                      cl_event&& this_event)
    {
        const size_t cl_local_work_size = 1;
        const size_t cl_global_work_size = m_count_maps;
        //
        ASSERTCL( kernel->set_arg(0, *m_cl_info) );
        ASSERTCL( kernel->set_arg(1, *m_cl_maps) );
        ASSERTCL( kernel->set_arg(2, *cl_words)  );
        ASSERTCL( kernel->set_arg(3, *m_cl_value));
        ASSERTCL( kernel->set_arg(4, *m_cl_args));
        //ouput params
        ASSERTCL_MSG(queue->enqueue_ndrange_kernel(*kernel,
                                                   1,
                                                   0,
                                                   &cl_global_work_size,
                                                   &cl_local_work_size,
                                                   0,
                                                   NULL,
                                                   &this_event), "Error: Failed to execute");
    }
    
    
    const std::vector< cl_ushort >& read_vector(OpenCLQueue::ptr  queue)
    {
        queue->read_buffer(*m_cl_value, true, 0, m_value.size()*sizeof(cl_ushort), m_value.data());
        return m_value;
    }
    
    const std::vector< cl_ushort >& get_vector() const
    {
        return m_value;
    }
    
protected:
    //big map
    OpenCLMemory::ptr m_cl_maps;
    OpenCLMemory::ptr m_cl_info;
    OpenCLMemory::ptr m_cl_value;
    OpenCLMemory::ptr m_cl_args;
    //kernel info
    cl_uint m_args[4]={ 0,0,0,0 };
    //last vector of info
    std::vector < MapsInfo > m_info;
    //vector of values
    std::vector < cl_ushort > m_value;
    size_t m_page_start;
    size_t m_count_maps;
};

class OpenCLInvertedIndex
{
public:
    
    void init(OpenCLContext& context)
    {
        //get devices
        m_devices=context.get_devices();
        //sort
        std::sort(m_devices.begin(), m_devices.end(),
        [](const cl_device_id& l_device,const cl_device_id& r_device) -> bool
        {
            auto l_info=OpenCLInfo::get_device_info(l_device);
            auto r_info=OpenCLInfo::get_device_info(r_device);
            // CPU < GPU
            if ( l_info.m_type == CL_DEVICE_TYPE_CPU &&
                r_info.m_type == CL_DEVICE_TYPE_GPU )
            {
                return false;
            }
            // SIZE LEFT < SIZE RIGHT
            return l_info.m_work_group_size > l_info.m_work_group_size;
        });
        //create options info
        std::string options = OpenCLProgram::include_option(m_include_path);
        //compile
        //-------------------------------------------------------------------------
        //load source
        std::string source_words = StringUtils::file_to_string(m_source_path_words);
        //program
        m_program_words = context.create_program(source_words);
        //compile
        if (m_program_words->build(m_devices, options.c_str()) != CL_SUCCESS)
        {
            std::cout << ("Error: Failed to build source\n");
            for (auto& device : m_devices)
            {
                std::cout << "Error from device:\n"
                    << OpenCLInfo::get_device_info(device).to_string()
                    << "\n"
                    << m_program_words->get_last_build_errors(device);
            }
            return;
        }
        //get kernel
        m_kernel_words = m_program_words->create_kernel(m_kernel_name_words);
        //-------------------------------------------------------------------------
       #if 1
        //
        //load source
        std::string source_pages = StringUtils::file_to_string(m_source_path_pages);
        //program
        m_program_pages = context.create_program(source_pages);
        //compile
        if (m_program_pages->build(m_devices, options.c_str()) != CL_SUCCESS)
        {
            std::cout << ("Error: Failed to build source\n");
            for (auto& device : m_devices)
            {
                std::cout << "Error from device:\n"
                    << OpenCLInfo::get_device_info(device).to_string()
                    << "\n"
                    << m_program_pages->get_last_build_errors(device);
            }
            return;
        }
        //get kernel
        m_kernel_pages = m_program_pages->create_kernel(m_kernel_name_pages);
        #endif
        //-------------------------------------------------------------------------
    }
    
    
   void execute_task(
                     OpenCLContext& context,
                     const  std::vector<  PageMapping::Map::ptr  >& maps,
                     WordMapInvertedIndex::InvertedIndexMap::ptr& iimap,
                     std::function<void(const std::vector< cl_ushort >& data,
                                        size_t words,
                                        size_t page_start,
                                        size_t n_page)> cb_save_fun
                    )
    {

        //info
        size_t word_len  = 0;
        size_t word_count = 0;
        //calc
        for(const auto& map: maps)
        {
            //max word size
            word_len    =  word_len > map->row_size() ? word_len : map->row_size();
            //max count words
            word_count += map->size();
        }
        //delete size of counter
        word_len-=sizeof(TYPE_COUNTER);
        //safe end word
        word_len += 1;
        //sequenciale part
        {
            WordMapInvertedIndex::InvertedIndexMap::ptr last;
            //device id
            size_t i_device = m_devices.size()-1;
            size_t i_size = m_kernel_words->get_work_goup_max_size(m_devices[i_device]);
            //compute size task
            size_t mem_list_word_size = word_len * word_count;
            size_t mem_max_task_word_size = 1048576 * 256 - mem_list_word_size;
            size_t tmp_size_task = 0;
            size_t tmp_index_map = 0;
            std::vector< size_t > size_tasks;
            //add start task
            size_tasks.push_back(0);
            //compute tasks
            for (const auto& map : maps)
            {
                //increment count
                ++tmp_index_map;
                //compute size task
                bool task_byte_size  = tmp_size_task + map->byte_size() <= mem_max_task_word_size;
                bool task_count_size = tmp_index_map - size_tasks[size_tasks.size() - 1] < i_size;
                if (task_byte_size && task_count_size)
                {
                    tmp_size_task += map->byte_size();
                }
                else
                {
                    tmp_size_task = 0;
                    size_tasks.push_back(tmp_index_map);
                }
            }
            //add end task
            size_tasks.push_back(tmp_index_map);
            //tasks
            for (size_t i = 1; i != size_tasks.size(); ++i)
            {
                //init task
                OpenCLQueue::ptr queue = context.create_queue(m_devices[i_device], NULL);
                auto iimap = WordMapInvertedIndex::shared_new();
                iimap->init(context, queue, size_tasks[i - 1], size_tasks[i], word_len, maps, last);
                //exec
                iimap->execute_task(queue, m_kernel_words);
                //read
                last = iimap->read_map(queue);
                //end
                queue->finish();
            }
            //save iimap
            iimap=last;
        }
        //alloc buff of words map
        OpenCLMemory::ptr word_buffer = iimap->create_buffer(context);
        //max size of a task
        const size_t mem_of_a_column = sizeof(cl_ushort) * iimap->real_count_row();
        const size_t mem_max_for_task = 1024 * 1024 * 512 / m_devices.size();
        
        //compute columns
        {
            //init
            size_t size =maps.size();
            size_t added=0;
            size_t done =0;
            
            
            //vector of columns
            std::vector< ColumMapInvertedIndex::ptr > v_cmii;
            v_cmii.resize(m_devices.size());
            
            //map
            for(auto& map_ptr:v_cmii) map_ptr =ColumMapInvertedIndex::shared_new();

            //events
            std::vector< cl_event > events;
            events.resize(m_devices.size());
            
            //works size
            std::vector< size_t > words_size;
            words_size.resize(m_devices.size());
            
            //page start
            std::vector< size_t > page_start;
            page_start.resize(m_devices.size());
            
            //queues
            std::vector< OpenCLQueue::ptr >    queues;
            queues.resize(m_devices.size());
            
            //init events
            std::memset(events.data(), 0, events.size()*sizeof(cl_event));
            
            //queues
            queues.resize(m_devices.size());
            
            //init queues write
            for(size_t i=0;i!=m_devices.size();++i)
            {
                queues[i]=context.create_queue(m_devices[i], NULL);
            }
            
            //for all
            while ( done!=size )
            {
                for(size_t i=0;i!=m_devices.size();++i)
                {
                    //init event status
                    cl_int event_status = CL_COMPLETE;
                    //event?
                    if(events[i])
                    {
                        size_t param_value_size_ret=0;
                        //reset
                        event_status = CL_QUEUED;
                        //get info
                        ASSERTCL_MSG(clGetEventInfo(events[i],
                                                    CL_EVENT_COMMAND_EXECUTION_STATUS,
                                                    sizeof(cl_int),
                                                    &event_status,
                                                    &param_value_size_ret), "Error: Failed to get event status");
                    }
                    //execute new instance
                    if(event_status == CL_COMPLETE)
                    {
                        //release event
                        if(events[i])
                        {
                            //end
                            ASSERTCL( clWaitForEvents(1, &events[i]) );
                            //release
                            clReleaseEvent(events[i]);
                            events[i] = NULL;
                            //wait
                            ASSERTCL( queues[i]->flush() );
                            //callback
                            cb_save_fun(v_cmii[i]->read_vector(queues[i]),
                                       (size_t)iimap->real_count_row(),
                                       (size_t)page_start[i],
                                        (size_t)words_size[i]);
                            //add
                            done += words_size[i];
                            //end
                            ASSERTCL( queues[i]->finish() );
                            //new queue
                            queues[i]=context.create_queue(m_devices[i], NULL);
                        }
                        //add a new work
                        if(added != size)
                        {
                            //init
                            size_t this_worker_size = m_kernel_pages->get_work_goup_max_size(m_devices[i]);
                            //compute max work items mem
                            size_t this_task_max_size = (mem_max_for_task - m_kernel_pages->get_local_mem_size(m_devices[i])) / mem_of_a_column;
                            //min
                            this_worker_size = std::min(this_task_max_size, std::min(this_worker_size, (size - added)));
                            //init
                            v_cmii[i]->init(context,
                                            queues[i],
                                            added,
                                            this_worker_size+added,
                                            word_len,
                                            maps,
                                            iimap);
                            //ececute task
                            v_cmii[i]->execute_task(queues[i],
                                                    m_kernel_pages,
                                                    word_buffer,
                                                    std::move(events[i]));
                            //flush
                            queues[i]->flush();
                            //start at
                            page_start[i] = added;
                            //done is...
                            added += this_worker_size;
                            //set
                            words_size[i] = this_worker_size;
                        }
                    }
                    
                }
            }
            //init queues read
            for(size_t i=0;i!=m_devices.size();++i)
            {
                queues[i]=context.create_queue(m_devices[i], NULL);
            }
            //wait all
            for(auto queue : queues)
            {
                ASSERTCL( queue->finish() );
            }
            
        }
    }
    
private:
    
    //list of device
    std::vector< cl_device_id > m_devices;
    OpenCLProgram::ptr m_program_words;
    OpenCLKernel::ptr  m_kernel_words;
    //
    OpenCLProgram::ptr m_program_pages;
    OpenCLKernel::ptr  m_kernel_pages;
    //include path
    const std::string  m_include_path = "script/";
    //source info
    const std::string  m_source_path_words = "script/inverted_index_words.cl";
    const std::string  m_kernel_name_words = "inverted_index_words";
    //source info 2
    const std::string  m_source_path_pages = "script/inverted_index_pages.cl";
    const std::string  m_kernel_name_pages = "inverted_index_pages";
    //maps
    std::vector<  PageMapping::Map::ptr  > m_maps;

};
