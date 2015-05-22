#include <Config.h>
#include <ParseWebPageText.h>
#include <DownloadWebPage.h>
#include <WebPage.h>
#include <iostream>
#include <vector>
#include <functional>
//n.b. use -fshort-wchar (in clang and gcc)
/* ---------------------------------------------------------------------------------------------------------- */
// SAVE UTF16
inline void save_web_pages_utf16(const QString& file_name, const std::vector< std::shared_ptr< WebPage > >& pages)
{
    FILE *file = fopen(file_name.toStdString().c_str(), "wb");
    if (file)
    {
        //save type
        unsigned short type_tile = 16;
        std::fwrite(&type_tile, sizeof(unsigned short), 1, file);
        //save size
        unsigned int pages_size = (unsigned int)pages.size();
        std::fwrite(&pages_size, sizeof(unsigned int), 1, file);
        //save all page
        for (auto& page : pages)
        {
            page->save_to_file_utf16(file);
        }
        fclose(file);
    }
}
// SAVE UTF8
inline void save_web_pages_utf8(const QString& file_name, const std::vector< std::shared_ptr< WebPage > >& pages)
{
    FILE *file = fopen(file_name.toStdString().c_str(), "wb");
    if (file)
    {
        //save type
        unsigned short type_tile = 8;
        std::fwrite(&type_tile, sizeof(unsigned short), 1, file);
        //save size
        unsigned int pages_size = (unsigned int)pages.size();
        std::fwrite(&pages_size, sizeof(unsigned int), 1, file);
        //save all page
        for (auto& page : pages)
        {
            page->save_to_file_utf8(file);
        }
        fclose(file);
    }
}
/* ---------------------------------------------------------------------------------------------------------- */



#define PARALLEL_DOWNLOAD
typedef std::vector < QString > VQString;
typedef std::vector < DownloadWebPage > VDPage;
typedef std::vector< std::shared_ptr< WebPage > > VPWPage;
class Task : public QRunnable
{
    std::function< void() > m_lambda;

public:

    Task(std::function< void() > lambda) 
    :m_lambda(lambda)
    {
    }

    void run()
    {
        m_lambda();
    }
};


int main(int argc, char** args)
{
    QApplication app(argc, args);
    //set size of index
    const size_t number_of_threads = 4;
    const unsigned int n_web_page = 39;
    //vector of pages
    VPWPage pages;
    
    #ifdef PARALLEL_DOWNLOAD
    
    //init size of pool threads
    QThreadPool::globalInstance()->setMaxThreadCount(number_of_threads);
    QMutex mutex;
    //vector of pages
    VDPage  downloaded;
    
    #endif
    
    //download all page
	for (unsigned int j = 1; j != n_web_page; ++j)
	{
		// Get HTML Source
        QString index_url( "http://www.treccani.it/enciclopedia/elenco-opere/Enciclopedia_della_Scienza_e_della_Tecnica/" + QString::number(j) + "/" );
		QString index = DownloadWebPage(QString(index_url));
        std::cout << "--------------------------------------------" << std::endl;
        std::cout << index_url.toStdString() << std::endl;
		// Cut HTML Source
		index = index.mid(index.indexOf("cont-lemmi") - 16);
		index = index.left(index.indexOf("pag-lemmi") - 12);
		// Parse HTML
		QWebPage page;
		QWebFrame& frame = *page.mainFrame();
		frame.setHtml(index);
		QWebElement document = frame.documentElement();
		QWebElementCollection anchors = document.findAll("a");
        
		//download
		#ifdef PARALLEL_DOWNLOAD
		//start download
		for (unsigned int i = 0; i != anchors.count(); ++i)
		{
			const QWebElement& anchor = anchors.at(i);
			if (!anchor.attribute("href").isEmpty())
			{
				QString link = "http://www.treccani.it" + anchor.attribute("href");
				QThreadPool::globalInstance()->start(
					new Task([link, &downloaded,&mutex]()
					{
						DownloadWebPage page(link);
						//save push
						mutex.lock();
						downloaded.push_back(page);
						mutex.unlock();
						//debug
						std::cout << "Download: " << (link).toStdString() << std::endl;
					})
				);
			}			
		}
		#else
		    //download & parsed
			for(unsigned int i = 0; i != anchors.count(); ++i)
			{
				const QWebElement& anchor = anchors.at(i);
				if (!anchor.attribute("href").isEmpty())
				{
					QString link = "http://www.treccani.it" + anchor.attribute("href");
					pages.push_back(ParseWebPageText(link));
					std::cout << "Downloaded & Parsed: " << (link).toStdString() << std::endl;
				}
			}	
		#endif
	}
    
    //parsing
    #ifdef PARALLEL_DOWNLOAD
    //wait download
    QThreadPool::globalInstance()->waitForDone();
    //do parsing
    size_t i = 0;
    pages.resize(downloaded.size());
    for (auto& adownload : downloaded)
    {
        pages[i++] = ParseWebPageText(adownload);
        std::cout << "Parsed: " << pages[i-1]->get_url().toStdString() << std::endl;
    }
    #endif
    //save
    save_web_pages_utf8("treccani.pages",pages);

	return 0;
}