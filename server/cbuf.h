#ifndef __CBUF_H__
#define __CBUF_H__


    #define BUFF_SIZE 5000
    #define RECORD_ITEM 3

//    typedef uint32_t BUFF_TYPE;
//    typedef uint32_t RECORD_TYPE;
//    typedef RECORD_TYPE* DATA_TYPE;

    typedef struct {
        uint32_t    timestamp;
        uint16_t    ch1;
        uint16_t    ch3;
    } sample_t;


    void cbuf_test();
    void cbuf_set_pos(int pos);
    int cbuf_get_pos();
    
    void cbuf_init();
//    void cbuf_add(BUFF_TYPE data);
    void cbuf_add(uint32_t time, uint16_t ch1, uint16_t ch3);
    void cbuf_print_all();
    void cbuf_print_last();
//    void cbuf_copy(BUFF_TYPE *dest, unsigned long start_pos);
    void cbuf_copy(char *dest, unsigned long start_pos);
    unsigned long cbuf_getpos();

#endif
