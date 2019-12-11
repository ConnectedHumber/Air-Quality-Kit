import serial
import sys
import time
import threading

class CubeTerminal(object):

    def read_line_from_serial(self,ser):
        result = ""
        # Stop the ticker from reading the input
        while True:
            b=ser.read(size=1)
            if len(b)==0:
                # timeout
                return result
            c = chr(b[0])
            if c=='\n':
                return result
            result = result + c

    def read_line(self):
        return self.read_line_from_serial(self.serial_port)

    def send_text(self, text):
        
        if self.serial_port == None:
            self.set_status('Serial port not connected')
            return

        clean_text = ""
        for ch in text:
            if ch != '\n' and ch !='\r':
                clean_text = clean_text + ch
        
        text = clean_text.strip()
        text = text + '\r'
        return_text = new_numbers_lambda = map(lambda x : x if x != '\n' else '\r', text)

        byte_text = bytearray()
        byte_text.extend(map(ord,return_text))

        print("send line: " + text)
        for ch in byte_text:
            print(ch)

        self.serial_port.write(byte_text)

    def open_connection(self,port_name):
        try:
            port = serial.Serial(port_name, 115200, timeout=1)
        except (OSError, serial.SerialException):
            return None
        print("port open")
        return port

    def try_to_connect(self,port_name):

        self.serial_port = self.open_connection(port_name)

        return True
    
    def do_tick(self):
        while True:
            if self.serial_port != None:
                try:
                    while self.serial_port.in_waiting > 0:
                        b = self.serial_port.read()
                        c = chr(b[0])
                        print(c,end='')
                except:
                    self.serial_port.close()
                    self.serial_port = None
                    print('Serial port disconnected')
                    break
                
c = CubeTerminal()

c.try_to_connect("com4")
x = threading.Thread(target=c.do_tick, args=(1,))
x.start()

while True:
    print("tick")
    time.sleep(1)
