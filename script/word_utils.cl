
inline bool is_end_char(char word)
{
    return (word == ' ') || (word == '\0');
}

inline void copy_word(ADDSQ char* dest_word,
                      ADDSQ char* from_word)
{
    //init index
    unsigned int i = 0;
    //copy
    while ( !is_end_char(from_word[i]) )
    {
        dest_word[i] = from_word[i];
        ++i;
    }
    //end word
    dest_word[i] = '\0';
}

inline unsigned int word_len(ADDSQ char* word_1)
{
    //init index
    unsigned int i = 0;
    //count
    while ( !is_end_char(*word_1)  )
    {
        word_1 += utf8_next_char_count(*word_1);
        ++i;
    }
    //return len
    return i;
}

inline bool compare_word(ADDSQ char* word_1,
                         ADDSQ char* word_2)
{
    //init index
    unsigned int i = 0;
    //compare
    while ( !is_end_char(word_1[i]) && !is_end_char(word_2[i]) )
    {
        if(word_1[i]!=word_2[i]) return false;
        ++i;
    }
    //true if the words are at the end
    bool ret_value = is_end_char(word_1[i]) && is_end_char(word_2[i]);
    //return value
    return ret_value;
}

inline bool compare_word_no_case_sensitive(ADDSQ char* word_1,
                                           ADDSQ char* word_2)
{
    //init index
    unsigned int i = 0;
    //compare
    while ( !is_end_char(word_1[i]) && !is_end_char(word_2[i]) )
    {
        if(utf8_to_lower_ushort(word_1+i) != utf8_to_lower_ushort(word_2+i))
        {
            return false;
        }
        //if are equals the next ids are equals
        i+=utf8_next_char_count(word_1[i]);
    }
    //true if the words are at the end
    bool ret_value = is_end_char(word_1[i]) && is_end_char(word_2[i]);
    //return value
    return ret_value;
}
