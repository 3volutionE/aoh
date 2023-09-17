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
#include <wiringPi.h>

#include "cbuf.h"
#include "socket.h"


int spi_cs0_fd;				//file descriptor for the SPI device
int spi_cs1_fd;				//file descriptor for the SPI device
unsigned char spi_mode;
unsigned char spi_bitsPerWord;
unsigned int spi_speed;




//***********************************
//***********************************
//********** SPI OPEN PORT **********
//***********************************
//***********************************
//spi_device	0=CS0, 1=CS1
int SpiOpenPort (int spi_device)
{
	int status_value = -1;
    int *spi_cs_fd;


    //----- SET SPI MODE -----
    //SPI_MODE_0 (0,0) 	CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge
    //SPI_MODE_1 (0,1) 	CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on falling edge, output data (change) on rising edge
    //SPI_MODE_2 (1,0) 	CPOL = 1, CPHA = 0, Clock idle high, data is clocked in on falling edge, output data (change) on rising edge
    //SPI_MODE_3 (1,1) 	CPOL = 1, CPHA = 1, Clock idle high, data is clocked in on rising, edge output data (change) on falling edge
    spi_mode = SPI_MODE_3;
    
    //----- SET BITS PER WORD -----
    spi_bitsPerWord = 8;
    
    //----- SET SPI BUS SPEED -----
    spi_speed = 40000000;		//40000000 = 40MHz (1uS per bit) 


    if (spi_device)
    	spi_cs_fd = &spi_cs1_fd;
    else
    	spi_cs_fd = &spi_cs0_fd;


    if (spi_device)
    	*spi_cs_fd = open("/dev/spidev0.1", O_RDWR);
    else
    	*spi_cs_fd = open("/dev/spidev0.0", O_RDWR);

    if (*spi_cs_fd < 0)
    {
        perror("Error - Could not open SPI device");
        exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_MODE, &spi_mode);
    if(status_value < 0)
    {
        perror("Could not set SPIMode (WR)...ioctl fail");
        exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_MODE, &spi_mode);
    if(status_value < 0)
    {
      perror("Could not set SPIMode (RD)...ioctl fail");
      exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bitsPerWord);
    if(status_value < 0)
    {
      perror("Could not set SPI bitsPerWord (WR)...ioctl fail");
      exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bitsPerWord);
    if(status_value < 0)
    {
      perror("Could not set SPI bitsPerWord(RD)...ioctl fail");
      exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
    if(status_value < 0)
    {
      perror("Could not set SPI speed (WR)...ioctl fail");
      exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
    if(status_value < 0)
    {
      perror("Could not set SPI speed (RD)...ioctl fail");
      exit(1);
    }
    return(status_value);
}



//************************************
//************************************
//********** SPI CLOSE PORT **********
//************************************
//************************************
int SpiClosePort (int spi_device)
{
	int status_value = -1;
    int *spi_cs_fd;

    if (spi_device)
    	spi_cs_fd = &spi_cs1_fd;
    else
    	spi_cs_fd = &spi_cs0_fd;


    status_value = close(*spi_cs_fd);
    if(status_value < 0)
    {
    	perror("Error - Could not close SPI device");
    	exit(1);
    }
    return(status_value);
}



//*******************************************
//*******************************************
//********** SPI WRITE & READ DATA **********
//*******************************************
//*******************************************
//SpiDevice		0 or 1
//TxData and RxData can be the same buffer (read of each byte occurs before write)
//Length		Max 511 (a C SPI limitation it seems)
//LeaveCsLow	1=Do not return CS high at end of transfer (you will be making a further call to transfer more data), 0=Set CS high when done
int SpiWriteAndRead (int SpiDevice, unsigned char *TxData, unsigned char *RxData, int Length, int LeaveCsLow)
{
	struct spi_ioc_transfer spi;
	int i = 0;
	int retVal = -1;
	int spi_cs_fd;

	if (SpiDevice)
		spi_cs_fd = spi_cs1_fd;
	else
		spi_cs_fd = spi_cs0_fd;

	spi.tx_buf = (unsigned long)TxData;		//transmit from "data"
	spi.rx_buf = (unsigned long)RxData;		//receive into "data"
	spi.len = Length;
	spi.delay_usecs = 0;
	spi.speed_hz = spi_speed;
	spi.bits_per_word = spi_bitsPerWord;
	spi.cs_change = LeaveCsLow;						//0=Set CS high after a transfer, 1=leave CS set low

	retVal = ioctl(spi_cs_fd, SPI_IOC_MESSAGE(1), &spi);

	if(retVal < 0)
	{
		perror("Error - Problem transmitting spi data..ioctl");
		exit(1);
	}

	return retVal;
}



int adc7380_write_reg(uint8_t reg_addr, uint16_t reg_val){
    
    struct spi_ioc_transfer spi;
    int spi_cs_fd;

    unsigned char TxData[2];
    unsigned char RxData[2];
    int retVal;

    spi_cs_fd = spi_cs0_fd;

    spi.tx_buf = (unsigned long)TxData;		//transmit from "data"
	spi.rx_buf = (unsigned long)RxData;		//receive into "data"
	spi.len = 2;
	spi.delay_usecs = 0;
	spi.speed_hz = spi_speed;
	spi.bits_per_word = 8;
	spi.cs_change = 0;						//0=Set CS high after a transfer, 1=leave CS set low

    //if(self.__if_spi()) :
    //    data_send = [(((reg_addr | 0x08) << 4) | (reg_val >> 8)) , (reg_val & 0x00FF)]
    TxData[0] = (((reg_addr | 0x08) << 4) | (reg_val >> 8));
    TxData[1] = (reg_val & 0x00ff);

    printf("Write 0x%02X, 0x%02X\n",TxData[0], TxData[1]);
    retVal = ioctl(spi_cs_fd, SPI_IOC_MESSAGE(1), &spi);
    //    self.Spi.writebytes(data_send)
    //    return 0
    //else:
        return retVal;
}

int adc7380_read_reg(uint8_t reg_addr, unsigned char *reg_val){
    
    struct spi_ioc_transfer spi;
    int spi_cs_fd;

    unsigned char TxData[2];
    unsigned char RxData[2];
    int retVal;

    spi_cs_fd = spi_cs0_fd;

    spi.tx_buf = (unsigned long)TxData;		//transmit from "data"
	spi.rx_buf = (unsigned long)RxData;		//receive into "data"
	spi.len = 2;
	spi.delay_usecs = 0;
	spi.speed_hz = spi_speed;
	spi.bits_per_word = 8;
	spi.cs_change = 0;						//0=Set CS high after a transfer, 1=leave CS set low

    //if(self.__if_spi()) :
    //    data_send = [(((reg_addr | 0x08) << 4) | (reg_val >> 8)) , (reg_val & 0x00FF)]
    //TxData[0] = (((reg_addr & 0x07) << 4) | (*reg_val >> 8));
    //TxData[0] = ((reg_addr & 0x07) << 4) | (*reg_val));
    TxData[0] = ((reg_addr & 0x07) << 4);
    TxData[1] = 0;
    //TxData[1] = *(reg_val+1);

    retVal = ioctl(spi_cs_fd, SPI_IOC_MESSAGE(1), &spi);

    TxData[0] = 0;
    TxData[1] = 0;

    retVal = ioctl(spi_cs_fd, SPI_IOC_MESSAGE(1), &spi);
    *(reg_val) = RxData[0];
    *(reg_val+1) = RxData[1];

    //    self.Spi.writebytes(data_send)
    //    return 0
    //else:
    return retVal;
}



struct spi_ioc_transfer spi;
int spi_fd;
unsigned char RxData[8];
unsigned char TxData[8];

void spi_config(){

    //unsigned char TxData[2];
    //unsigned char RxData[2];
    //int retVal;

    spi_fd = spi_cs0_fd;

    spi.tx_buf = (unsigned long)TxData;		//transmit from "data"
	spi.rx_buf = (unsigned long)RxData;		//receive into "data"
	spi.len = 4;                // 2 wire mode
	spi.delay_usecs = 0;
	spi.speed_hz = spi_speed;
	spi.bits_per_word = 8;
	spi.cs_change = 0;						//0=Set CS high after a transfer, 1=leave CS set low
}

int adc7380_read_adc(){
    int retVal;


    retVal = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi);
    
    //    self.Spi.writebytes(data_send)
    //    return 0
    //else:

    


    return retVal;
}

int timer_flag = 0;

static void *timer(void *arg){
    while(1){
        usleep(10);
        timer_flag = 1;
    }
}


//uint32_t sample_count = 0;
int read_flag = 0;
int trig_flag = 0;
int done_flag = 0;

static void *read_adc(void *arg){
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

struct timespec res1;
struct timespec res2;


BUFF_TYPE adc_buff[BUFF_SIZE];
unsigned long trig_pos;


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
    SpiOpenPort(0);
    spi_config();
    // Initialize circular buffer
    cbuf_init();

    goto TEST;

    while(1){
        //scanf("%s", &keyin);
        printf("Press <enter> to start capture\n");
        cin = getchar();
        if(cin == 'e') break;
        
        // Reset flag
        read_flag = 0;
        trig_flag = 0;
        done_flag = 0;

        // Create ADC read thread
        pthread_create(&th_readadc, NULL, read_adc, NULL);
    

        // Following is the temp code for testing.
        sleep(1);
        printf("After 1s, start reading\n");
        read_flag = 1;

        sleep(5);
        printf("After 5s, simulate trig signal\n");
        trig_flag = 1;

        while(!done_flag); 
        printf("Done flag is set, done read adc\n");
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
    regval = adc7380_read_reg(0x05, rx_buf);
    printf ("Configuration 2 Reg = 0x%02X%02X\n" ,rx_buf[0], rx_buf[1]);

    //return 0;

    //regval = adc.read_reg(0x02);
    //printf ("Configuration 2 Reg = 0x%02X%02X" ,rx_buf[0], rx_buf[1]);


    adc7380_write_reg(0x05, 0x0500);         // Set alert to about 2.5V
    regval = adc7380_read_reg(0x05, rx_buf);
    printf ("Alert Reg = 0x%02X%02X\n" ,rx_buf[0], rx_buf[1]);

    //return 0;

    adc7380_write_reg(0x01, 0x0008);         // Enable alert function
    regval = adc7380_read_reg(0x01, rx_buf);
    printf ("Configuration 1 Reg = 0x%02X%02X\n" ,rx_buf[0], rx_buf[1]);

    adc7380_write_reg(0x02, 0x0100);         // Set 2-Wire output mode, SDOA will output channel A and C, SDOB will outout channel B and D. We connect to only SDOA, so signal in must use channel A and C
    regval = adc7380_read_reg(0x02, rx_buf);
    printf ("Configuration 2 Reg = 0x%02X%02X\n" ,rx_buf[0], rx_buf[1]);



    
        
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