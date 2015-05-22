#pragma once

#include <Config.h>

class WebPage
{
	QString m_text;
	QString m_url;
    unsigned int m_count_words;
    unsigned int m_max_lenght;

public:
	WebPage(const QString& text,
            const QString& url, 
            unsigned int count_words, 
            unsigned int max_lenght)
	{
		m_text = text;
		m_url = url;
		m_count_words = count_words;
		m_max_lenght = max_lenght;		
	}

	const QString& get_text() const
	{
		return m_text;
	}

	const QString& get_url() const
	{
		return m_url;
	}

	const unsigned int& get_count_words() const
	{
		return m_count_words;
	}

	const unsigned int& get_max_lenght() const
	{
		return m_max_lenght;
    }
    
    void save_to_file_utf16(FILE* file) const
    {
        std::fwrite(&m_count_words, sizeof(unsigned int), 1, file);
        std::fwrite(&m_max_lenght, sizeof(unsigned int), 1, file);
        
        std::wstring url = m_url.toStdWString();
        unsigned int url_size = (unsigned int)url.size();
        std::fwrite(&url_size, sizeof(unsigned int), 1, file);
        std::fwrite(url.data(), sizeof(wchar_t)*url_size, 1, file);
        
        std::wstring text = m_text.toStdWString();
        unsigned int text_size = (unsigned int)text.size();
        std::fwrite(&text_size, sizeof(unsigned int), 1, file);
        std::fwrite(text.data(), sizeof(wchar_t)*text_size, 1, file);
    }
    
    void save_to_file_utf8(FILE* file) const
    {
        //write number of words
        std::fwrite(&m_count_words, sizeof(unsigned int), 1, file);
        //write max size
        unsigned int max_lenght_utf8=m_max_lenght*2;
        std::fwrite(&max_lenght_utf8, sizeof(unsigned int), 1, file);
        //write url
        QByteArray url = m_url.toUtf8();
        unsigned int url_size = (unsigned int)url.size();
        std::fwrite(&url_size, sizeof(unsigned int), 1, file);
        std::fwrite(url.data(), url.size(), 1, file);
        //write text
        QByteArray text = m_text.toUtf8();
        unsigned int text_size = (unsigned int)text.size();
        std::fwrite(&text_size, sizeof(unsigned int), 1, file);
        std::fwrite(text.data(), text.size(), 1, file);
    }
};