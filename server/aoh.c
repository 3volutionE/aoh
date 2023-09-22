#include <fcntl.h>				//Needed for SPI port
#include <sys/ioctl.h>			//Needed for SPI port
#include <linux/spi/spidev.h>	//Needed for SPI port
#include <unistd.h>			//Needed for SPI port
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <iostream>
#include <unistd.h>
//#include <cstring>

#include <stdint.h>
#include <pthread.h>
#include <time.h>

#include "adc.h"
#include "spi.h"
#include "cbuf.h"
#include "socket.h"
#include "gpio.h"
#include "setting.h"





int spi_fd;
struct spi_ioc_transfer spi;


unsigned char RxData[10];
unsigned char TxData[10];


struct timespec res1;
struct timespec res2;


BUFF_TYPE adc_buff[BUFF_SIZE];
unsigned long trig_pos;
//uint32_t sample_count = 0;
int read_flag = 0;
int trig_flag = 0;
int done_flag = 0;
//uint alarm_flag = 0;
uint32_t alert_flag = 0;





// Function Prototype //
void setup_gpio();
void setup_adc();
void setup_spi();



int adc7380_write_reg(uint8_t reg_addr, uint16_t reg_val){
    int retval;

    spi.len = ADC_REG_LEN;

    TxData[0] = (((reg_addr | 0x08) << 4) | (reg_val >> 8));
    TxData[1] = (reg_val & 0x00ff);
    retval = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi);
    
    spi.len = ADC_DATA_LEN;
    return retval;
}

int adc7380_read_reg(uint8_t reg_addr)//, unsigned char *reg_val){
{
    int retval;

    //printf("txbuf = %d\n", spi.tx_buf);
    spi.len = ADC_REG_LEN;
    TxData[0] = ((reg_addr & 0x07) << 4);
    TxData[1] = 0;
    retval = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi);

    TxData[0] = 0;
    TxData[1] = 0;
    retval = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi);
    //printf("txbuf = %d\n", spi.tx_buf);
    spi.len = ADC_DATA_LEN;
    return retval;
}


static inline void adc7380_read_adc(){
    //int retVal;
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi);
    //return retVal;
}




static void *read_adc(void *arg){
    uint32_t sample_count = 0;
    printf("Thread read adc created\n");
    while(!read_flag);      // Wait until read_flag is set to start reading
    while(read_flag){
        adc7380_read_adc();
        printf("%04X : vol = %f V\n",(RxData[0] << 8) | RxData[1], ((RxData[0] << 8) | RxData[1])*2*3.3/65536);
        cbuf_add((RxData[0] << 8) | RxData[1]);
        if(trig_flag){
            sample_count++;
            if(sample_count == 5000){
                done_flag = 1;
                break;
            }
        }
    }
    printf("Thread read adc exited\n");
}

static void *wait_for_alarm(void *arg){
    uint32_t sample_count = 0;
    printf("Thread read adc created\n");
    while(!read_flag);      // Wait until read_flag is set to start reading
    while(read_flag){
        adc7380_read_adc();
        printf("%d\n",(RxData[0] << 8) | RxData[1]);
        cbuf_add((RxData[0] << 8) | RxData[1]);
        if(trig_flag){
            sample_count++;
            if(sample_count == 5000){
                done_flag = 1;
                break;
            }
        }
    }
    printf("Thread read adc exited\n");
}



int main(){
    int i;
    long t_next;
    long l1, l2;
    uint16_t regval;
    unsigned char tx_buf[100];
    unsigned char rx_buf[100];

    pthread_t th_timer;
    pthread_t th_readadc;


    int inp;
    char keyin[100];
    char cin;

    //cbuf_set_pos(5);

    //for (i=1; i<=20; i++)
    //    cbuf_add(i);

    //cbuf_print_all();

    //return 0;


    // Setup SPI port
    // Initialize circular buffer
    setup_spi();
    setup_gpio();
    cbuf_init();
    setup_adc();

//    goto TEST;




    while(1){
        //scanf("%s", &keyin);
        printf("Press <enter> to start capture\n");
        cin = getchar();
        if(cin == 'e') break;
        
        // Reset flag
        read_flag = 0;
        trig_flag = 0;
        done_flag = 0;
        alert_flag = 0;

        // Create ADC read thread
        pthread_create(&th_readadc, NULL, read_adc, NULL);
    

        // Following is the temp code for testing.
        sleep(1);
        printf("After 1s, start reading\n");
        read_flag = 1;

        //while(1);
        //sleep(5);
        //printf("After 5s, simulate trig signal\n");
        //trig_flag = 1;
        do{            
            alert_flag = (gpio_read(ADC_ALARM_GPIO_IN));
            //printf("alert = %d\n",alert_flag);
        }while(alert_flag > 0);            
        trig_flag = 1;
        

        // Wait until done_flag is set.
        while(!done_flag); 
        printf("Done flag is set, done read adc\n");
        adc7380_read_reg(AD7380_REG_ALERT);
        printf ("Alert Reg = 0x%02X%02X\n" ,RxData[0], RxData[1]);

    }

    printf("Exit main program\n");
    return 0;

    // Setup
    //adc7380_write_reg(0x02, 0x00FF);         // Hard reset ADC
    //time.sleep(1)                       // Wait 2 second for the reset process
    
    //adc7380_write_reg(0x02, 0x0100);         // Set 2-Wire output mode, SDOA will output channel A and C, SDOB will outout channel B and D. We connect to only SDOA, so signal in must use channel A and C
    //time.sleep(1)                       // Wait 2 second for the reset process
    
    
    while(1);
    return 0;

    
TEST:



    while(1){
        alert_flag = (gpio_read(ADC_ALARM_GPIO_IN));
        printf("alert = %d\n",alert_flag);
    }

    sock_init();            // Create socket to send data to Python program
    if(socket_wait_client() < 0) {
        // Error, abort program
    }


    printf("Socket Create\n");




    cbuf_set_pos(5);
    for (i=1; i<=100; i++)
        cbuf_add(i);


    cbuf_print_all();
   
    trig_pos = 0;
    cbuf_copy(adc_buff, trig_pos);
    
    for (i=0; i<BUFF_SIZE; i++){
        printf("%d\n",*(adc_buff+i));
    }

    return 0;




    return 0;
    



    
        
    //pthread_create(&th_timer, NULL, timer, NULL);
    
    while(1){
        //cbuf_print_all();
        usleep(1000);
    }
    
    return 0;
    clock_gettime(CLOCK_REALTIME,&res1);
    clock_gettime(CLOCK_REALTIME,&res2);
    
    printf("Difference: %lu\n", res2.tv_nsec - res1.tv_nsec);
    while(1){
        clock_gettime(CLOCK_REALTIME,&res1);
        //printf("%lu\n", res1.tv_nsec);
        t_next = res1.tv_nsec + 1000;
        //usleep(1000);
        adc7380_read_adc();
        do {
            clock_gettime(CLOCK_REALTIME,&res2);
            //printf("%lu, %lu, %lu\n", res1.tv_nsec, t_next, res2.tv_nsec);
        }
        //while(res2.tv_nsec - res1.tv_nsec < 1000);
        //while(res2.tv_nsec < t_next);
        while( (res2.tv_nsec < res1.tv_nsec ? res2.tv_nsec + 1000000000 : res2.tv_nsec) - res1.tv_nsec < 1000);
            //res1.tv_nsec + 1000 > res2.tv_nsec);
    }

//    SpiWriteAndRead(0, tx_buf, rx_buf, 4, 0);
 //  printf("Data = %d, %d, %d, %d\n", rx_buf[0], rx_buf[1], rx_buf[2], rx_buf[3]);
    SpiClosePort(0);  
    return 0;
}


void setup_gpio()
{
    gpio_mem_map();    
    gpio_set_dir(GPIO_DIR_IN, ADC_ALARM_GPIO_IN);

}

void setup_adc(){
    uint32_t regval;
    //char rx_buf[10];
    printf ("Setup ADC\n");
    // Reset ADC
    adc7380_write_reg(AD7380_REG_CONFIG2, 0x00FF);          // Hard reset
    sleep(1);                                               // Wait 10ms
    adc7380_write_reg(AD7380_REG_CONFIG2, 0x0000);          // 2-wire output mode
    regval = adc7380_read_reg(AD7380_REG_CONFIG2);  //, rx_buf);
    printf ("Configuration 2 Reg = 0x%02X%02X\n" ,RxData[0], RxData[1]);
    
    // --- Set Alert function --- //    
    // Set Alert threshold, Threshold (Vth), set register value = ((Vth * 65536 / (2 * Vref)) >> 4)
    regval = ((uint32_t)((V_TRIGGER_THRESHOLD * 65536) / (2 * V_REF))) >> 4;
    printf ("Calaulated threshold value = 0x%04X\n",regval);
    adc7380_write_reg(AD7380_REG_ALERT_HIGH_THRESHOLD, regval);         // Set alert to about 2.5V
    regval = adc7380_read_reg(AD7380_REG_ALERT_HIGH_THRESHOLD);         //, rx_buf);
    printf ("Alert High Reg = 0x%02X%02X\n" ,RxData[0], RxData[1]);

    // Enable Alert function
    adc7380_write_reg(AD7380_REG_CONFIG1, 0x0008);         // Enable alert function
    regval = adc7380_read_reg(AD7380_REG_CONFIG1);  //, rx_buf);
    printf ("Configuration 1 Reg = 0x%02X%02X\n" ,RxData[0], RxData[1]);
}

void setup_spi(){
    int retval;
    printf ("Setup SPI Port\n");
    retval = spi_open(&spi_fd);
    
    spi.tx_buf = (unsigned long)TxData;		//transmit from "data"
	spi.rx_buf = (unsigned long)RxData;		//receive into "data"
	spi.len = ADC_DATA_LEN;
	spi.delay_usecs = 0;
	spi.speed_hz = SPI_SPEED;
	spi.bits_per_word = SPI_BIT_PER_WORD;
	spi.cs_change = 0;						//0=Set CS high after a transfer, 1=leave CS set low
    
}