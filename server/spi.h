#ifndef __SPI_H_
#define __SPI_H_

    #define SPI_DEV_NAME            "/dev/spidev0.0"
    
    #define SPI_SPEED               10000000            // 10 MHz
    #define SPI_BIT_PER_WORD        8 


    int spi_open(int* spi_fd);
    int SpiClosePort (int spi_device);

#endif