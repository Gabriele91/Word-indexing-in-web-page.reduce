#include <GetHTTP.h>
#include <StringUtils.h>
//init curl
CURL* GetHTTP::m_curl = nullptr;

//append
void GetHTTP::StreamRead::append(void* ptr, size_t add_size)
{
    size_t last_size=m_buffer.size();
    m_buffer.resize(last_size+add_size);
    memcpy(&m_buffer[last_size], ptr, add_size);
}
//add to string (and relase context)
void GetHTTP::StreamRead::copy_to_string(std::string& outstr)
{
    m_buffer.resize(m_buffer.size() + 1);
    //safe end line
    m_buffer[m_buffer.size() - 1] = '\0';
    //copy to string
    outstr = m_buffer.data();
    //release
    m_buffer.clear();
    m_buffer.shrink_to_fit();
}
//add to string (and relase context)
void GetHTTP::StreamRead::copy_to_string(std::wstring& outstr)
{
    std::string bufferUtf8;
    copy_to_string(bufferUtf8);
    StringUtils::from_utf8(outstr, bufferUtf8);
}

static size_t download_content(void* ptr, size_t size, size_t nmemb, GetHTTP::StreamRead* buffer)
{
	size_t c_size = size*nmemb;
    //alloc not alloc
    buffer->append(ptr, c_size);
    //return size
	return c_size;
}

GetHTTP::GetHTTP(const std::string& url)
{
	//init curl
	if NOT(m_curl)
	{
		m_curl = curl_easy_init();
		assert(m_curl);
		atexit([]()
		{
			curl_easy_cleanup(m_curl);
		});
	}
	// download page
	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, download_content);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_content);
	//execute download
	m_res = curl_easy_perform(m_curl);
	//take a error
	if (m_res != CURLE_OK)
	{
		MESSAGE("curl_easy_perform() failed: " << curl_easy_strerror(m_res));
	}
}