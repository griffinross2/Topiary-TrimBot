import serial
import serial.tools.list_ports

class USBDev():
    def __init__(self, vid=0x4842, pid=0x4001, baudrate = 115200):
        self.ser = None
        self.vid = vid
        self.pid = pid
        self.baudrate = baudrate

    # Find and connect to trimbot. If trimbot is not found, or the port could not be
    # opened, a serial exception is thrown
    def connect(self):
        ports = serial.tools.list_ports.comports()
        for port in ports:
            if port.vid and port.pid:
                print(f"{port.vid:04x}, {port.pid:04x}")
                if port.vid == self.vid and port.pid == self.pid:
                    # Found trimbot
                    print(f"Found: {port.device} - VID: {port.vid:04x}, PID - {port.pid:04x}")

                    self.ser = serial.serial_for_url(port.device, baudrate=self.baudrate, timeout=0, write_timeout=1)
                    if self.ser is None:
                        raise serial.SerialException("Could not create serial device.")

                    self.ser.reset_input_buffer()
                    self.ser.reset_output_buffer()
                    self.send(bytes([0]))

                    return
                
        raise serial.SerialException("Could not find serial device.")
    
    def disconnect(self):
        if self.ser:
            self.ser.close()

    def send(self, data: bytes):
        if self.ser:
            ret = self.ser.write(data)
            self.ser.flush()
            return ret
        return 0
    
    def receive(self, count):
        if self.ser:
            return self.ser.read(count)
        
    def clear(self):
        if self.ser:
            self.ser.reset_input_buffer()