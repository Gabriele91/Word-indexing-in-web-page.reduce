
/* ------------------------------------------------------------------ */
#define ADDSQ global
#include <map_row.cl>
#include <utf8_utils.cl>
#include <word_utils.cl>
#include <debug.cl>
/* ------------------------------------------------------------------ */
typedef struct __attribute__((packed)) __MapsInfo
{
    unsigned short m_row;   //< row size in bytes
    unsigned int   m_size;  //< row count
    unsigned int   m_offset;
    unsigned int   m_page;
} MapsInfo ;


void add_a_map(
               //input map
               const uint  page_map_rows,
               const uint  page_map_rows_size,
               ADDSQ char* page_map,
               //ouput reduce map
               const  uint word_capacity,
               const  uint count_rows,
               ADDSQ  char* raw_out_map
               )
{
    uint i=0;
    uint r=0;
    //all word of map
    for(i=0;i!=page_map_rows;++i)
    {
        //search into big map
        for(r=0; r!=count_rows; ++r)
        {
            ADDSQ char*   word = (ADDSQ char*)  (raw_out_map + word_capacity*r);
            //void word
            if(!(*word))
            {
                //put word
                copy_word(word,get_row(page_map_rows_size,page_map,i)->m_word);
                break;
            }
            //compare word
            if(compare_word(word,get_row(page_map_rows_size,page_map,i)->m_word))
            {
                break;
            }
        }
        
        CPU(printf("Page %d: row %d, added %d, %s\n", page, i, r, (raw_out_map + word_capacity*r));)
    }
}

kernel void inverted_index_words
                          (//raw
                           ADDSQ  char*      raw_info,
                           ADDSQ  char*      raw_maps,
                           ADDSQ  char*      raw_out_map,
                           //value
                           ADDSQ  uint*      raw_out_info)
{
    //get global id
    unsigned int g_id     = get_global_id(0);
    //get map
    ADDSQ MapsInfo*  maps_info = (ADDSQ MapsInfo*)(raw_info + sizeof(MapsInfo)*g_id);
    ADDSQ char*      page_map  = (ADDSQ char*)    (raw_maps + maps_info->m_offset);

    CPU(
        printf("id %u, size row %u, n rows %u, offset %u, page %u\n", 
                g_id,
                maps_info->m_row,
                maps_info->m_size,
                maps_info->m_offset,
                maps_info->m_page
                );
    )
    //add map to big map
    add_a_map(//input map
              maps_info->m_size,
              maps_info->m_row,
              page_map,
              //ouput          //n_maps
              raw_out_info[1], //word_capacity,
              raw_out_info[2], //cout_rows,
              raw_out_map
              );
    
}