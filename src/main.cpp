//
//  main.cpp
//  Word-indexing-in-web-page
//
//  Created by Gabriele Di Bari on 10/04/15.
//  Copyright (c) 2015 Gabriele Di Bari. All rights reserved.
//

#include <stdio.h>
#include <OpenCLInfo.h>
#include <OpenCLContext.h>
#include <iostream>
#include <sstream>
#include <GetHTTP.h>
#include <WebSite.h>
#include <SiteMapping.h>
#include <InvertedIndex.h>
#include <OpenCLInvertexIndex.h>

//#define ONLY_CPU

#if 0
int main(int argc, const char* argv[])
{
    MESSAGE( OpenCLInfo::to_string() );
    //test create context
	//auto platform = OpenCLInfo::get_id_platforms()[0];
	OpenCLContext context(OpenCLContext::TYPE_ALL);
    auto platform = context.get_platform();
    auto device   = context.get_devices()[0];
    MESSAGE("Select "  << OpenCLInfo::get_platform_info(platform).to_string() <<  "\n\n"
                       << OpenCLInfo::get_device_info(device).to_string()     << "\n\n");
    
    //try to create queue
    auto queue= context.create_queue(device,NULL);
    
    const std::string  source_path="script/copy_buffer.cl";
    const std::string  kernel_name="copy_buffer";
    const char   bvalue[]    = "hello world";
    const size_t worker_size = std::strlen(bvalue);
    //program
    auto program = context.create_program(StringUtils::file_to_string(source_path));
    //compile
    if(program->build(1, &device , OpenCLProgram::debug_option(source_path).c_str()) != CL_SUCCESS)
    {
        std::cout << ("Error: Failed to build source\n");
        std::cout << program->get_last_build_errors(device);
        return 1;
    }
    //get kernel
    auto kernel=program->create_kernel(kernel_name);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //opencl buffer 1
    auto buffer_1= context.create_buffer(OpenCLMemory::WRITE_ONLY|OpenCLMemory::COPY_HOST_PTR, worker_size,  (void*)bvalue);
    std::cout << "Buffer 1 write: "<< bvalue << "\n";
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //opencl buffer 2
    auto buffer_2= context.create_buffer(OpenCLMemory::READ_ONLY, worker_size, NULL);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    //set args
    kernel->set_args({ *buffer_1, *buffer_2 });
    //execute
    ASSERTCL_MSG( queue->enqueue_ndrange_kernel( *kernel, 1, NULL, &worker_size, &worker_size), "Error: Failed to execute");
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //read data buffer
    char rbvalue[sizeof(bvalue)];
    //end line
    rbvalue[worker_size] = '\0';
    //read data 
    ASSERTCL_MSG(queue->read_buffer( *buffer_2,  CL_TRUE, NULL, worker_size, rbvalue), "Error: Failed to read");
    std::cout << "Buffer 2 read: "<< rbvalue << "\n\n";
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //await completion of all the above
    ASSERTCL_MSG( queue->finish(), "Error: Failed to await completion \n");
    
    return 0;
}
#elif 0
int main()
{
    #define EXEMPLE_PAGE "http://www.december.com/html/demo/hello.html"
    #define EXEMPLE_PAGE2 "http://opencltests.altervista.org/index.html"
    #define TRECCANI_01 "http://www.treccani.it/enciclopedia/ada_%28Enciclopedia_della_Scienza_e_della_Tecnica%29/"
    #define WIKI_IT "http://it.wikipedia.org/wiki/Pagina_principale"
    #define WIKI_JP "http://ja.wikipedia.org/wiki/%E3%83%A1%E3%82%A4%E3%83%B3%E3%83%9A%E3%83%BC%E3%82%B8"

    GetHTTP page(TRECCANI_01);
    StringUtils::string_to_file("test_web.txt", page.get_buffer().to_string());
    
    ParserHTML parser(page);
    StringUtils::string_to_file("test.txt", parser.get_text());

    MESSAGE(parser.get_text());
    for (auto& link : parser.get_links()) MESSAGE(link);
    return 0;
}
#elif 0
int main()
{
    std::string str=StringUtils::file_to_utf8("words.filter");
    MESSAGE( str );
    StringUtils::to_lower_utf8(str);
    MESSAGE( str );
    return 0;
}
#elif 0
int main()
{
    //load website
    auto treccani=(new WebSite("Web-page-parser/treccani.pages"))->shared();
    //test filter
    auto filter  =(new WordsFilter(StringUtils::file_to_utf8("words.filter")))->shared();
    if(filter->get_errors().size())
    {
        MESSAGE(filter->errors_to_string());
    }
    else
    {
        MESSAGE( "Ingnore: " );
        MESSAGE(filter->c_str());
    }
    MESSAGE( OpenCLInfo::to_string() );
    //test create context
    //auto platform = OpenCLInfo::get_id_platforms()[0];
    OpenCLContext::DeviceType type = OpenCLContext::TYPE_CPU;
    OpenCLContext context(type);
    auto platform = context.get_platform();
    auto device   = context.get_devices()[0];
    MESSAGE("Select "  << OpenCLInfo::get_platform_info(platform).to_string() <<  "\n\n"
                       << OpenCLInfo::get_device_info(device).to_string()     << "\n\n");
    
    //try to create queue
    auto queue = context.create_queue(device, NULL);
    //compile mapping kernel
    const std::string  include_path="script/";
    const std::string  source_path="script/mapping_site.cl";
    const std::string  kernel_name="mapping_site";
    const bool               debug=true;
    //create string
    const std::string options =
    (type == OpenCLContext::TYPE_CPU ? OpenCLProgram::debug_option(source_path) : "")
    +OpenCLProgram::include_option(include_path)
    +std::string( debug ? " -DDEBUG" : "" ) ;
    //program
    auto program = context.create_program(StringUtils::file_to_string(source_path));
    //compile
    if (program->build(1, &device, options.c_str() ) != CL_SUCCESS)
    {
        std::cout << ("Error: Failed to build source\n");
        std::cout << program->get_last_build_errors(device);
        return 1;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    //get kernel
    auto kernel = program->create_kernel(kernel_name);
    MESSAGE("MAX SIZE: " << kernel->get_work_goup_max_size(device));
    MESSAGE("PREF SIZE MULTIPLE: " << kernel->get_prefered_work_goup_max_size_multiple(device));
    //set size
    const size_t g_worker_size = kernel->get_work_goup_max_size(device);//1023; // :P
    const size_t l_worker_size = 1;
    //create cl buffers of site
    WebSiteMapping site_buffer(context, queue, treccani, filter, 0, g_worker_size);
    //size
    MESSAGE("Get Buffers size: "<<site_buffer.get_buffers_size()<<"\n");
    ////////////////////////////////////////////////////////////////////////////////////////////////
    //set args
    site_buffer.set_args(kernel);
    //execute
    ASSERTCL_MSG(queue->enqueue_ndrange_kernel(*kernel, 1, NULL, &g_worker_size, &l_worker_size), "Error: Failed to execute");
    //await completion of all the above
    ASSERTCL_MSG(queue->finish(), "Error: Failed to await completion \n");
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    //new queue
    auto read_queue = context.create_queue(device, NULL);
    //read maps
    for(size_t i=0; i!=g_worker_size; ++i)
    {
        auto page_map=site_buffer.read_map(read_queue,i);
        std::stringstream out_str;
        out_str << "{\n";
        for (size_t i = 0; i != page_map->size() && page_map->at(i)->m_count ; ++i)
            out_str << " \"" << page_map->at(i)->m_word << "\" : " << page_map->at(i)->m_count << ",\n";
        out_str << ("};");
        StringUtils::string_to_file("tests/size_"+std::to_string(i)+"_map.txt", out_str.str());
        StringUtils::string_to_file("tests/size_"+std::to_string(i)+".txt", treccani->at(i)->get_text());
    }
    
    //create index
    InvertedIndex inverted_index(read_queue, site_buffer,0,g_worker_size);
    ////////////////////////////////////////////////////////////////////////////////////////////////

    //save index
	std::stringstream out_str;
	out_str << "{\n";
    for (auto& index : inverted_index)
	{
		out_str << index.get_word() << " - ";
        for (size_t j = 0; j != index.count_indices(); ++j)
		{
			out_str << index.get_url(j) << " (" << index.get_count(j) << "), ";
		}
		out_str << "\n";
	}
	out_str << ("};");
	StringUtils::string_to_file("tests/reduce.txt", out_str.str());
    
    return 0;
}
#elif 1
int main()
{
    //load website
    auto treccani=(new WebSite("Web-page-parser/treccani.pages"))->shared();
    //test filter
    auto filter  =(new WordsFilter(StringUtils::file_to_utf8("words.filter")))->shared();
    DEBUGCODE
    (
        if(filter->get_errors().size())
        {
            MESSAGE(filter->errors_to_string());
        }
        else
        {
            MESSAGE( "Ingnore: " );
            MESSAGE(filter->c_str());
        }
    );
    MESSAGE( OpenCLInfo::to_string() );
    //test create context
    OpenCLContext::DeviceType type = OpenCLContext::TYPE_ALL;
    OpenCLContext context(type);
    auto platform = context.get_platform();
    //print platform
    MESSAGE("Select "  << OpenCLInfo::get_platform_info(platform).to_string() <<  "\n");
    //print all devices
    for (auto& device:context.get_devices())
    {
        MESSAGE(OpenCLInfo::get_device_info(device).to_string()<< "\n");
    }
    MESSAGE("START MAPPING")
    //manager mapping
    Mapping map;
    //init mapping
    map.init(context);
    //execute mapping
    map.execute_task(context, treccani, filter);
   
    DEBUGCODE
    (
            //print devices
            MESSAGE( "List device used for mapping:\n" );
            for(auto device : map.get_devices())
            {
                MESSAGE( OpenCLInfo::get_device_info(device).to_string() << "\n" );
            }
    );
    ////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
    DEBUGCODE
    (
         MESSAGE("SAVE MAPPING")
         //print maps
         size_t i=0;
         for(auto page_map :  map.get_maps())
         {
             std::stringstream out_str;
             out_str << "{\n";
             for (size_t i = 0; i != page_map->size() && page_map->at(i)->m_count ; ++i)
                 out_str << " \"" << page_map->at(i)->m_word << "\" : " << page_map->at(i)->m_count << ",\n";
             out_str << ("};");
             StringUtils::string_to_file("tests/size_"+std::to_string(i)+"_map.txt", out_str.str());
             StringUtils::string_to_file("tests/size_"+std::to_string(i)+".txt", treccani->at(i)->get_text());
             //next
             ++i;
         }
     );
#endif
#ifdef ONLY_CPU
    MESSAGE("START REDUCE")
    ////////////////////////////////////////////////////////////////////////////////////////////////
    //create index
    InvertedIndex inverted_index(treccani, map.get_maps());
    ////////////////////////////////////////////////////////////////////////////////////////////////
    MESSAGE("SAVE REDUCE")
    //save index
    std::stringstream out_str;
    out_str << "{\n";
    for (auto& index : inverted_index)
    {
        out_str << index.get_word() << " - ";
        for (size_t j = 0; j != index.count_indices(); ++j)
        {
            out_str << index.get_url(j) << " (" << index.get_count(j) << "), ";
        }
        out_str << "\n";
    }
    out_str << ("};");
    StringUtils::string_to_file("tests/reduce.txt", out_str.str());
#else
    //////////////////////////////////////////////////////////
    WordMapInvertedIndex::InvertedIndexMap::ptr reduce_map;
    //////////////////////////////////////////////////////////
    OpenCLInvertedIndex inverted_index;
    inverted_index.init(context);
    inverted_index.execute_task(context,map.get_maps(),
                                reduce_map,
                                [](const std::vector< cl_ushort >& data, size_t words,size_t page_start, size_t n_page)
                                {
                                    size_t end_page=page_start+n_page;
                                    for(size_t p=page_start; p!=end_page; ++p)
                                    {
                                        //alloc
                                        size_t w_start=words*(p-page_start);
                                        size_t w_end  =w_start + words;
                                        std::stringstream out_str;
                                        //write
                                        for(size_t v=w_start; v!=w_end && v!=data.size() ;++v)
                                        {
                                            out_str << (v-w_start) << " : " << data[v] << "\n";
                                        }
                                        //save
                                        StringUtils::string_to_file("tests/reduce_map"+std::to_string(p)+"_cl.txt", out_str.str());
                                    }
                                });

    DEBUGCODE
    (
        std::stringstream out_str;
        for(unsigned int y=0;y!=reduce_map->count_row() && *reduce_map->at(y);++y)
        {
            out_str << y << " : " << reduce_map->at(y) << " \n";
        }
        StringUtils::string_to_file("tests/reduce_word_cl.txt", out_str.str());
    )
#endif

    return 0;
}
#endif