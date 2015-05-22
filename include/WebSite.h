#pragma once
#include <Config.h>
#include <GCPointer.h>
#include <StringUtils.h>

class WebPage : public GCPointer< WebPage >
{
    unsigned int m_count_words { 0 };
    unsigned int m_size_word   { 0 };
    std::string m_url;
    std::string m_text;

public:

    WebPage(){}


    WebPage(unsigned int count_words,
            unsigned int size_word,
            const std::string& url,
            const std::string& text)
    {
        init(count_words, size_word, url, text);
    }
    ~WebPage() { }

    void init(unsigned int count_words,
              unsigned int size_word,
              const std::string& url,
              const std::string& text)
    {
        m_count_words = count_words;
        m_size_word = size_word;
        m_url = url;
        m_text = text;
    }

    void init_from_utf8(FILE* file)
    {
        //read count of words
        std::fread(&m_count_words, sizeof(unsigned int), 1, file);
        //read max size of words
        std::fread(&m_size_word, sizeof(unsigned int), 1, file);
        //size of url
        unsigned int url_size { 0 };
        std::fread(&url_size, sizeof(unsigned int), 1, file);
        //read url
        m_url.resize(url_size);
        std::fread(&m_url[0], url_size, 1, file);
        //size of text
        unsigned int text_size{ 0 };
        std::fread(&text_size, sizeof(unsigned int), 1, file);
        //read text
        m_text.resize(text_size);
        std::fread(&m_text[0], text_size, 1, file);
    }

    void init_from_utf16(FILE* file)
    {
        //read count of words
        std::fread(&m_count_words, sizeof(unsigned int), 1, file);
        //read max size of words
        std::fread(&m_size_word, sizeof(unsigned int), 1, file);
        //size of url
        unsigned int url_size{ 0 };
        std::fread(&url_size, sizeof(unsigned int), 1, file);
        //read url
        std::wstring tmp_url(L' ', url_size);
        std::fread(&tmp_url[0], url_size, 1, file);
        //size of text
        unsigned int text_size{ 0 };
        std::fread(&text_size, sizeof(unsigned int), 1, file);
        //read text
        std::wstring tmp_text(L' ', text_size);
        std::fread(&tmp_text[0], text_size, 1, file);
        //utf16 to utf8
        m_size_word *= 2;
        StringUtils::from_utf16(m_url, tmp_url);
        StringUtils::from_utf16(m_text, tmp_text);
    }
    
    unsigned int get_count_words()
    {
        return m_count_words;
    }

    unsigned int get_max_size_word()
    {
        return m_size_word;
    }

    const std::string& get_text()
    {
        return m_text;
    }

    const std::string& get_url()
    {
        return m_url;
    }
    
};


class WebSite : public GCPointer< WebSite >
{
    std::vector< WebPage::ptr > m_pages;

public:

    WebSite(){}
    WebSite(const std::string& path)
    {
        init_from_file(path);
    }

    void init_from_file(const std::string& path)
    {
        //try open
        FILE* file=fopen(path.c_str(),"rb");
        //if is opened:
        if (file)
        {
            init_from_file(file);
            fclose(file);
        }
    }

    void init_from_file(FILE* file)
    {
        //get type
        unsigned short type_file = 0;
        std::fread(&type_file, sizeof(unsigned short), 1, file);
        //get size
        unsigned int pages_size = 0;
        std::fread(&pages_size, sizeof(unsigned int), 1, file);
        //resize vector
        m_pages.resize((size_t)pages_size);
        //read
        switch (type_file)
        {
        case 8:
            for (unsigned int i = 0; i != pages_size; ++i)
            {
                m_pages[i] = (new WebPage())->shared();
                m_pages[i]->init_from_utf8(file);
            }
            break;
        case 16:
            for (unsigned int i = 0; i != pages_size; ++i)
            {
                m_pages[i] = (new WebPage())->shared();
                m_pages[i]->init_from_utf16(file);
            }
            break;
        default: MESSAGE("Readed a not valid WebSize's file") break;
        }
        //end
    }

    void append(WebPage::ptr web_page)
    {
        m_pages.push_back(web_page);
    }

    size_t size()
    {
        return m_pages.size();
    }
    
    WebPage::ptr at (size_t i)
    {
        return m_pages[i];
    }
    
    const WebPage::ptr at (size_t i) const
    {
        return m_pages[i];
    }
    
    WebPage::ptr operator[] (size_t i)
    {
        return m_pages[i];
    }

    const WebPage::ptr operator[] (size_t i) const
    {
        return m_pages[i];
    }

    void clear()
    {
        m_pages.clear();
    }

};