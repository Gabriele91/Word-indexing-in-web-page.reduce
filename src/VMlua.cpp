#include <VMLua.h>
#include <StringUtils.h>
#include <GetHTTP.h>
#include <ParserHTML.h>
#include <ThreadPool.h>
#include <WebSite.h>
#include <mutex>
#include <LuaBridge/LuaBridge.h>

class LuaWebSite
{

    WebSite::ptr site;
    mutable std::mutex mutex;

public:

    LuaWebSite()
    {
        site = WebSite::shared_new();
    }
    
    void add(const ParserHTML& parse)
    {
        auto page = WebPage::shared_new();
        page->set_url(parse.get_url());
        page->set_text_raw(parse.get_text());
        mutex.lock();
        site->add(page);
        mutex.unlock();
    }
    
    void unsafe_add(const ParserHTML& parse)
    {
        auto page = WebPage::shared_new();
        page->set_url(parse.get_url());
        page->set_text_raw(parse.get_text());
        site->add(page);
    }

    size_t size() const
    {
        return site->size();
    }

    WebSite::ptr get_site_raw() const
    {
        return site;
    }

    void save_to_file(const std::string& path) 
    {
        mutex.lock();
        site->save_to_file(path);
        mutex.unlock();
    }
    
    void lock() const
    {
        mutex.lock();
    }
    
    void unloack() const
    {
        mutex.unlock();
    }
};

class LuaParserHTML : public ParserHTML
{
public:
    int get_links(lua_State* state)
    {
        auto table = luabridge::newTable(state);
        auto links = this->ParserHTML::get_links();
        for (size_t i = 0; i != links.size(); ++i)
        {
            table[i] = links[i];
        }
        table.push(state);
        return 1;
    }
};

class ThreadPoolLua : protected ThreadPool
{
    

public:
    ThreadPoolLua(int threads) :ThreadPool(threads){}
    int enqueue(lua_State* state)
    {
        int const nargs = lua_gettop(state);
        //
        if (nargs != 3)
        {
            luaL_argerror(state, 2, "ThreadPool::enqueue(website,url) fail");
        }
        //values
        auto* web_site= luabridge::Stack<LuaWebSite*>::get(state, 2);
        auto  web_url = luabridge::Stack<std::string>::get(state, 3);
        ThreadPool::enqueue([=]()
        {
            //parallel download
            GetHTTP site(web_url);
            //parsing
            ParserHTML html(site);
            //save
            web_site->add(html);
        });
        return 0;
    }
    virtual ~ThreadPoolLua(){}
    size_t task_count() const
    {
        return tasks.size();
    }
}; 


VMLua::VMLua()
{
    m_state = lua_open();
    luaopen_base(m_state);             /* opens the basic library */
    luaopen_table(m_state);            /* opens the table library */
  //luaopen_io(m_state);               /* opens the I/O library */
    luaopen_string(m_state);           /* opens the string lib. */
    luaopen_math(m_state);             /* opens the math lib. */
    //init curl
    GetHTTP::force_start_curl();
    //download libs
    luabridge::getGlobalNamespace(m_state)
        .addFunction<void(*)(void)>("system_clear", []()
        {
            #ifdef _WIN32
                system("cls");
            #else
                system("clear");
            #endif
        });

    luabridge::getGlobalNamespace(m_state)
    .beginClass<GetHTTP>("GetHTTP")
        .addConstructor<void(*)(const std::string&)>()
        .addFunction("to_string", &GetHTTP::to_string)
        .addFunction("get_error_to_string", &GetHTTP::get_error_to_string)
        .endClass();

    luabridge::getGlobalNamespace(m_state)
    .beginClass<ParserHTML>("ParserHTML")
        .addConstructor<void(*)(const GetHTTP&)>()
        .addFunction("get_text", &ParserHTML::get_text)
        .addCFunction("get_links", (int (ParserHTML::*)(lua_State*))&LuaParserHTML::get_links)
        .addFunction("get_title", &ParserHTML::get_title)
        .endClass();

    luabridge::getGlobalNamespace(m_state)
        .beginClass<ThreadPoolLua>("ThreadPool")
        .addConstructor<void(*)(int)>()
        .addFunction("task_count", &ThreadPoolLua::task_count)
        .addCFunction("enqueue", &ThreadPoolLua::enqueue)
        .endClass(); 

    luabridge::getGlobalNamespace(m_state)
        .beginClass<LuaWebSite>("WebSite")
        .addConstructor<void(*)(void)>()
        .addFunction("add", &LuaWebSite::add)
        .addFunction("size", &LuaWebSite::size)
        .addFunction("save_to_file", &LuaWebSite::save_to_file)
        .endClass();
}

VMLua::~VMLua()
{
    lua_close(m_state);
}

bool VMLua::execute_from_file(const std::string& path)
{
    //load source
    std::string source=StringUtils::file_to_string(path);
    //execute
    if(luaL_dostring(m_state, source.c_str()))
    {
        //get error
        m_error = lua_tostring(m_state, -1);
        //return 
        return false;
    }
    //good execution
    return true;
}