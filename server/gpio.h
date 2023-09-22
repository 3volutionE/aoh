#ifndef __GPIO_H_
#define __GPIO_H_
    

    #define GPIO_DIR_IN             (1)
    #define GPIO_DIR_OUT            (~GPIO_DIR_IN)

    #define PAGE_SIZE               (4*1024)
    #define BLOCK_SIZE              (4*1024)

    #define GPIO_MEMFILE             "/dev/gpiomem"

    #define REG_GPIO_GPSET0             0x1C
    #define GPIO_PUP_PDN_CNTRL_REG1     0xE8
    #define GPIO_GPLEV0                 0x34
    
    
    

    void gpio_mem_map();
    void gpio_set_dir(int dir, int gpio_num);
    int gpio_read(int gpio_num);

#endif