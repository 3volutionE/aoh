//
//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program
//  15-January-2012
//  Dom and Gert
//  Revised: 15-Feb-2013


// Access from ARM Running Linux

#define BCM2708_PERI_BASE        0x20000000
//#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
//#define GPIO_BASE                0x7E200000
#define GPIO_BASE                0x00200000


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "gpio.h"




int  mem_fd;
void *gpio_map;

// I/O access
volatile unsigned *gpio;


// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock



   #define GPIO_SET_1(n)         *((volatile unsigned*)((void *)gpio + REG_GPIO_GPSET0)) = (1 << n)
   //#define GPIO_READ_1(n)        ((*((volatile unsigned*)((void *)gpio + GPIO_GPLEV0)) & (1 << n)) > 0 ? 1 : 0)
   #define GPIO_READ_1(n)        (*((volatile unsigned*)((void *)gpio + GPIO_GPLEV0)) & (1 << n))


//
// Set up a memory regions to access GPIO
//
//void setup_io()
void gpio_set_dir(int dir, int gpio_num)
{
   if(dir == GPIO_DIR_IN){
      INP_GPIO(gpio_num);
      *((volatile unsigned*)((void *)gpio + GPIO_PUP_PDN_CNTRL_REG1)) = (1 << 18);     // Pull up
   }
   else
   {
      OUT_GPIO(gpio_num);
   }
}


void gpio_mem_map()
{
   /* open /dev/mem */
   if ((mem_fd = open(GPIO_MEMFILE, O_RDWR|O_SYNC) ) < 0) {
      printf("can't open %s \n", GPIO_MEMFILE);
      exit(-1);
   }

   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );

   close(mem_fd); //No need to keep mem_fd open after mmap

   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map);//errno also set!
      exit(-1);
   }

   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;
    int reg_val = *(gpio+0xe4); 
    printf("REg val = 0x%04X\n",reg_val);

} 

int gpio_read(int gpio_num){
   return GPIO_READ_1(gpio_num);
}

int mainxx(){
   char io;
   int reg;
    gpio_mem_map();
    //OUT_GPIO(26);
    INP_GPIO(25);
    INP_GPIO(26);
    
    //sleep(1);
    //*(gpio+REG_GPIO_GPSET0) = (1 << 26);
    //GPIO_CLR = (1 << 26);
    printf("BASE = %X\n", gpio);
    printf("PULL = %X\n", gpio+13);
    printf("PULL = %X\n", (void*)gpio + GPIO_PUP_PDN_CNTRL_REG1);
      *((volatile unsigned*)((void *)gpio + REG_GPIO_GPSET0)) = (1 << 26);


      while(1){
         io = GPIO_READ_1(26);
         //reg = *((volatile unsigned*)((void *)gpio + GPIO_GPLEV0));
         reg = *((volatile unsigned*)((void *)gpio + GPIO_GPLEV0)) & (1 << 26);
         printf("Level = %x\n", reg);
         printf("Level = %d", io);
         sleep(1);
      }
    return 0;
}
