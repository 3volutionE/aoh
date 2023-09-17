import spidev
import threading
import time

import RPi.GPIO as GPIO
import ADC


adc = None

t1 = time.time_ns()
t2 = time.time_ns()

print ("t1 = %d, t2 = %d" % (t1, t2))
diff = t2-t1
print ("diff %d" % (diff))


def thread_read_adc () :
    pass

def main():    
    print ("-- Initial SPI --")
    print ("-- Setup GPIO --")

    # Setup GPIO
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(25, GPIO.IN, pull_up_down=GPIO.PUD_UP)


    spi = spidev.SpiDev()
    spi.open(0,0)

    spi.max_speed_hz = 45000000
    spi.mode = 0b11

    to_send = [0x0123, 0x0234, 0x0356]
    #spi.xfer2 ([1,2,3], 1000000, 100, 7)
    #spi.readbytes(8)
    reg_val = [0 ,0]




    adc = ADC.ad7380(spi)

    regval = adc.read_reg(0x02)
    print ("Configuration 2 Reg = 0x%04X" % (regval))


    adc.write_reg(0x02, 0x00FF)         # Hard reset ADC
    time.sleep(1)                       # Wait 2 second for the reset process
    
    adc.write_reg(0x02, 0x0100)         # Set 2-Wire output mode, SDOA will output channel A and C, SDOB will outout channel B and D. We connect to only SDOA, so signal in must use channel A and C
    time.sleep(1)                       # Wait 2 second for the reset process
    regval = adc.read_reg(0x02)
    print ("Configuration 2 Reg = 0x%04X" % (regval))


    regval = adc.read_reg(0x02)
    print ("Configuration 2 Reg = 0x%04X" % (regval))


    adc.write_reg(0x05, 0x0500)         # Set alert to about 2.5V
    regval = adc.read_reg(0x02)
    print ("Alert High = 0x%04X" % (regval))

    adc.write_reg(0x01, 0x0008)         # Enable alert function
    regval = adc.read_reg(0x01)
    print ("Configuration 1 Reg = 0x%04X" % (regval))

    adc.write_reg(0x02, 0x0100)         # Set 2-Wire output mode, SDOA will output channel A and C, SDOB will outout channel B and D. We connect to only SDOA, so signal in must use channel A and C
    regval = adc.read_reg(0x02)
    print ("Configuration 2 Reg = 0x%04X" % (regval))

    i = 0
    t2 = 0;
    
    while i < 10 :
        t1 = time.perf_counter_ns()
        adcval = adc.read_adc()
        t2 = time.perf_counter_ns()
        
        print (adcval)
        #print ("t1 = %d, t2 = %d, diff = %d" % (t1, t2, t2-t1))
        #uint16_ch_a = (adc[0] << 8) | adc[1]
        #uint16_ch_b = (adc[2] << 8) | adc[3]
        #print ("Channel A = 0x%04X, voltage = %f V, Channel B = 0x%04X, voltage = %f V" % (uint16_ch_a, uint16_ch_a * (2 * 3.3 / 65536 ), uint16_ch_b, uint16_ch_b * (2 * 3.3 / 65536 )))
        if (GPIO.input(25) == 0) :
            print ("There is a trigger")
            regval = adc.read_reg(0x03)     # Read Alert register
            print("Alert Reg = 0x%04X" % (regval))
            break
        i += 1
        
    spi.close()



if __name__ == "__main__" :
    main()


