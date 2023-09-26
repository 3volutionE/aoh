#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "cbuf.h"
#include "setting.h"

sample_t *head;
sample_t buff[SAMPLE_COUNT];

void cbuf_test()
{
    sample_t *pbuf;
    pbuf = buff;
    printf("pbuf = 0x%X\n", pbuf);
    printf("pbuf = 0x%X\n", pbuf+1);
    printf("pbuf = 0x%X\n", pbuf+2);
    printf("sizeof(head) = %d\n",sizeof(head));
    printf("sizeof(pbuf) = %d\n",sizeof(pbuf));
    printf("sizeof(sample_t) = %d\n",sizeof(sample_t));
}

void cbuf_set_pos(int pos){
    printf("Head = %X\n", head);
    if (pos < BUFF_SIZE){
        head = buff + pos;
        printf("Head = %X\n", head);
    }
}

int cbuf_get_pos(){
    return head - buff;
}

void cbuf_init(){
    head = buff;
    memset (buff, 0, sizeof(buff));
}

void cbuf_add(uint32_t time, uint16_t ch1, uint16_t ch3)
{
    
    head->timestamp = time;
    head->ch1 = ch1;
    head->ch3 = ch3;
    
    head++;
    if(head == buff + SAMPLE_COUNT){
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

void cbuf_copy(char *dest, unsigned long start_pos)
{

    sample_t *start_addr;
    uint32_t no_of_bytes_1;
    uint32_t no_of_bytes_2;

    start_addr = buff + start_pos;
    no_of_bytes_1 = SAMPLE_COUNT - start_pos;
    no_of_bytes_2 = SAMPLE_COUNT - no_of_bytes_1;

    printf("----- Start Pos = %d\n", start_pos);
    printf("----- no_of_byte_1 = %lu, no_of_byte_2 = %lu\n", no_of_bytes_1, no_of_bytes_2);
    
    memcpy(dest, start_addr, no_of_bytes_1 * sizeof(sample_t));
    memcpy(dest + (no_of_bytes_1 * sizeof(sample_t)), buff , no_of_bytes_2 * sizeof(sample_t));
}


unsigned long cbuf_getpos(){
    return head - buff;
}
