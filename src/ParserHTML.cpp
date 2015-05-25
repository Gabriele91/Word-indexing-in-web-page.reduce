#define AUTO_PTR_BROKEN
#include <ParserHTML.h>
#include <html2text/html.h>
#include <html2text/HTMLControl.h>
#include <html2text/urlistream.h>
#include <ostream>
#include <StringUtils.h>
#include <limits>
////////////////////////////
//encoding HTML PARSER
int use_encoding = UTF8;
////////////////////////////

template<class T>
class Buffer : urlistream
{
    const T& m_bf;
    size_t   m_it;

public:

    Buffer(const T& buffer)
    :m_bf(buffer)
    ,m_it(0)
    {
    
    }

    virtual int get()
    {
        if (m_it != m_bf.size())
        {
            return (int)m_bf[m_it++];
        }
        else
        {
            return (int)EOF;
        }
    }

    urlistream& get_urlistream()
    {
        return *((urlistream*)this);
    }
}; 

class Parser : public HTMLControl 
{

public:

    Parser(urlistream      &s_in,
           bool             enable_debug_scanner,
           bool             enable_debug_parser,
           std::function<void(const Document &document)> callback)
    :HTMLControl(s_in,
                 enable_debug_scanner,
                 enable_debug_parser),
    m_callback(callback)
    {
    }

private:

    void yyerror(char *p)
    {
        //print error
        //MESSAGE( "Line " << current_line << ", column " << current_column << ": " << p );
    }

    void process(const Document &document)
    {
        if (m_callback) m_callback(document);
    }

    std::function<void(const Document &document)> m_callback;
};

/* -------------------------------------------------------------- */
//functions declaration
static void for_each_elements(
    Document* document,
    std::function<void(auto_ptr< Element >& element)> callback
    );

static void for_each_elements(
    Body* body,
    std::function<void(auto_ptr< Element >& element)> callback
    );

static void for_each_elements_list(
    std::list< auto_ptr< Element > >* elements,
    std::function<void(auto_ptr< Element >& element)> callback
    );

static void for_each_elements_unorderedList(
    UnorderedList* element,
    std::function<void(auto_ptr< Element >& element)> callback
    );

static void for_each_elements_orderedList(
    OrderedList* element,
    std::function<void(auto_ptr< Element >& element)> callback
    );

static void for_each_elements_definitionList(
    DefinitionList* element,
    std::function<void(auto_ptr< Element >& element)> callback
    ); 

static void for_each_elements_table(
    Table* element,
    std::function<void(auto_ptr< Element >& element)> callback
    );
/* -------------------------------------------------------------- */
//functions implementation
static void for_each_elements(
    Document* document,
    std::function<void(auto_ptr< Element >& element)> callback
    )
{
    for_each_elements(&document->body, callback);
}

static void for_each_elements(
    Body* body,
    std::function<void(auto_ptr< Element >& element)> callback
    )
{
    for_each_elements_list(body->content.get(), callback);
}

static void for_each_elements_list(
    std::list< auto_ptr< Element > >* elements,
    std::function<void(auto_ptr< Element >& element)> callback
    )
{
    //recursive
    for (auto& it : (*elements))
    {
        callback(it);
        //get tag
        std::string tag(it->get_name());
        //cases
        if      (tag == "Table")          for_each_elements_table((Table*)it.get(), callback);
        else if (tag == "OrderedList")    for_each_elements_orderedList((OrderedList*)it.get(), callback);
        else if (tag == "UnorderedList")  for_each_elements_unorderedList((UnorderedList*)it.get(), callback);
        else if (tag == "DefinitionList") for_each_elements_definitionList((DefinitionList*)it.get(), callback);
        //ric
        if (it->get_content())            for_each_elements_list(it->get_content(), callback);
    };
}

static void for_each_elements_unorderedList(
    UnorderedList* element,
    std::function<void(auto_ptr< Element >& element)> callback
    )
{
    //recursive call
    if (element->items.get())
        for (auto& item : *(element->items))
        {
            //get list of childs
            auto elements = item->get_content();
            //recursive
            if (elements)
                for_each_elements_list(elements, callback);
        };
}

static void for_each_elements_orderedList(
    OrderedList* element, 
    std::function<void(auto_ptr< Element >& element)> callback
    )
{
    //recursive call
    if (element->items.get())
    for (auto& item : *(element->items))
    {
        //get list of childs
        auto elements = item->get_content();
        //recursive
        if (elements)
            for_each_elements_list(elements, callback);
    };
}

static void for_each_elements_definitionList(
    DefinitionList* element,
    std::function<void(auto_ptr< Element >& element)> callback
    )
{
    //recursive call
    if (element->items.get())
        for (auto& item : *(element->items))
        {
            //get list of childs
            auto elements = item->get_content();
            //recursive
            if (elements)
                for_each_elements_list(elements, callback);
        };
}
static void for_each_elements_table(
    Table* element,
    std::function<void(auto_ptr< Element >& element)> callback
    )
{
    for (auto& row : *(element->rows))
    {
        for (auto& cell : *(row->cells))
        {
            for_each_elements_list(cell->content.get(), callback);
        }
    }
}
/* -------------------------------------------------------------- */
static void area_to_buff_string(ostream &os, const Area &a)
{
    for (Area::size_type y = 0; y < a.height(); y++)
    {
        const Cell *cell = a[y], *end = cell + a.width();
        while (end != cell && end[-1].character == ' ' &&
            (end[-1].attribute & (Cell::UNDERLINE | Cell::STRIKETHROUGH)) == 0)
            end--;

        for (const Cell *p = cell; p != end; p++)
        {
            char c = p->character;
            char a = p->attribute;

            if (c == (char)LATIN1_nbsp && !USE_UTF8) c = ' ';

            if (a == Cell::NONE)
            {
                os << c;
            }
            else
            {
                os << c;
            }
        }
        os << std::endl;
    }
}
static void area_to_string(std::string& out, const Area &a)
{
    for (Area::size_type y = 0; y < a.height(); y++)
    {
        const Cell *cell = a[y], *end = cell + a.width();
        while (end != cell && end[-1].character == ' ' &&
            (end[-1].attribute & (Cell::UNDERLINE | Cell::STRIKETHROUGH)) == 0)
            end--;

        for (const Cell *p = cell; p != end; p++)
        {
            char c = p->character;
            char a = p->attribute;

            if (c == (char)LATIN1_nbsp && !USE_UTF8) c = ' ';

            if (a == Cell::NONE)
            {
                out.push_back(c);
            }
            else
            {
                out.push_back(c);
            }
        }
        out.push_back('\n');
    }
}
/* -------------------------------------------------------------- */

ParserHTML::ParserHTML(const GetHTTP& webpage)
{
    Buffer< GetHTTP::StreamRead::Bytes > buffer(webpage.get_buffer().m_buffer);
    Parser parser(buffer.get_urlistream(), false, false, 
    [this](const Document &document)
    {
        std::ostringstream output;
        //get title
        m_title=document.head.title->text;
        //html to string
        area_to_string(m_text, *document.format(79, Area::LEFT));
        //get links (filter function)
        auto get_link = [this](auto_ptr< Element >& element)
        {
            std::string tag(element->get_name());
            //if "a" tag
            if (tag == "Anchor")
            {
                //get attribute href
                auto attributes=element->get_attributes();
                if (attributes)
                {
                    bool exists=false;
                    std::string link = get_attribute(attributes, "href", &exists);
                    if (exists)  m_links.push_back(link);
                }
            }
        };
        //get links from body
        for_each_elements((Document*)&document, get_link);
    });
    parser.yyparse();
    //save url
    m_url = webpage.get_url();
}