
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


static void add_a_map(
               //input map
               const uint    page_map_rows,
               const uint    page_map_rows_size,
               ADDSQ char*   page_map,
               //input words
               const  uint   n_pages,
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
        ADDSQ Row*    row    = get_row(page_map_rows_size, page_map, i);
        //search into big map
        for(r=0; r!=count_rows; ++r)
        {
            ADDSQ char*   word   = (ADDSQ char*)   (words_map + word_capacity*r);
            //compare word
            if (compare_word(word, row->m_word))
            {
                out_value[r*n_pages + page] = row->m_count;
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
                           //value info
                           ADDSQ  uint*        raw_out_info)
{
    //get global id
    unsigned int g_id     = get_global_id(0);
    //get map
    ADDSQ MapsInfo*  maps_info = (ADDSQ MapsInfo*)(raw_info + sizeof(MapsInfo)*g_id);
    ADDSQ char*      page_map  = (ADDSQ char*)    (raw_maps + maps_info->m_offset);

    CPU(
        printf("id %u, size row %u, n rows %u, offset %u, page %u ||  "
               "n_maps: %u, start_map: %u, word_capacity: %u, cout_rows: %u\n",
                g_id,
                maps_info->m_row,
                maps_info->m_size,
                maps_info->m_offset,
                maps_info->m_page,
                raw_out_info[0], //n_maps,
                raw_out_info[1], //start_map,
                raw_out_info[2], //word_capacity,
                raw_out_info[3]  //cout_rows
                );
    )
    //filter
    if(raw_out_info[3])
    {
        //add map to big map
        add_a_map(//input map
                  maps_info->m_size,
                  maps_info->m_row,
                  page_map,
                  //input words
                  raw_out_info[0],                     //n_maps
                  raw_out_info[2],                     //word_capacity
                  raw_out_info[3],                     //cout_rows
                  maps_info->m_page - raw_out_info[1], //start_map
                  words_map,
                  //ouput
                  out_map
                  );
    }
}