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
#include <VMLua.h>

#define VERBOSE(x) if(verbose) { MESSAGE(x); }
int main(int argc,const char** args)
{
    //no args
    if(argc<=1) return 0;
    //name app
    std::string name(args[0]);
    //C++ args
    std::vector< std::string > v_args;
    //alloc args
    v_args.resize(argc-1);
    //copy args
    for(size_t i=1;i!=argc;++i) v_args[i-1]=args[i];
    //options
    std::string o_site_path;
    std::string o_lua_path;
    std::string o_mapping_path;
    std::string o_reduce_path;
    std::string o_reduce_cpu_path;
    std::string o_filter_path;
    bool verbose  = false;
    bool split    = false;
    OpenCLContext::DeviceType type = OpenCLContext::TYPE_ALL;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //ARGS PARSER
    {
        if(v_args.size()==1 && ( v_args[0]=="-h" || v_args[0]=="--help" ))
        {
            MESSAGE( name << " [options]" );
            MESSAGE( "Options:" );
            MESSAGE( "\t--input-site/-is <path>  input site file" );
            MESSAGE( "\t--input-lua/-il <path>  input lua download script" );
            MESSAGE( "\t--ouput-mapping/-om <path>  directory of ouput of mapping" );
            MESSAGE( "\t--ouput-reduce/-or <path> directory of ouput of reduce" );
            MESSAGE( "\t--ouput-reduce-cpu/-orc <path> directory of ouput of reduce (no OCL)" );
            MESSAGE( "\t--input-filter/-if <path> input filter file" );
            MESSAGE( "\t--verbose/-v verbose mode [default: false]");
            MESSAGE( "\t--cpu/--gpu/--all/-c/-g/-a select type of devices [default: all]");
            MESSAGE( "\t--split/--s split the reduce map (OCL)" );
            return 0;
        }
        else if(v_args.size()==1)
        {
            MESSAGE("Invalid input");
            return -1;
        }
        else
        {
            //for all
            for(size_t arg=0; arg!= v_args.size() ; ++arg)
            {
                if (v_args[arg] == "-v" || v_args[arg] == "--verbose")
                {
                    verbose = true;
                }
                else if (v_args[arg] == "-s" || v_args[arg] == "--split")
                {
                    split = true;
                }
                else if( v_args[arg]=="-a" || v_args[arg]=="--all" )
                {
                    type = OpenCLContext::TYPE_ALL;
                }
                else if( v_args[arg]=="-c" || v_args[arg]=="--cpu" )
                {
                    type = OpenCLContext::TYPE_CPU;
                }
                else if( v_args[arg]=="-g" || v_args[arg]=="--gpu" )
                {
                    type = OpenCLContext::TYPE_GPU;
                }
                else if( (arg+1)!= v_args.size() )
                {
                    if (v_args[arg] == "-is" || v_args[arg] == "--input-site")
                    {
                        o_site_path = v_args[arg + 1]; ++arg;
                    }
                    else if (v_args[arg] == "-il" || v_args[arg] == "--input-lua")
                    {
                        o_lua_path = v_args[arg + 1]; ++arg;
                    }
                    else if( v_args[arg]=="-om" || v_args[arg]=="--ouput-mapping" )
                    {
                        o_mapping_path=v_args[arg+1]; ++arg;
                    }
                    else if( v_args[arg]=="-or" || v_args[arg]=="--ouput-reduce" )
                    {
                        o_reduce_path=v_args[arg+1]; ++arg;
                    }
                    else if( v_args[arg]=="-orc" || v_args[arg]=="--ouput-reduce-cpu" )
                    {
                        o_reduce_cpu_path=v_args[arg+1]; ++arg;
                    }
                    else if( v_args[arg]=="-if" || v_args[arg]=="--input-filter" )
                    {
                        o_filter_path=v_args[arg+1]; ++arg;
                    }
                    else
                    {
                        MESSAGE( "Invalid input" );
                        return -1;
                    }
                }
                else
                {
                    MESSAGE( "Invalid input" );
                    return -1;
                }
            }
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //not execute reduce/mapping
    bool  not_exe_rm = !o_site_path.size() || (!o_mapping_path.size() && !o_reduce_path.size() && !o_reduce_cpu_path.size());
    //not execute lua script
    bool  not_exe_lua = !o_lua_path.size();
    //error...
    if (not_exe_rm && not_exe_lua)
    {
        MESSAGE("Invalid input, required site path and a ouput and/or script input");
        return -1;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (o_lua_path.size())
    {
        VERBOSE("Execute lua file: " << o_lua_path)
        VMLua lua;
        if (!lua.execute_from_file(o_lua_path))
        {
            MESSAGE("Lua errors:\n" << lua.get_error());
        }
        //no reduce mapping?
        if (not_exe_rm) return 0;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VERBOSE("Loading site file: "<<o_site_path)
    WebSite::ptr     web_site=WebSite::shared_new(o_site_path);
    //read filter
    WordsFilter::ptr filter=WordsFilter::shared_new();
    //load filter
    if(o_filter_path.size())
    {
        VERBOSE("Loading filter file: "<<o_filter_path)
        filter  =WordsFilter::shared_new(StringUtils::file_to_utf8(o_filter_path));
        //errors?
        if(filter->get_errors().size())
        {
            MESSAGE("Error to load filter:" << filter->errors_to_string());
            return -1;
        }
    }
    //devices
    VERBOSE( OpenCLInfo::to_string()  );
    VERBOSE( "Create context" )
    OpenCLContext context(type);
    //plathform
    auto platform = context.get_platform();
    VERBOSE("Select:\n"  << OpenCLInfo::get_platform_info(platform).to_string());
    //
    VERBOSE( "Start mapping" )
    //manager mapping
    Mapping map;
    //init mapping
    map.init(context);
    //execute mapping
    map.execute_task(context, web_site, filter);
    //save
    if(o_mapping_path.size())
    {
        VERBOSE("Save mapping")
        //print maps
        size_t i=0;
        for(auto page_map :  map.get_maps())
        {
            std::stringstream out_str;
            out_str << "{\n";
            for (size_t i = 0; i != page_map->size() && page_map->at(i)->m_count ; ++i)
                out_str << " \"" << page_map->at(i)->m_word << "\" : " << page_map->at(i)->m_count << ",\n";
            out_str << ("};");
            StringUtils::string_to_file(o_mapping_path+"/size_"+std::to_string(i)+"_map.txt", out_str.str());
            StringUtils::string_to_file(o_mapping_path+"/size_"+std::to_string(i)+".txt", web_site->at(i)->get_text());
            //next
            ++i;
        }
    }
    //compute reduce only if required
    if(o_reduce_cpu_path.size())
    {
        VERBOSE( "Start reduce cpu" );
        ////////////////////////////////////////////////////////////////////////////////////////////////
        //create index
        InvertedIndex inverted_index(web_site, map.get_maps());
        ////////////////////////////////////////////////////////////////////////////////////////////////
        VERBOSE( "Save reduce cpu" );
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
        StringUtils::string_to_file(o_reduce_cpu_path+"/reduce_cpu.txt", out_str.str());
    };
    
    //compute reduce only if required
    if(o_reduce_path.size())
    {
        //execute reduce
        VERBOSE("Start reduce")
        //////////////////////////////////////////////////////////
        WordMapInvertedIndex::InvertedIndexMap::ptr reduce_map;
        //////////////////////////////////////////////////////////
        auto save_function =
        [=](const std::vector< cl_ushort >& data, size_t words, size_t page_start, size_t n_page)
        {
            //print state
            VERBOSE("Start save pages [ " << page_start << ", " << (page_start + n_page) << " ]");
            //save
            size_t end_page = page_start + n_page;
            for (size_t p = page_start; p != end_page; ++p)
            {
                //alloc
                size_t w_start = words*(p - page_start);
                size_t w_end = w_start + words;
                std::stringstream out_str;
                //write
                for (size_t v = w_start; v != w_end && v != data.size(); ++v)
                {
                    out_str << (v - w_start) << " : " << data[v] << "\n";
                }
                //save
                StringUtils::string_to_file(o_reduce_path + "/reduce_map_" + std::to_string(p) + "_cl.txt", out_str.str());
            }
            VERBOSE("end save")
        };
        auto save_function_compress =
        [=](const std::vector< cl_ushort >& data, size_t words, size_t page_start, size_t n_page)
        {
            //print state
            VERBOSE("Start save pages [ " << page_start << ", " << (page_start + n_page) << " ]");
            //save
            size_t end_page = page_start + n_page;
            //alloc
            std::stringstream out_str;
            //write
            for (size_t v = 0; v != words && v != data.size(); ++v)
            {
                out_str << v << " : ";
                for (size_t p = 0; p != n_page; ++p)
                {
                    out_str << data[v + words*p] << ' ';
                }
                out_str << '\n';
            }
            //save
            StringUtils::string_to_file(o_reduce_path +
                                        "/reduce_map_[ " + 
                                        std::to_string(page_start) +
                                        " - " + 
                                        std::to_string(end_page) +
                                        " ]_cl.txt",
                                        out_str.str());
            VERBOSE("end save")
        };
        //select function
        std::function<void(const std::vector< cl_ushort >& data, size_t words, size_t page_start, size_t n_page)>
        save_select_function = nullptr;
        //init select function
        if (split) save_select_function = save_function;
        else       save_select_function = save_function_compress;
        //////////////////////////////////////////////////////////
        OpenCLInvertedIndex inverted_index;
        inverted_index.init(context);
        inverted_index.execute_task(context,
                                    map.get_maps(),
                                    reduce_map,
                                    save_select_function);
        //////////////////////////////////////////////////////////
        std::stringstream out_str;
        for(cl_uint y=0;y!=reduce_map->real_count_row();++y)
        {
            out_str << y << " : " << reduce_map->at(y) << " \n";
        }
        StringUtils::string_to_file(o_reduce_path+"/reduce_words_cl.txt", out_str.str());

    };
    VERBOSE("The execution is complete")
    return 0;
}