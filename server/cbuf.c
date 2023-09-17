#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "cbuf.h"


BUFF_TYPE *head;
BUFF_TYPE buff[BUFF_SIZE];

void test(){
    BUFF_TYPE i;
    i = 10;
    printf("%d\n", i);
}

void cbuf_set_pos(int pos){
    printf("Head = %X\n", head);
    if (pos < BUFF_SIZE){
        head = buff + pos;
        printf("Head = %X\n", head);
    }
}

void cbuf_init(){
    head = buff;
    memset (buff, 0, BUFF_SIZE);
}

void cbuf_add(BUFF_TYPE data){
    *head = data;
    
    head++;
    if(head == buff + BUFF_SIZE){
        head = buff;
    }
}

void cbuf_print_all(){
    int i;
    for (i=0; i<BUFF_SIZE; i++){
        printf("%d\n",*(buff+i));
    }
}

void cbuf_print_last(){
    printf("%d\n",*(head-1));
}

void cbuf_copy(BUFF_TYPE *dest, unsigned long start_pos){
    uint32_t count = 0;
    uint32_t pos;

    uint32_t no_of_bytes_1;
    uint32_t no_of_bytes_2;

    BUFF_TYPE *start_addr;

    start_addr = buff + start_pos;
    no_of_bytes_1 = BUFF_SIZE - start_pos;
    no_of_bytes_2 = BUFF_SIZE - no_of_bytes_1;

    printf("no_of_byte_1 = %lu, no_of_byte_2 = %lu\n", no_of_bytes_1, no_of_bytes_2);
    printf("Start Pos = %d\n", start_pos);

    memcpy(dest, start_addr, no_of_bytes_1 * sizeof(BUFF_TYPE));
    memcpy(dest + no_of_bytes_1, buff , no_of_bytes_2 * sizeof(BUFF_TYPE));
    
}

unsigned long cbuf_getpos(){
    return head - buff;
}
