//
//  StringUtils.h
//  Word-indexing-in-web-page
//
//  Created by Gabriele Di Bari on 10/04/15.
//  Copyright (c) 2015 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Config.h>
#include <vector>

namespace StringUtils
{
    //is a space
    static int isspace(int c)
    {
        return ((c >= 0x09 && c <= 0x0D) || (c == 0x20));
    }
    
    // trim from start
    static inline std::string &ltrim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), 
                                        s.end(), 
                                        std::not1(std::ptr_fun<int, int>(isspace))
                                        ));
        return s;
    }
    static inline std::wstring &ltrim(std::wstring &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), 
                                        s.end(), 
                                        std::not1(std::ptr_fun<int, int>(isspace))
                                        ));
        return s;
    }
    
    // trim from end
    static inline std::string  &rtrim(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(), 
                             s.rend(), 
                             std::not1(std::ptr_fun<int, int>(isspace))).base(),
                             s.end());
        return s;
    }
    static inline std::wstring &rtrim(std::wstring &s)
    {
        s.erase(std::find_if(s.rbegin(),
                             s.rend(), 
                             std::not1(std::ptr_fun<int, int>(isspace))).base(), 
                             s.end());
        return s;
    }
    
    // trim from both ends
    static inline std::string &trim(std::string &s)
    {
        return ltrim(rtrim(s));
    }
    static inline std::wstring &trim(std::wstring &s)
    {
        return ltrim(rtrim(s));
    }
    
    // remove all double space
    static void remove_double_space(std::string &s)
    {
        auto it=s.find("  ");
        //delete all double space
        while( it!=std::string::npos )
        {
            s.erase(it,1);
            it=s.find("  ");
        }
    }
    
    // split
    void split(const std::string& str ,
               const std::string& delimiters ,
               std::vector<std::string>& tokens);
    
    //replace
    void replace_all_recursive(std::string& str ,
                               const std::string& to_replace,
                               const std::string& replace_with);
    void replace_all(std::string& str ,
                     const std::string& to_replace,
                     const std::string& replace_with);
    // read all file
    static std::string file_to_string(const std::string& path)
    {
        //open
        std::ifstream file(path);
        if (!file.is_open()){ return ""; }
        //read
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
    static std::string file_to_utf8(const std::string& path)
    {
        //open
        std::ifstream file(path);
        if (!file.is_open()){ return ""; }
        //iterator
        std::istreambuf_iterator<char> it(file);
        //BOM
        if (*(it)   == (char)0xEF &&
            *(++it) == (char)0xBB &&
            *(++it) == (char)0xBF)
            //read
            return std::string(++it, std::istreambuf_iterator<char>());
        //else void
        return std::string();
    }
    static std::wstring file_to_wstring(const std::string& path)
    {
        //open
        std::wifstream file(path.c_str());
        if (!file.is_open()){ return L""; }
        //read
        return std::wstring((std::istreambuf_iterator<wchar_t>(file)), std::istreambuf_iterator<wchar_t>());
    }
    
    // to file
    static void string_to_file(const std::string& path, const std::string& str)
    {
        FILE* file = fopen(path.c_str(), "w");
        if (file)
        {
            unsigned char smarker[3] = { 0xEF, 0xBB, 0xBF };
            fwrite((void*)smarker, 3, 1, file);
            fwrite((void*)str.data(), str.size(), 1, file);
            fclose(file);
        }
    }
    static void string_to_file(const std::string& path, const std::wstring& wstr)
    {
        FILE* file = fopen(path.c_str(), "w");
        if (file)
        {
            unsigned char smarker[2] = { 0xFE, 0xFF };
            fwrite((void*)smarker, 2, 1, file);
            fwrite((void*)wstr.data(), wstr.size() * 2, 1, file);
            fclose(file);
        }
    }
    
    // utf8 to unicode and viceversa
    void from_utf8(std::wstring& outwstr, const std::string& data);
    void from_utf16(std::string& outstr, const std::wstring& data);
    void iso_latin_1_to_utf8(std::string& out, const std::string& in);
    
    // upper / lower cases
    void to_lower_utf8(std::string& outstr);
    void to_upper_utf8(std::string& outstr);

    //read utf8
    static size_t next_char_count_utf8(char c)
    {
        if ((c & 0x80) == 0)         // 0XXX XXXX
            return 1;
        else if ((c & 0xE0) == 0xC0) // 110X XXXX
            return 2;
        else if ((c & 0xF0) == 0xE0) // 1110 XXXX
            return 3;
        else if ((c & 0xF8) == 0xF0) // 1111 0XXX
            return 4;

        return (size_t)(c != 0);
    }
    static void   next_char_utf8(const char** achar)
    {
        (*achar) += next_char_count_utf8(*(*achar));
    }
    static size_t word_len_utf8(char* word)
    {
        //init index
        size_t i = 0;
        //count
        while (!isspace(*word))
        {
            word += next_char_count_utf8(*word);
            ++i;
        }
        //return len
        return i;
    }    
    static size_t text_len_utf8(char* text)
    {
        //init index
        size_t i = 0;
        //count
        while (*text)
        {
            text += next_char_count_utf8(*text);
            ++i;
        }
        //return len
        return i;
    }
    static unsigned short utf8_to_ushort(const char* achar)
    {
        unsigned short out = 0;
        //cases
        if (((*achar) & 0x80) == 0x00)
        {
            out = *achar;
        }
        /* XXX|0 0000 == 110|0 0000 */
        else if ((((*achar) & 0xE0) == 0xC0))
        {                                                    //00000
            out = (unsigned short)((achar[0]) & 0x1F) << 6;//     XXXXX
            out |= (unsigned short)((achar[1]) & 0x3F);     //          XXXXXX
        }
        /* XXXX| 0000 == 1110| 0000 */
        else if (((*achar) & 0xF0) == 0xE0)  // 3 chars
        {
            out = (unsigned short)((achar[0]) & 0x0F) << 12;//XXXX
            out |= (unsigned short)((achar[1]) & 0x3F) << 6; //    XXXXXX
            out |= (unsigned short)((achar[2]) & 0x3F);      //          XXXXXX
        }
        /* XXXX 0|000 == 1111 0|000 */
        else if (((*achar) & 0xF8) == 0xF0)  // 4 chars
        {
            out = (unsigned short)((achar[1]) & 0x3F) << 12;//XXXX
            out |= (unsigned short)((achar[2]) & 0x3F) << 6; //    XXXXXX
            out |= (unsigned short)((achar[3]) & 0x3F);      //          XXXXXX

        }

        return out;
    }
};

