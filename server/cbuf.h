#ifndef __CBUF_H__
#define __CBUF_H__


    #define BUFF_SIZE 1000

    typedef uint32_t BUFF_TYPE;
    //typedef uint8_t BUFF_TYPE;

    void test();
    void cbuf_set_pos(int pos);

    void cbuf_init();
    void cbuf_add(BUFF_TYPE data);
    void cbuf_print_all();
    void cbuf_print_last();
    void cbuf_copy(BUFF_TYPE *dest, unsigned long start_pos);
    unsigned long cbuf_getpos();

#endif
