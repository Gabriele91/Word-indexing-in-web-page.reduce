typedef struct __attribute__((packed)) __Row
{
    unsigned short m_count;
    char           m_word[0];
} Row ;

/* ------------------------------------------------------------------ */
inline ADDSQ Row* get_row( unsigned int row_size,
                          ADDSQ char*  raw_rows,
                          unsigned int i )
{
    return (ADDSQ Row*) &(raw_rows[row_size*i]);
}
/* ------------------------------------------------------------------ */
