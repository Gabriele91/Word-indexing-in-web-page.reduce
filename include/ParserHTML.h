#pragma once
#include <wchar.h>
#include <Config.h>
#include <GetHTTP.h>

class ParserHTML
{
    std::string m_text;
    std::string m_title;
    std::string m_url;
    std::vector < std::string > m_links;

public: 

    ParserHTML(const GetHTTP& webpage);

    const std::string& get_text() const
    {
        return m_text;
    }

    const std::string& get_title() const
    {
        return m_title;
    }

    const std::string& get_url() const
    {
        return m_url;
    }

    const std::vector < std::string >& get_links() const
    {
        return m_links;
    }

};