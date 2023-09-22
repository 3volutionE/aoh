#include <stdio.h>
#include <fcntl.h>				//Needed for SPI port
#include <sys/ioctl.h>			//Needed for SPI port
#include <linux/spi/spidev.h>	//Needed for SPI port
#include <unistd.h>			//Needed for SPI port

#include "spi.h"


int spi_cs_fd;
//struct spi_ioc_transfer spi_transfer;



//***********************************
//***********************************
//********** SPI OPEN PORT **********
//***********************************
//***********************************
int spi_open(int* spi_fd)
{

    unsigned char spi_mode;
    unsigned char spi_bitsPerWord;
    unsigned int spi_speed;

	int status_value = -1;

    //int *spi_cs_fd;

    //----- SET SPI MODE -----
    //SPI_MODE_0 (0,0) 	CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge
    //SPI_MODE_1 (0,1) 	CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on falling edge, output data (change) on rising edge
    //SPI_MODE_2 (1,0) 	CPOL = 1, CPHA = 0, Clock idle high, data is clocked in on falling edge, output data (change) on rising edge
    //SPI_MODE_3 (1,1) 	CPOL = 1, CPHA = 1, Clock idle high, data is clocked in on rising, edge output data (change) on falling edge
    spi_mode = SPI_MODE_3;
    
    //----- SET BITS PER WORD -----
    spi_bitsPerWord = SPI_BIT_PER_WORD;
    
    //----- SET SPI BUS SPEED -----
    spi_speed = SPI_SPEED;
    
    *spi_fd = open(SPI_DEV_NAME, O_RDWR);
    if (*spi_fd < 0)
    {
        printf("Error - Could not open SPI device");        
        return (-1);
    }

    status_value = ioctl(*spi_fd, SPI_IOC_WR_MODE, &spi_mode);
    if(status_value < 0)
    {
        printf("Could not set SPIMode (WR)...ioctl fail");
        return (-1);
    }

    status_value = ioctl(*spi_fd, SPI_IOC_RD_MODE, &spi_mode);
    if(status_value < 0)
    {
      printf("Could not set SPIMode (RD)...ioctl fail");
      return (-1);
    }

    status_value = ioctl(*spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bitsPerWord);
    if(status_value < 0)
    {
      printf("Could not set SPI bitsPerWord (WR)...ioctl fail");
      return (-1);
    }

    status_value = ioctl(*spi_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bitsPerWord);
    if(status_value < 0)
    {
      printf("Could not set SPI bitsPerWord(RD)...ioctl fail");
     return (-1);
    }

    status_value = ioctl(*spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
    if(status_value < 0)
    {
      perror("Could not set SPI speed (WR)...ioctl fail");
     return (-1);
    }

    status_value = ioctl(*spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
    if(status_value < 0)
    {
      perror("Could not set SPI speed (RD)...ioctl fail");
      return (-1);
    }    
    return(status_value);
}

int spi_open2(int* spi_fd)
{

    unsigned char spi_mode;
    unsigned char spi_bitsPerWord;
    unsigned int spi_speed;

	int status_value = -1;

    //int *spi_cs_fd;

    //----- SET SPI MODE -----
    //SPI_MODE_0 (0,0) 	CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge
    //SPI_MODE_1 (0,1) 	CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on falling edge, output data (change) on rising edge
    //SPI_MODE_2 (1,0) 	CPOL = 1, CPHA = 0, Clock idle high, data is clocked in on falling edge, output data (change) on rising edge
    //SPI_MODE_3 (1,1) 	CPOL = 1, CPHA = 1, Clock idle high, data is clocked in on rising, edge output data (change) on falling edge
    spi_mode = SPI_MODE_3;
    
    //----- SET BITS PER WORD -----
    spi_bitsPerWord = SPI_BIT_PER_WORD;
    
    //----- SET SPI BUS SPEED -----
    spi_speed = SPI_SPEED;
    
    spi_cs_fd = open(SPI_DEV_NAME, O_RDWR);
    if (spi_cs_fd < 0)
    {
        printf("Error - Could not open SPI device");        
        return (-1);
    }

    status_value = ioctl(spi_cs_fd, SPI_IOC_WR_MODE, &spi_mode);
    if(status_value < 0)
    {
        printf("Could not set SPIMode (WR)...ioctl fail");
        return (-1);
    }

    status_value = ioctl(spi_cs_fd, SPI_IOC_RD_MODE, &spi_mode);
    if(status_value < 0)
    {
      printf("Could not set SPIMode (RD)...ioctl fail");
      return (-1);
    }

    status_value = ioctl(spi_cs_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bitsPerWord);
    if(status_value < 0)
    {
      printf("Could not set SPI bitsPerWord (WR)...ioctl fail");
      return (-1);
    }

    status_value = ioctl(spi_cs_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bitsPerWord);
    if(status_value < 0)
    {
      printf("Could not set SPI bitsPerWord(RD)...ioctl fail");
     return (-1);
    }

    status_value = ioctl(spi_cs_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
    if(status_value < 0)
    {
      perror("Could not set SPI speed (WR)...ioctl fail");
     return (-1);
    }

    status_value = ioctl(spi_cs_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
    if(status_value < 0)
    {
      perror("Could not set SPI speed (RD)...ioctl fail");
      return (-1);
    }

    *spi_fd = spi_cs_fd;
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
    status_value = close(spi_cs_fd);
    if(status_value < 0)
    {
    	printf("Error - Could not close SPI device");
    	return (-1);
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

	retVal = ioctl(spi_cs_fd, SPI_IOC_MESSAGE(1), &spi);

	if(retVal < 0)
	{
		printf("Error - Problem transmitting spi data..ioctl");
		return (-1);
	}

    spi.len = Length;

	return retVal;
}


