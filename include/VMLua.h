#pragma once
#include <Config.h>
#include <luajit/lua.hpp>

class VMLua
{
    //lua state
    lua_State* m_state;
    //last error
    std::string m_error;

public:

    VMLua();

    ~VMLua();

    bool execute_from_file(const std::string& path);

    const std::string& get_error() const
    {
        return m_error;
    }

};