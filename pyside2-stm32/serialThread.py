import serial
from PySide2.QtCore import Signal, QThread

class serialThreadClass(QThread):

    msg = Signal(str)

    def __init__(self, parent=None):
        super(serialThreadClass, self).__init__(parent)
        self.serialport = serial.Serial()
        self.serialport.baudrate = 115200

    def isPortOpen(self):
        return self.serialport.isOpen()

    def setPortName(self, pName):
        self.serialport.port = pName

    def setBaudRate(self, pBaud):
        self.serialport.baudrate = pBaud

    def portOpen(self):
        retVal = False
        try:
            self.serialport.open()
            retVal = True
        except:
            print("Port Açma Hatası")

        return retVal

    def portClose(self):
        self.serialport.close()

    def run(self):
        while True:
            # _data = self.serialport.readline()
                try:
                    _data = self.serialport.readline()
                    mData = _data.decode("utf-8") 
                    self.msg.emit(str(mData))
                except:
                    pass

    def sendSerial(self, pData):
        if self.serialport.isOpen():
            self.serialport.write(serial.to_bytes(pData.encode()))

    def sendBytes(self, pData):
        if self.serialport.isOpen():
            self.serialport.write(pData)