#I2C-IMU SET UP, 10/04/2019
#Libararies
import jetsonI2C
import time

#BNO05 config
IMU_Address = 0xA0 #I2C ADDRESS
ACCEL_REG = 0x08
UNIT_SELECT_REG = 0x3B
CONFIG_REG = 0x3D # Want to set to xxxx-0000 for config(19ms wait)
#then xxxx 1011(7ms)after setup, use sleep(.05)

