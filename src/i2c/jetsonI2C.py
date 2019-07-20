from smbus2 import SMBus, SMBusWrapper, i2c_msg

# jetsonI2C class
class jetsonI2C(object):
    def __init__(self, bus, address):
        self.bus = bus
        self.address = address

    def change_address(self, address):
        self.address = address

    # read # of bytes from address with offset
    def read_byte(self, offset, bytes):
        with SMBusWrapper(1) as bus:
            return bus.read_i2c_block_data(self.address, offset, bytes)

    # write # of bytes to address with offset
    def write_byte(self, offset, data):
        with SMBusWrapper(1) as bus:
            bus.write_i2c_block_data(self.address, offset, data)
    