
//#define TARGET_CPU
#if /* defined(TARGET_CPU) &&*/ defined(DEBUG)
    void __print_cstr(constant char* text)
    {
        for (unsigned int i = 0; text[i]; ++i) printf("%c", text[i]);
    }
    void __print_str(ADDSQ char* text)
    {
        for (unsigned int i = 0; text[i]; ++i) printf("%c", text[i]);
    }
    void __print_uint(unsigned int i)
    {
        printf("%d", i);
    }
    void __print_word(ADDSQ char* text)
    {
        for (unsigned int i = 0; text[i]!='\0' && text[i]!=' ' ; ++i) printf("%c", text[i]);
    }
    void __print_row(ADDSQ Row* row)
    {
        printf("[ %d, ",row->m_count);
        __print_word(row->m_word);
        printf(" ](%p); ",row);
    }
    #define CPU(x) x
    #define print_cstr(x)  __print_cstr(x)
    #define print_str(x)  __print_str(x)
    #define print_word(x) __print_word(x)
    #define print_uint(x) __print_uint(x)
    #define print_row(x)  __print_row(x)
    #define print_endl()  printf("\n")
#else
    #define CPU(x)
    #define print_cstr(x)
    #define print_str(x)
    #define print_word(x)
    #define print_row(x)
    #define print_endl()
#endif