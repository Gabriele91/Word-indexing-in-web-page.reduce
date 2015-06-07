#include <StringUtils.h>
#include <codecvt>
#include <clocale>
#include <cstdlib>
#if _WIN32
#include <Windows.h>
#endif

void StringUtils::from_utf8(std::wstring& outwstr, const std::string& data)
{
#ifndef _WIN32
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    outwstr=converter.from_bytes(data);
#else
    int len = MultiByteToWideChar(CP_UTF8, 0, data.data(), -1, NULL, 0);
    if (len > 1)
    {
        outwstr.resize(len - 1);
        wchar_t* ptr = &outwstr[0];
        MultiByteToWideChar(CP_UTF8, 0, data.data(), -1, ptr, len);
    }
#endif 
}
void StringUtils::from_utf16(std::string& outstr, const std::wstring& data)
{
#ifndef _WIN32
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    outstr=converter.to_bytes(data);
#else
    int len = WideCharToMultiByte(CP_UTF8, 0, data.c_str(), -1, NULL, 0, 0, 0);
    if (len > 1)
    {
        outstr.resize(len - 1);
        char* ptr = &outstr[0];
        WideCharToMultiByte(CP_UTF8, 0, data.c_str(), -1, ptr, len, 0, 0);
    }
#endif 
}
void StringUtils::iso_latin_1_to_utf8(std::string& out, const std::string& in)
{
    //rinit
    out = "";
    //
    for (unsigned char c : in)
    {
        if (c < 128)
        {
            out += c;
        }
        else
        {
            out += 0xC0 | ((c >> 6) & 0x1f);
            out += 0x80 | (c & 0x3f);
        }
    }
}
void StringUtils::split(const std::string& str ,
                        const std::string& delimiters ,
                        std::vector<std::string>& tokens)
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
    
    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

void StringUtils::replace_all_recursive(std::string& str ,
                                        const std::string& to_replace,
                                        const std::string& replace_with)
{
    if(to_replace.empty()) return;
    //first
    long lfind=str.find(to_replace);
    //all for all
    while(lfind>-1)
    {
        str.replace(lfind,to_replace.size(),replace_with);
        lfind=str.find(to_replace);
    }
}


void StringUtils::replace_all(std::string& str ,
                              const std::string& to_replace,
                              const std::string& replace_with)
{
    if(to_replace.empty()) return;
    //first
    long lfind=str.find(to_replace);
    //all for all
    while(lfind>-1)
    {
        str.replace(lfind,to_replace.size(),replace_with);
        lfind=str.find(to_replace,lfind+replace_with.size());
    }
}

