#pragma once

#include <Config.h>
#include <SiteMapping.h>

class WordIndices
{
public:
    
    struct RowItem
    {
        std::string		m_url;
        size_t			m_count ;
    };
    
	WordIndices(const std::string& word, const RowItem&& item)
	{
		m_word = word;
		m_indices.push_back(std::move(item));
	}

	const std::string& get_word() const
	{
		return m_word;
	}

	size_t count_indices() const
	{
		return m_indices.size();
	}

	const std::string& get_url(size_t i) const
	{
		return m_indices.at(i).m_url;
	}

	size_t get_count(size_t i) const 
	{
		return m_indices.at(i).m_count;
	}
    
    void add_index(const RowItem&& item)
    {
        m_indices.push_back(std::move(item));
    }
    
public:
    
    std::string				m_word;
    std::vector<RowItem>	m_indices;

};

class InvertedIndex
{
	std::vector<WordIndices> m_inverted_index;

public:
    
    InvertedIndex(){}
    InvertedIndex(OpenCLQueue::ptr queue,
                  WebSiteMapping&  site,
                  size_t first,
                  size_t last)
    {
        read_result_from_mapping(queue, site,first,last);
    }
    
    InvertedIndex(WebSite::ptr web_size,
                  const std::vector<  PageMapping::Map::ptr  >& maps)
    {
        read_result_from_mapping(web_size,maps);
    }
    
    void read_result_from_mapping(OpenCLQueue::ptr queue,
                                  WebSiteMapping& site_buffer,
                                  size_t first,
                                  size_t last) 
    {
        for(size_t i=first; i!=last; ++i)
        {
            auto page_map=site_buffer.read_map(queue,i);
            auto web_page=site_buffer.get_page(i);
            
            for (size_t j = 0; j != page_map->size() && page_map->at(j)->m_count;  ++j)
            {
                add_map_word(page_map->at(j)->m_word,
                             web_page->get_url(),
                             page_map->at(j)->m_count);
            }
        }
    }
    
    
    void read_result_from_mapping(WebSite::ptr web_size,
                                  const std::vector<  PageMapping::Map::ptr  >& maps)
    {
        size_t i=0;
        //all maps
        for(auto& map:maps)
        {
            //all words
            for (size_t j = 0; j != map->size() && map->at(j)->m_count;  ++j)
            {
                add_map_word( map->at(j)->m_word,
                              web_size->at(i)->get_url(),
                              map->at(j)->m_count );
            }
            //next site
            ++i;
        }
    }

	void add_map_word(const std::string& word, const std::string& url, size_t count)
	{
		bool found   = false;
		size_t index = 0;
        for (auto& wi : m_inverted_index)
		{
			if (wi.get_word() == word)
			{
				found = true;
				break;
			}
            ++index;
		}

		if (found)
		{
            m_inverted_index[index].add_index({url, count});
		}
		else
		{
            m_inverted_index.push_back(std::move(WordIndices(word, {url, count})));
		}
	}
    
    
    size_t size()
    {
        return m_inverted_index.size();
    }
    
    std::vector<WordIndices>::iterator begin()
    {
        return m_inverted_index.begin();
    }
    
    std::vector<WordIndices>::const_iterator begin() const
    {
        return m_inverted_index.begin();
    }
    
    std::vector<WordIndices>::iterator end()
    {
        return m_inverted_index.end();
    }
    
    std::vector<WordIndices>::const_iterator end() const
    {
        return m_inverted_index.end();
    }
    
    const WordIndices& at(size_t i) const
    {
        return m_inverted_index.at(i);
    }
    
    const WordIndices& operator [] (size_t i) const
    {
        return m_inverted_index.at(i);
    }


};