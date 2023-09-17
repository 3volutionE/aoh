
class ad7380 :
    Spi = None
        
    def __init__(self, spi = None) -> None:
        self.Spi = spi        
    
    def __if_spi(self) -> bool :
        return (self.Spi != None)
    
    def read_reg(self, reg_addr) -> int :        
        if(self.__if_spi()) :
            data_send = [((reg_addr & 0x07) << 4), 0]
            self.Spi.writebytes(data_send)
            data_read = self.Spi.readbytes(2)
            return (data_read[0] << 8) | (data_read[1] & 0x00FF)
        else :
            print ("Error, Spi is not initialized")
            return -1;

    def write_reg(self, reg_addr, reg_val) -> int :
        if(self.__if_spi()) :
            data_send = [(((reg_addr | 0x08) << 4) | (reg_val >> 8)) , (reg_val & 0x00FF)]
            self.Spi.writebytes(data_send)
            return 0
        else:
            return -1
        
    def read_adc(self) -> int :
        if (self.__if_spi()) :
            data_read = self.Spi.readbytes(4)           # 2-Wire mode, each transaction use 32 clocks (4 Bytes)
            return data_read
        else:
            return -1;
                
        
    
    