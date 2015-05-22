#pragma once
#include <wchar.h>
#include <Config.h>
#include <GetHTTP.h>

class ParserHTML
{
    std::string m_text;
    std::string m_title;
    std::vector < std::string > m_links;

public: 

    ParserHTML(const GetHTTP& webpage);

    const std::string& get_text()
    {
        return m_text;
    }

    const std::string& get_title()
    {
        return m_title;
    }

    const std::vector < std::string >& get_links()
    {
        return m_links;
    }

};