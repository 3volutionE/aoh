#include <fcntl.h>				//Needed for SPI port
#include <sys/ioctl.h>			//Needed for SPI port
#include <linux/spi/spidev.h>	//Needed for SPI port
#include <unistd.h>			//Needed for SPI port

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

char adc_buff[SAMPLE_COUNT * sizeof(sample_t)];

unsigned long trig_pos;
int read_flag = 0;
int trig_flag = 0;
int done_flag = 0;
uint32_t alert_flag = 0;


struct timespec ts;

struct timespec res1;
struct timespec res2;





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
    //uint32_t time = 0x12345678;
    printf("----- Thread read adc created\n");
    printf("----- Read until there is a trigger\n");
    while(!read_flag);      // Wait until read_flag is set to start reading
    while(read_flag){
        // Get time
        clock_gettime(CLOCK_REALTIME, &ts);
        // Get data
        adc7380_read_adc();

        //printf("%d : vol = %f V\n",(RxData[0] << 8) | RxData[1], ((RxData[0] << 8) | RxData[1])*2*3.3/65536);
        //printf("%d %d %d %d\n",RxData[0], RxData[1], RxData[2] ,RxData[3]);
        //printf("time %lu\n", ts.tv_nsec);
        
        cbuf_add(ts.tv_nsec, (RxData[0] << 8) | RxData[1], (RxData[2] << 8) | RxData[3] );
        //cbuf_add(time, (RxData[0] << 8) | RxData[1], (RxData[2] << 8) | RxData[3] );

        if(trig_flag){
            sample_count++;
            if(sample_count >= (SAMPLE_COUNT >> 1)){   // equiv to : BUFF_SIZE / 2
                done_flag = 1;
                break;
            }
        }
    }
    printf("----- Thread read adc exited\n");
}

int main(){

    pthread_t th_readadc;
    char sock_buff[100];
    printf("******************************\n");
    printf("***    ADC INTERFACE APP   ***\n");
    printf("******************************\n");

    // Initial hardware
    printf("-- Initialize Hardware --\n");
    setup_spi();
    setup_gpio();
    cbuf_init();
    setup_adc();
    sock_init();            // Create socket to send data to Python program
    printf("-- Done Initialize Hardware --\n");
    
    while(1){ 
        
        // Wait until receive the capture command
        printf("-- Wait for incoming capture command --\n");
        while(socket_receive(sock_buff) < 0);
        printf("--- Received Command = %s\n", sock_buff);
        if(strcmp((const char *)sock_buff, "capture") != 0) continue;

        // Reset flag
        read_flag = 0;
        trig_flag = 0;
        done_flag = 0;
        alert_flag = 0;

        // Create ADC read thread
        printf("--- Create thread to read data from ADC\n");
        pthread_create(&th_readadc, NULL, read_adc, NULL);
        read_flag = 1;

        // Wait until there is an alert signal, then assert trig_flag        
        do{            
            alert_flag = (gpio_read(ADC_ALARM_GPIO_IN));
        }while(alert_flag > 0);
        trig_flag = 1;

        // Wait until done_flag is set
        while(!done_flag); 
        printf("--- Done flag is set, done read adc\n");
        adc7380_read_reg(AD7380_REG_ALERT);
        printf ("--- Alert Reg = 0x%02X%02X\n" ,RxData[0], RxData[1]);

        // Copy data from circular buffer to ADC buffer
        printf("--- Copy data to buffer\n");
        trig_pos = cbuf_get_pos();   
        cbuf_copy(adc_buff, trig_pos);
        //printf("Send result to PY with %d bytes\n",BUFF_SIZE * sizeof(adc_buff));        
        
        /*
        for(int i=0; i<100; i++){
            printf("Data [%d] : ", i);
            for(int j=0; j<8; j++){
                printf("%x", *(adc_buff+(i*8)+j));
            }
            printf("\n");
        }
        */
        // Send adc data to client
        socket_send((unsigned char*)adc_buff, sizeof(adc_buff));
    }
    
    SpiClosePort(0);
    printf("Exit main program\n");
    return 0;
    
    // Trap state
    while(1);
}


void setup_gpio()
{
    printf ("--- Setup GPIO\n");
    gpio_mem_map();    
    gpio_set_dir(GPIO_DIR_IN, ADC_ALARM_GPIO_IN);

}

void setup_adc()
{
    uint32_t regval;

    printf ("--- Setup ADC Port\n");
    // Reset ADC
    printf ("---- Reset ADC chip\n");
    adc7380_write_reg(AD7380_REG_CONFIG2, 0x00FF);          // Hard reset
    sleep(1);                                               // Wait 10ms

    printf ("---- Set Config 2 Reg\n");
    adc7380_write_reg(AD7380_REG_CONFIG2, 0x0000);          // 2-wire output mode    
    //regval = adc7380_read_reg(AD7380_REG_CONFIG2);  //, rx_buf);    
    //printf ("Configuration 2 Reg = 0x%02X%02X\n" ,RxData[0], RxData[1]);
    
    // --- Set Alert function --- //    
    printf ("---- Set Alert High Threshold\n");
    // Set Alert threshold, Threshold (Vth), set register value = ((Vth * 65536 / (2 * Vref)) >> 4)
    regval = ((uint32_t)((V_TRIGGER_THRESHOLD * 65536) / (2 * V_REF))) >> 4;
    adc7380_write_reg(AD7380_REG_ALERT_HIGH_THRESHOLD, regval);         // Set alert to about 2.5V
    //regval = adc7380_read_reg(AD7380_REG_ALERT_HIGH_THRESHOLD);         //, rx_buf);
    //printf ("Alert High Reg = 0x%02X%02X\n" ,RxData[0], RxData[1]);
    printf("----- Trigger Point = %.2f V, [Reg HEX value : 0x%04X\n",V_TRIGGER_THRESHOLD, regval);

    // Enable Alert function
    printf ("---- Set Config 1 Reg\n");
    adc7380_write_reg(AD7380_REG_CONFIG1, 0x0008);         // Enable alert function
    //regval = adc7380_read_reg(AD7380_REG_CONFIG1);  //, rx_buf);
    //printf ("Configuration 1 Reg = 0x%02X%02X\n" ,RxData[0], RxData[1]);
}

void setup_spi(){
    int retval;
    printf ("--- Setup SPI Port\n");
    retval = spi_open(&spi_fd);
    
    spi.tx_buf = (unsigned long)TxData;		//transmit from "data"
	spi.rx_buf = (unsigned long)RxData;		//receive into "data"
	spi.len = ADC_DATA_LEN;
	spi.delay_usecs = 0;
	spi.speed_hz = SPI_SPEED;
	spi.bits_per_word = SPI_BIT_PER_WORD;
	spi.cs_change = 0;						//0=Set CS high after a transfer, 1=leave CS set low
    
}