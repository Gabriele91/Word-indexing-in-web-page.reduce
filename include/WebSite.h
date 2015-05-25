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
    //compute size
    void compute_sizes()
    {
        //ptr to str
        const char* c_str     = m_text.c_str();
        const char* c_ext_str = m_text.c_str();
        //count
        unsigned int size_word = 0;
        //it attrs
        m_count_words = 0;
        m_size_word = 0;
        //for all words
        while (*c_str)
        {
            //next char utf8
            StringUtils::next_char_utf8(&c_ext_str);
            //compute...
            bool is_space      = StringUtils::isspace(*c_str);
            bool next_is_space = StringUtils::isspace(*c_ext_str) || !(*c_ext_str);
            if (!is_space && next_is_space)
            {
                //update count of words
                ++m_count_words;
                //update char count
                size_word += c_ext_str - c_str;
                //get max
                m_size_word = std::max(m_size_word, size_word);
                //reset
                size_word=0;
            }
            else if (!is_space)
            {
                size_word += c_ext_str - c_str;
            }
            //next
            c_str = c_ext_str;
        }
    }
    //aux to clean text
    static bool isspace(const char* c, bool noalpha_are_spaces)
    {
        return StringUtils::isspace(*c) || 
              (
               noalpha_are_spaces &&
               !iswalpha((wint_t)StringUtils::utf8_to_ushort(c))
              );
    }
    //clean text
    void clean_text(bool noalpha_are_spaces = true)
    {
        //ptr to str
        const char* c_str     = m_text.c_str();
        const char* c_ext_str = m_text.c_str();
        //ptr to word
        const char* c_word = nullptr;
        //new text
        auto buffer = std::make_unique<char[]>(m_text.size() + 2);
        std::memset(buffer.get(), 0, m_text.size()+1);
        //ptr to buffer
        char* c_now = buffer.get();
        //for all words
        while (*c_str)
        {
            //next char utf8
            StringUtils::next_char_utf8(&c_ext_str);
            //
            bool is_space      = isspace(c_str, noalpha_are_spaces);
            bool next_is_space = isspace(c_ext_str, noalpha_are_spaces) || !(*c_ext_str);
            //init ptr to word
            if (!is_space && !c_word) c_word = c_str;
            //copy
            if (next_is_space && c_word)
            {
                //copy word
                size_t size = c_str - c_word + 1;
                std::memcpy(c_now, c_word, size);
                c_now += size; 
                //add space
                (*c_now) = ' ';
                ++c_now;
                //reset ptr to word
                c_word=nullptr;
            }
            //next
            c_str = c_ext_str;
        }
        //if save some words, delete last space
        if (c_now != buffer.get()) (*(c_now-1))='\0';
        //save
        m_text = buffer.get();
    }

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
    
    void save_to_file(FILE* file) const
    {
        //write count of words
        std::fwrite(&m_count_words, sizeof(unsigned int), 1, file);
        //write max size of words
        std::fwrite(&m_size_word, sizeof(unsigned int), 1, file);
        //size of url
        unsigned int url_size = m_url.size();
        std::fwrite(&url_size, sizeof(unsigned int), 1, file);
        //write url
        std::fwrite(&m_url[0], url_size, 1, file);
        //size of text
        unsigned int text_size = m_text.size();
        std::fwrite(&text_size, sizeof(unsigned int), 1, file);
        //write text
        std::fwrite(&m_text[0], text_size, 1, file);
    }

    unsigned int get_count_words() const
    {
        return m_count_words;
    }

    unsigned int get_max_size_word() const
    {
        return m_size_word;
    }

    const std::string& get_text() const
    {
        return m_text;
    }

    const std::string& get_url() const 
    {
        return m_url;
    }

    void set_url(const std::string& url)
    {
        m_url = url;
    }

    void set_text(unsigned int count_words, unsigned int size_word, const std::string& text)
    {
        m_count_words = count_words;
        m_size_word = size_word;
        m_text = text;
    }

    void set_text(const std::string& text)
    {
        m_text = text;
        compute_sizes();
    }

    void set_text_raw(const std::string& text)
    {
        m_text = text;
        clean_text();
        compute_sizes();
    }
};


class WebSite : public GCPointer< WebSite >
{
    std::vector< WebPage::ptr > m_pages;

public:

    WebSite()
    {
    }
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

    void save_to_file(const std::string& path) const
    {
        //try open
        FILE* file = fopen(path.c_str(), "wb");
        //if is opened:
        if (file)
        {
            save_to_file(file);
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

    void save_to_file(FILE* file) const
    {
        //set type
        unsigned short type_file = 8;
        std::fwrite(&type_file, sizeof(unsigned short), 1, file);
        //get size
        unsigned int pages_size = size();
        std::fwrite(&pages_size, sizeof(unsigned int), 1, file);
        //write
        for (unsigned int i = 0; i != pages_size; ++i)
        {
            m_pages[i]->save_to_file(file);
        }
        //end
    }

    void append(WebPage::ptr web_page)
    {
        m_pages.push_back(web_page);
    }

    void add(WebPage::ptr page)
    {
        m_pages.push_back(page);
    }

    size_t size() const
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