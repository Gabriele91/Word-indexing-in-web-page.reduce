#pragma once
#include <Config.h>
#define CURL_STATICLIB
#include <curl/curl.h>

class GetHTTP
{

public:

    struct StreamRead
    {
        typedef std::vector< char > Bytes;
        //buffer
        Bytes m_buffer;
        //append
        void append(void* ptr, size_t add_size);
        //add to string (and relase context)
        void copy_to_string(std::string& outstr);
        void copy_to_string(std::wstring& outstr);
        //safe to string
        std::string to_string() const
        {
            StreamRead bcopy(*this);
            std::string rstr;
            bcopy.copy_to_string(rstr);
            return rstr;
        }
        std::wstring to_wstring() const
        {
            StreamRead bcopy(*this);
            std::wstring rwstr;
            bcopy.copy_to_string(rwstr);
            return rwstr;
        }
    };

	GetHTTP(const std::string& url);

	operator std::string ()
	{
        std::string out;
        m_content.copy_to_string(out);
        return out;
	}

	const CURLcode get_error() const
	{
		return  m_res;
	}

    const StreamRead& get_buffer() const
    {
        return m_content;
    }

public:

    static CURL* m_curl;
    CURLcode     m_res;
    StreamRead   m_content;
};