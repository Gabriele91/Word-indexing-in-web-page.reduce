typedef struct __InfoMap
{
    unsigned int m_max_row;
    unsigned int m_max_size_word;
    unsigned int m_count_words;
    unsigned int m_text_start;
    unsigned int m_rows_start;
} InfoMap;

typedef struct __GlobalInfo
{
    unsigned int m_pages;
    unsigned int m_cl_start;
    unsigned int m_start;
    unsigned int m_end;
    unsigned int m_info_size;
    unsigned int m_min_len;
    bool         m_case_sen;
    
} GlobalInfo;
/* ------------------------------------------------------------------ */
#define ADDSQ global
#include <map_row.cl>
#include <utf8_utils.cl>
#include <word_utils.cl>
#include <debug.cl>
/* ------------------------------------------------------------------ */

inline void add_row(ADDSQ Row* row, 
                    ADDSQ char* word)
{ 
    //init count
    row->m_count = 1;
    //add new word
    copy_word(row->m_word, word);
    //debug
    print_cstr(" added: ");
    print_row(row);
}

void update_map(unsigned int   row_size,
                ADDSQ    char* word,
                ADDSQ    char* raw_rows)
{
    //index
    unsigned int i = 0;
    //init loop
    ADDSQ Row* row = get_row(row_size, raw_rows, i);
    //debug
    print_row(row);
    //if is olready added
    while ( row->m_count )
    {
        //compare
        if (compare_word(row->m_word, word))
        {
            ++row->m_count;
            return;
        }
        //next
        ++i;
        row = get_row(row_size, raw_rows, i);
        //debug
        print_row(row);
    }
    //-------------
    add_row(row, word);
}

bool next_word( ADDSQ char** text )
{
    while((*(*text)) == ' ') ++(*text);
    return ((*(*text)) != '\0');
}

void eat_word( ADDSQ char**  text )
{
    while(!is_end_char(*(*text))) ++(*text);
}

bool not_ignore( read_only ADDSQ  char* words_ignore, ADDSQ  char* word )
{
    //loop
    while ( next_word(&words_ignore) )
    {
        if( compare_word( words_ignore, word ) )
        {
            return false;
        }
        eat_word(&words_ignore);
    }
    return true;
}

bool no_case_sensitive_not_ignore( read_only ADDSQ  char* words_ignore, ADDSQ  char* word )
{
    //loop
    while ( next_word(&words_ignore) )
    {
        if( compare_word_no_case_sensitive( words_ignore, word ) )
        {
            return false;
        }
        eat_word(&words_ignore);
    }
    return true;
}

kernel void mapping_site( read_only  global GlobalInfo*  ginfo,
                          read_only  ADDSQ  char*        raw_wign,
                                     ADDSQ  char*        raw_info,
                                     ADDSQ  char*        raw_text,
                                     ADDSQ  char*        raw_rows )
{
    //get global id
    unsigned int g_id          = get_global_id(0);
    //get global page
    unsigned int page_id       = g_id + ginfo->m_cl_start;
    //get global page
    unsigned int page_site_id  = g_id + ginfo->m_start;
    //get header page
    ADDSQ InfoMap* info   = (ADDSQ InfoMap*)(raw_info + ginfo->m_info_size * page_id);
    //get text
    ADDSQ char* text      = (ADDSQ char*)(raw_text + info->m_text_start);
    //get rows
    ADDSQ char* rows      = (ADDSQ char*)(raw_rows + info->m_rows_start);
    //debug
    CPU(
    printf("I'm %d, byte of header %d, %d and %d, %d, %d, %d, %d\n",
           g_id,
           ginfo->m_info_size * page_id,
           ginfo->m_min_len,
           info ->m_max_size_word,
           info ->m_text_start,
           info ->m_rows_start,
           info ->m_max_row,
           info ->m_max_size_word
        );
    )
    ////////////////////////////////////////////////////////////////////////////
    //for all words  //ignore case sensitive
    if( ginfo->m_case_sen )
    {
        while ( next_word(&text) )
        {
            //debug
            CPU( printf("I'm %d, case sensitive, at ",g_id); )
            print_word(text);
            
            //ignore?  Not case sensitive
            if( (ginfo->m_min_len <= word_len(text)) && not_ignore ( raw_wign, text ) )
            {
                update_map(info->m_max_row, text, rows);
            }
                
            //next
            eat_word(&text);
            
            //debug
            print_endl();
        }
    }
    ////////////////////////////////////////////////////////////////////////////
    //for all words  //ignore not case sensitive
    else
    {
        while ( next_word(&text) )
        {
            //debug
            CPU( printf("I'm %d, not case sensitive, at ",g_id); )
            print_word(text);
            
            //ignore?  case sensitive
            if ( (ginfo->m_min_len <= word_len(text)) && no_case_sensitive_not_ignore ( raw_wign, text )  )
            {
                update_map(info->m_max_row, text, rows);
            }
                
            //next
            eat_word(&text);
            
            //debug
            print_endl();
        };
    }
    ////////////////////////////////////////////////////////////////////////////
}