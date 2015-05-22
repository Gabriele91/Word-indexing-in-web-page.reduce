
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
               const uint    page_map_rows,
               const uint    page_map_rows_size,
               ADDSQ char*   page_map,
               //input words
               const  uint   word_capacity,
               const  uint   count_rows,
               const  uint   page,
               ADDSQ  char*  words_map,
               //ouput map
               ADDSQ  ushort* out_value
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
            ADDSQ char*   word   = (ADDSQ char*)   (words_map + word_capacity*r);
            ADDSQ Row*    row    = get_row(page_map_rows_size, page_map, i);
            //compare word
            if (compare_word(word, row->m_word))
            {
                out_value[r + page*count_rows] = row->m_count;
                break;
            }
        }
    }
}

kernel void inverted_index_pages
                          (//raw
                           ADDSQ  char*        raw_info,
                           ADDSQ  char*        raw_maps,
                           ADDSQ  char*        words_map,
                           ADDSQ  ushort*      out_map,
                           //value 
                           read_only  const  uint      n_maps,
                           read_only  const  uint      start_map,
                           read_only  const  uint      word_capacity,
                           read_only  const  uint      cout_rows)
{
    //get global id
    unsigned int g_id     = get_global_id(0);
    //get map
    ADDSQ MapsInfo*  maps_info = (ADDSQ MapsInfo*)(raw_info + sizeof(MapsInfo)*g_id);
    ADDSQ Row*       page_map  = (ADDSQ Row*)      (raw_maps + maps_info->m_offset);

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
              //input words
              word_capacity,
              cout_rows,
              maps_info->m_page - start_map,
              words_map,
              //ouput
              out_map
              );
    
}