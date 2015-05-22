#pragma once

#include <Config.h>
#include <DownloadWebPage.h>
#include <WebPage.h>
#include <memory>

class ParseWebPageText
{
    std::shared_ptr< WebPage > m_web_page;

public:

    ParseWebPageText():m_web_page(nullptr){};
    ParseWebPageText(const QString& url)
    {
        parsing(DownloadWebPage(url),url);
    }
    ParseWebPageText(const DownloadWebPage& download_page)
    {
        parsing(download_page, download_page.get_url());
    }

	const  std::shared_ptr< WebPage > & get_text() const
	{
		return m_web_page;
	}

    operator const  std::shared_ptr< WebPage > & () const
	{
		return m_web_page;
	}
    
    void parsing(const QString& source, const QString& url);
};

inline void ParseWebPageText::parsing(const QString& source, const QString& url)
{
    //text
    QString source_text;
    // Cut HTML Source
    source_text = source.mid(source.indexOf("text-enciclopedia") - 25);
    source_text = source_text.left(source_text.indexOf("<!--  include \"box_lemma_foto.html\"  -->"));

    // Parse HTML
    QWebPage page;
    QWebFrame& frame = *page.mainFrame();
    frame.setHtml(source_text);
    QWebElement document = frame.documentElement();


    // Get Plain Text
    QString text_full = document.toPlainText();

    // Elaborate Text
    QString text;
    QString current_word;
    unsigned int count = 0;
    unsigned int max_lenght = 0;
    unsigned int word_counter = 0;
    for (unsigned int i = 0; i != text_full.size(); ++i)
    {
        if (text_full[i].isLetter())
        {
            current_word.append(text_full[i]);
            ++count;
            if (i == text_full.size() - 1)
            {
                if (count > 1)
                {
                    text.append(current_word + " ");
                    if (current_word.size() > max_lenght)
                    {
                        max_lenght = current_word.size();
                    }
                    ++word_counter;
                }
                current_word = "";
                count = 0;
            }
        }
        else
        {
            if (count > 1)
            {
                text.append(current_word + " ");
                if (current_word.size() > max_lenght)
                {
                    max_lenght = current_word.size();
                }
                ++word_counter;
            }
            current_word = "";
            count = 0;
        }
    }

    // Save Text
    m_web_page = std::shared_ptr< WebPage >(new WebPage(text, url, word_counter, max_lenght));
}