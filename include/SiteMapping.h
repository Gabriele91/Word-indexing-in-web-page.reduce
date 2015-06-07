#pragma once

#include <Config.h>
#include <algorithm>
#include <OpenCLContext.h>
#include <OpenCLBuffer.h>
#include <OpenCLQueue.h>
#include <OpenCLKernel.h>
#include <WebSite.h>
#include <AlignedAllocator.h>
//define
#define ALIGNED_BIT 64
#define TYPE_COUNTER cl_ushort
//class declaretrion
class CLMapMemory;
class PageMapping;
class WebSiteMapping;

struct SizeBuffers
{
    unsigned int m_info_size { 0 };
    unsigned int m_text_size { 0 };
    unsigned int m_rows_size { 0 };
    
    SizeBuffers(){}
    SizeBuffers(unsigned int info_size,
                unsigned int text_size,
                unsigned int rows_size)
    {
        m_info_size=info_size;
        m_text_size=text_size;
        m_rows_size=rows_size;
    }
    
    SizeBuffers& operator +=  (const SizeBuffers& size)
    {
        m_info_size+=size.m_info_size;
        m_text_size+=size.m_text_size;
        m_rows_size+=size.m_rows_size;
        return *this;
    }
    
    SizeBuffers operator + (const SizeBuffers& size) const
    {
        return
        {
            m_info_size+size.m_info_size,
            m_text_size+size.m_text_size,
            m_rows_size+size.m_rows_size
        };
        
    }
};

struct GlobalInfo
{
    cl_uint m_pages    { 0    };
    cl_uint m_cl_start { 0    };
    cl_uint m_start    { 0    };
    cl_uint m_end      { 0    };
    cl_uint m_info_size{ 0    };
    cl_uint m_min_len  { 0    };
    cl_bool m_case_sen { true };
    GlobalInfo(){}
    GlobalInfo(cl_uint pages,
               cl_uint cl_start,
               cl_uint start,
               cl_uint end,
               cl_uint info_size,
               cl_uint min_len,
               cl_bool case_sen)
    {
        m_pages    = pages;
        m_cl_start = cl_start;
        m_start    = start;
        m_end      = end;
        m_min_len  = min_len;
        m_case_sen = case_sen;
        m_info_size=info_size;
    }
};

class CLMapMemory
{
public:
    //Buffer OpenCL
    OpenCLMemory::ptr m_ginf{ nullptr };
    OpenCLMemory::ptr m_wign{ nullptr };
    OpenCLMemory::ptr m_text{ nullptr };
    OpenCLMemory::ptr m_info{ nullptr };
    OpenCLMemory::ptr m_rows{ nullptr };
    //save size of buffers
    size_t            m_global_size{ 0 };
    //init
    void init(OpenCLContext&                      context,
              const GlobalInfo&                   ginfo,
              const std::vector< unsigned char >& wign,
              const SizeBuffers&                  sizes,
              bool fill_rows_to_zero=false)
    {
        //global info
        m_ginf = context.create_buffer(OpenCLMemory::READ_ONLY | OpenCLMemory::COPY_HOST_PTR,
                                       sizeof(GlobalInfo),
                                       (void*)&ginfo);
        //global ignore words
        m_wign = context.create_buffer(OpenCLMemory::READ_ONLY | OpenCLMemory::COPY_HOST_PTR,
                                       wign.size(),
                                       (void*)wign.data());
        //pages info buffer
        m_info = context.create_buffer(OpenCLMemory::READ_WRITE, sizes.m_info_size, NULL);
        //pages text buffer
        m_text = context.create_buffer(OpenCLMemory::READ_WRITE, sizes.m_text_size, NULL);
        //pages map buffer
        if (fill_rows_to_zero)
        {   
            //put all to 0
            std::vector <char>  buffer; 
            buffer.resize(sizes.m_rows_size);
            std::memset(buffer.data(), 0, buffer.size());
            m_rows = context.create_buffer(OpenCLMemory::READ_WRITE | OpenCLMemory::COPY_HOST_PTR,
                                           sizes.m_rows_size, 
                                           buffer.data());
        }
        else
            m_rows = context.create_buffer(OpenCLMemory::READ_WRITE, sizes.m_rows_size,  NULL);
        //save size info
        m_global_size =  sizeof(GlobalInfo) +
                         wign.size() +
                         sizes.m_info_size +
                         sizes.m_text_size +
                         sizes.m_rows_size;
    }
    
};

class WordsFilter : public GCPointer< WordsFilter >
{
    
    size_t                       m_min_length { 1 };
    bool                         m_case_sensitive{ false };
    std::vector< unsigned char > m_buffer;
    std::vector< size_t >        m_words_index;
    
    //parsing structs
    struct Error
    {
        std::string m_type_error;
        size_t      m_line;
    };
    struct Command
    {
        std::string m_name;
        std::string m_args;
    };
    //parsing data
    std::vector< Error > m_errors;
    
public:
    
    WordsFilter(){}
    
    WordsFilter(const std::string& text)
    {
        parser(text);
    }
    
    bool parser(const std::string& text)
    {
        //get start
        const char* ptr=text.data();
        size_t  line = 1;
        Command command;
        //for all chars
        while(*ptr && parsing_a_command(ptr,line,command))
        {
            //clean string
            clean_string(command.m_name);
            //
            if(command.m_name == "length" && parsing_length(line,command));
            else if(command.m_name == "case sensitive" && parsing_case_sensitive(line,command));
            else if(command.m_name == "words" && parsing_words(line,command));
            else
            {
                push_error({ "Command not valid", line});
                return false;
            };
        }
        return true;
    }
    
    const std::vector< Error >& get_errors() const
    {
        return m_errors;
    }
    
    const std::vector< unsigned char >& get_buffer() const
    {
        return m_buffer;
    }
    
    size_t get_min_length()
    {
        return m_min_length;
    }
    
    bool get_case_sensitive()
    {
        return m_case_sensitive;
    }
    
    const char* c_str() const
    {
        return (const char*)m_buffer.data();
    }
    
    std::string errors_to_string() const
    {
        std::stringstream sstr;
        for(auto& error:m_errors)
        {
            sstr << "Error: "<< error.m_line << ": "<< error.m_type_error << "\n";
        }
        return sstr.str();
    }
    
    size_t size() const
    {
        return m_words_index.size();
    }
    
    std::string get_word(size_t i) const
    {
        //get ptr
        const unsigned char* word= &m_buffer[ m_words_index[i] ];
        //get out string
        std::string str_out;
        while( *word && *word != ' ' )
        {
            str_out += (char)(*(word++));
        }
        return str_out;
    }
    
    std::string to_str_buffer()
    {
        std::string out;
        for(size_t i=0; i!=  size(); ++i)
        {
            out += get_word(i)+" ";
        }
        return out;
    }
    
private:
    
    void push_error(const Error&& error)
    {
        m_errors.push_back(std::move(error));
    }
    
    void clean_string(std::string& str, bool to_lower=true)
    {
        //trim
        StringUtils::trim(str);
        //remove double space
        StringUtils::remove_double_space(str);
        //to lower
        if(to_lower)
        std::transform(str.begin(),
                       str.end(),
                       str.begin(),
                       ::tolower);
    }
    
    bool parsing_a_command(const char*& text, size_t& line, Command& cmd_out)
    {
        enum State
        {
            NO_CHARS,
            FIND_CHARS,
            ADDED_NAME,
            ADDED_ARG
        };
        State state=NO_CHARS;
        std::string name;
        std::string args;
        
        while (*(text))
        {
            //count line
            line += (*(text))=='\n';
            //cases
            switch (state)
            {
                //NO_CHARS
                case NO_CHARS:
                    if (!StringUtils::isspace(*text))
                    {
                        name=*text; state=FIND_CHARS;
                    }
                break;
                //FIND_CHARS
                case FIND_CHARS:
                    if((*text)!=':')  name += (*text);
                    else state=ADDED_NAME;
                break;
                //ADDED_NAME
                case ADDED_NAME:
                    if((*text)!=';')  args += (*text);
                    else{ state=ADDED_ARG; --text; }
                break;
                case ADDED_ARG:
                    //jmp ;
                    ++text;
                    cmd_out.m_name = name;
                    cmd_out.m_args = args;
                    return true;
                break;
                //unknow
                default:
                    push_error({"unknow", line});
                    return false;
                break;
            };
            //next char
            ++text;
        }
        //error?
        if(state==NO_CHARS) return true;
        if(state==FIND_CHARS) push_error({"Not find :", line});
        if(state==ADDED_NAME) push_error({"Not find ;", line});
        return false;
    }
    
    bool parsing_length(size_t line,Command& cmd_in)
    {
        m_min_length = std::stoi( StringUtils::trim(cmd_in.m_args) );
        if(m_min_length >= 1) return true;
        
        push_error({ "length: must to be a value greater or equal of 1", line});
        return false;
    }
    
    bool parsing_case_sensitive(size_t line,Command& cmd_in)
    {
        clean_string(cmd_in.m_args);
        if( cmd_in.m_args == "false" ){  m_case_sensitive=false; return true; }
        if( cmd_in.m_args == "true"  ){  m_case_sensitive=true; return true; }
        push_error({ "case sensitive: must to be a value true or false", line});
        return false;
    }
    
    bool parsing_words(size_t line,Command& cmd_in)
    {
        //replace space
        for (auto& c : cmd_in.m_args) if (StringUtils::isspace(c)) c = ' ';
        //clean
        clean_string(cmd_in.m_args,false);
        //split binary
        m_buffer.resize(cmd_in.m_args.size()+1);
        m_words_index.push_back(0);
        //copy
        for(size_t i=0; i != cmd_in.m_args.size()+1; ++i)
        {
            //add ptr
            if(cmd_in.m_args[i] == ' ')
            {
                m_words_index.push_back(i+1);
            }
            //copy char
            m_buffer[i] = cmd_in.m_args[i];
        }
        //safe end
        m_buffer[m_buffer.size()-1]='\0';
        //end
        return true;
    }
};

class PageMapping
{
public:
    //constructor used by vector
    PageMapping() { }
    //struct of a row
	ALIGNED( ALIGNED_BIT , struct Row
    {
		TYPE_COUNTER m_count;
        char         m_word[0];
    });
    //struct Map
    class Map : public GCPointer< Map >
    {
        size_t m_cl_offset;
        size_t m_size_row;
        size_t m_real_size;
        size_t m_count_rows;
        std::vector< unsigned char, AlignedAllocator< unsigned char, ALIGNED_BIT > > m_bytes;
        friend class PageMapping;
        //compute size
        void compute_real_size()
        {
            for( m_real_size=0;
                 m_real_size!=max_size() && at(m_real_size)->m_count;
               ++m_real_size );
        }
    
    public:
        
        size_t max_size() const
        {
            return  m_count_rows;
        }
        
        size_t byte_max_size() const
        {
            return m_bytes.size();
        }
        
        size_t byte_size() const
        {
            return row_size()*size();
        }
        
        size_t row_size() const
        {
            return m_size_row;
        }
        
        size_t size() const
        {
            return  m_real_size;
        }

        Row* at(size_t i) const
        {
            //get row[0+i]
            return (Row*)&m_bytes[m_size_row*i];
        }

        Row* operator[] (size_t i) const
        {
            return at(i);
        }
        
        const unsigned char* data() const
        {
            return m_bytes.data();
        }

        
    };
    //struct
	ALIGNED( ALIGNED_BIT ,  struct InfoMap
    {
        cl_uint m_max_size_row{ 0 };
        cl_uint m_max_size_word{ 0 };
        cl_uint m_count_words{ 0 };
        cl_uint m_text_start{ 0 };
        cl_uint m_rows_start{ 0 };
        InfoMap(){}
        InfoMap(cl_uint max_size_row,
                cl_uint max_size_word,
                cl_uint count_words,
                cl_uint text_start,
                cl_uint rows_start)
        {
            m_max_size_row =max_size_row;
            m_max_size_word=max_size_word;
            m_count_words  =count_words;
            m_text_start   =text_start;
            m_rows_start   =rows_start;
        }
    });
    //get page
    WebPage::ptr get_page() const
    {
        return m_page;
    }
    //get page
    Map::ptr get_map() const
    {
        return m_map;
    }
    
private:
    
    struct SizeRows
    {
        size_t m_counter_size{ 0 };
        size_t m_word_size   { 0 };
        size_t m_row_size    { 0 };
        size_t m_rows_size   { 0 };
    };
    //get size row
    static SizeRows compute_size_row(WebPage::ptr page)
    {
        SizeRows res;
        res.m_counter_size = sizeof(TYPE_COUNTER);
        res.m_word_size = page->get_max_size_word() + 1/* plus end string */;
        //compute
        res.m_row_size = res.m_counter_size + res.m_word_size;
        res.m_rows_size = (unsigned int)(res.m_row_size  * page->get_count_words());

        return res;
    }
    //get size
    static SizeBuffers compute_size(WebPage::ptr page)
    {
        SizeBuffers size;
        //info size
        size.m_info_size = sizeof(InfoMap);
        //text size
        size.m_text_size = (unsigned int)page->get_text().size() + 1;
        //rows size
        size.m_rows_size = compute_size_row(page).m_rows_size;
        
        return size;
    }
    //set web page
    void init_mapping(WebPage::ptr page,
                      OpenCLQueue::ptr queue,
                      CLMapMemory& memory,
                      SizeBuffers& offsets)
    {
        /* ------------------------------------------------------------------ */
        //save page
        m_page = page;
        //new map
        m_map = (new Map())->shared();
        //get size
        SizeBuffers sizes=compute_size(page);
        //get size of row
        SizeRows size_row=compute_size_row(page);
        /* ------------------------------------------------------------------ */
        //put info
        m_info_map=InfoMap(
            (cl_uint)size_row.m_row_size,
            (cl_uint)size_row.m_word_size,
            (cl_uint)page->get_count_words(),
            (cl_uint)offsets.m_text_size,
            (cl_uint)offsets.m_rows_size
        );
        queue->write_buffer(*memory.m_info,
                            true,
                            offsets.m_info_size,
                            sizes.m_info_size,
                            (void*)&m_info_map);
        /* ------------------------------------------------------------------ */
        //put text
        queue->write_buffer(*memory.m_text,
                            true,
                            offsets.m_text_size,
                            page->get_text().size() + 1/* plus end string */,
                     (void*)page->get_text().data());
        /* ------------------------------------------------------------------ */
        assert( m_map );
        //init ouput memory
        m_map->m_count_rows = page->get_count_words();
        m_map->m_size_row   = size_row.m_row_size;
        m_map->m_bytes.resize(size_row.m_rows_size);
        m_map->m_cl_offset  = m_info_map.m_rows_start;
        std::memset((void*)m_map->m_bytes.data(),0,sizes.m_rows_size);
        //put rows
        #if 0 //put all to 0
        queue->write_buffer(*memory.m_rows,
                            true,
                            m_info_Map.m_rows_start,
                            sizes.m_rows_size,
                            (void*)m_map->m_bytes.data());
        #endif
        /* ------------------------------------------------------------------ */
        //next
        offsets += sizes;

    }
    //get ptr to buffer
    Map::ptr read_map(OpenCLQueue::ptr queue,
                      CLMapMemory& memory)
    {
        //read from buffer
        ASSERTCL_MSG( queue->read_buffer(*memory.m_rows,
                                          CL_TRUE,
                                          m_map->m_cl_offset,
                                          m_map->m_bytes.size(),
                                          (void*)m_map->m_bytes.data()),
                     "Error: Failed to read");
        //compute size of map
        m_map->compute_real_size();
        //return
        return m_map;
    }
    //Last info map
    InfoMap m_info_map;
    //ptr to page
    WebPage::ptr      m_page{ nullptr };
    //ptr to map
    Map::ptr          m_map{ nullptr };
    //friend class
    friend class WebSiteMapping;
    friend class std::vector< PageMapping >;
    
};

class WebSiteMapping : public GCPointer< WebSiteMapping >
{
public:
    
    WebSiteMapping(){}
    WebSiteMapping(OpenCLContext&     context,
                   OpenCLQueue::ptr   queue,
                   WebSite::ptr site,
                   WordsFilter::ptr   filter,
                   size_t page_start=0)
    {
        init(context, 
             queue, 
             site,
             filter,
             page_start);
    }

    WebSiteMapping(OpenCLContext&     context,
                   OpenCLQueue::ptr   queue,
                   WebSite::ptr       site,
                   WordsFilter::ptr   filter,
                   size_t page_start,
                   size_t page_end)
    {
        init(context,
             queue,
             site,
             filter,
             page_start,
             page_end);
    }
    
    void init(OpenCLContext&     context,
              OpenCLQueue::ptr   queue,
              WebSite::ptr       site,
              WordsFilter::ptr   filter,
              size_t page_start =0)
    {

        init(context,
             queue,
             site,
             filter,
             page_start,
             site->size());
    }

    void init(OpenCLContext&     context,
              OpenCLQueue::ptr   queue,
              WebSite::ptr       site,
              WordsFilter::ptr   filter,
              size_t page_start,
              size_t page_end)
    {
        //save site
        m_site=site;
        //init pages
        m_pages.resize(page_end - page_start);
        //compute size
        SizeBuffers sizes;
        for (size_t i = page_start; i != page_end; ++i)
        {
            sizes += PageMapping::compute_size(m_site->at(i)) ;
        }
        //init buffers
        GlobalInfo ginf(
            (cl_uint)m_pages.size(),
            (cl_uint)0,
            (cl_uint)page_start,
            (cl_uint)page_end,
            (cl_uint)sizeof(PageMapping::InfoMap),
            (cl_uint)filter->get_min_length(),
            (cl_bool)filter->get_case_sensitive()
        );
        m_cl_memory.init(context,
                         ginf,
                         filter->get_buffer(),
                         sizes,
                         true);
        //write buffers
        SizeBuffers offsets;
        for (size_t i = page_start; i != page_end; ++i)
        {
            m_pages[i-page_start].init_mapping(m_site->at(i),
                                               queue,
                                               m_cl_memory,
                                               offsets);
        }
    }
    
    void set_args(OpenCLKernel::ptr kernel)
    {
        const std::vector< cl_mem > args=
        {
            *(m_cl_memory.m_ginf),
            *(m_cl_memory.m_wign),
            *(m_cl_memory.m_info),
            *(m_cl_memory.m_text),
            *(m_cl_memory.m_rows)
        };
        kernel->set_args(args);
    }
    
    PageMapping::Map::ptr read_map(OpenCLQueue::ptr queue, size_t i)
    {
        return m_pages[i].read_map(queue,m_cl_memory);
    }
    
    void read_all(OpenCLQueue::ptr queue)
    {
        for(auto& page: m_pages) page.read_map(queue,m_cl_memory);
    }
    
    size_t size() const
    {
        return m_pages.size();
    }
    
    PageMapping::Map::ptr get_map(size_t i) const
    {
        return m_pages[i].get_map();
    }
    
    WebPage::ptr get_page(size_t i) const
    {
        return m_pages[i].get_page();
    }
    
    size_t get_buffers_size() const
    {
        return m_cl_memory.m_global_size;
    }
    
    
    
private:
    //Buffer OpenCL
    CLMapMemory   m_cl_memory;
    //ptr to page
    WebSite::ptr  m_site{ nullptr };
    //ptr to pages
    std::vector< PageMapping > m_pages;
 };

class Mapping
{
    struct Task
    {
        cl_device_id     m_device;
        cl_uint          m_page_start;
        cl_uint          m_page_end;
        OpenCLQueue::ptr m_queue;
    };
    
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
        //load source
        std::string source  = StringUtils::file_to_string(m_source_path);
        //compile
        //program
        m_program = context.create_program(source);
        //compile
        if (m_program->build(m_devices, options.c_str() ) != CL_SUCCESS)
        {
            std::cout << ("Error: Failed to build source\n");
            for(auto& device : m_devices)
            {
                std::cout << "Error from device:\n"
                          << OpenCLInfo::get_device_info(device).to_string()
                          << "\n"
                          << m_program->get_last_build_errors(device);
            }
            return;
        }
        //get kernel
        m_kernel = m_program->create_kernel(m_kernel_name);
    }
    
    void execute_task(OpenCLContext&     context,
                      WebSite::ptr       site,
                      WordsFilter::ptr   filter)
    {
        //init
        size_t size =site->size();
        size_t added=0;
        size_t done =0;
        std::vector< size_t >              id_devices;
        std::vector< WebSiteMapping::ptr > ws_mapping;
        
        //events
        std::vector< cl_event > events;
        events.resize(m_devices.size());
        
        //works size
        std::vector< size_t > words_size;
        words_size.resize(m_devices.size());
        
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
                    //reset
                    event_status = CL_QUEUED;
                    //get info
                    ASSERTCL_MSG(clGetEventInfo(events[i],
                                                CL_EVENT_COMMAND_EXECUTION_STATUS,
                                                sizeof(cl_int),
                                                &event_status,
                                                NULL),  "Error: Failed to get event status");
                }
                //execute new instance
                if(event_status == CL_COMPLETE)
                {
                    //release event
                    if(events[i])
                    {
                        clReleaseEvent(events[i]);
                        events[i]=0;
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
                        size_t g_worker_size = m_kernel->get_work_goup_max_size(m_devices[i]);
                        size_t l_worker_size = 1;
                        //min
                        if(g_worker_size >  (size - added) )
                            g_worker_size = size - added;
                        
                        //save id of device
                        id_devices.push_back(i);
                        
                        //new buffer
                        auto ws_map=
                        (new WebSiteMapping(
                                            context,
                                            queues[i],
                                            site,
                                            filter,
                                            added,
                                            g_worker_size + added
                                            ))->shared();
                        //save
                        ws_mapping.push_back(ws_map);
                        //set args
                        ws_map->set_args(m_kernel);
                        //execute
                        ASSERTCL_MSG(queues[i]->enqueue_ndrange_kernel(*m_kernel, 1,
                                                                       NULL,
                                                                       &g_worker_size,
                                                                       &l_worker_size,
                                                                       0,
                                                                       NULL,
                                                                       &events[i]), "Error: Failed to execute");
                        //flush
                        queues[i]->flush();
                        //done is...
                        added += g_worker_size;
                        //set
                        words_size[i] = g_worker_size;
                    }
                }
                
            }
        }
        //init queues read
        for(size_t i=0;i!=m_devices.size();++i)
        {
            queues[i]=context.create_queue(m_devices[i], NULL);
        }
        //read all
        for(size_t i=0;i!=id_devices.size();++i)
        {
            ws_mapping[i]->read_all(queues[ id_devices[i] ]);
        }
        //wait all
        for(auto queue : queues)
        {
            ASSERTCL( queue->finish() );
        }
        //save all maps
        m_maps.clear();
        for (auto ws : ws_mapping)
        {
            for(size_t i=0; i!=ws->size(); ++i)
            {
                m_maps.push_back(ws->get_map(i));
            }
        }
        
        
    }
    
    const std::vector<  PageMapping::Map::ptr  >& get_maps() const
    {
        return m_maps;
    }
    
    const std::vector< cl_device_id > get_devices() const
    {
        return  m_devices;
    }
    
private:
    //list of device
    std::vector< cl_device_id > m_devices;
    OpenCLProgram::ptr m_program;
    OpenCLKernel::ptr  m_kernel;
    //web sites
    WebSite::ptr       site;
    WordsFilter::ptr   filter;
    //results
    std::vector<  PageMapping::Map::ptr  > m_maps;
    //source info
    const std::string  m_include_path="script/";
    const std::string  m_source_path="script/mapping_site.cl";
    const std::string  m_kernel_name="mapping_site";
    
};
