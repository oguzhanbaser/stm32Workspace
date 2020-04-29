import sys
from PySide2.QtWidgets import *
from PySide2.QtCore import Qt

from serialThread import serialThreadClass

class MainApp(QWidget):

    def __init__(self):
        super().__init__()

        # import serial communication class 
        self.mySerial = serialThreadClass()
        self.mySerial.msg.connect(self.serialEventHandler)

        # gui design section

        # com name and baud rate settings
        self.mComLabel = QLineEdit()
        self.mBaudLabel = QLineEdit()

        # connection button
        mConnBtn = QPushButton("BAĞLANTI")
        mConnBtn.clicked.connect(self.serialPortBtn)
        
        # info labels
        self.connStatLbl = QLabel("Bağlı değil")
        self.connStatLbl.setAlignment(Qt.AlignCenter)

        lblPort = QLabel("PORT İSMİ")
        lblPort.setAlignment(Qt.AlignCenter)
        lnbBaud = QLabel("BAUD RATE")
        lnbBaud.setAlignment(Qt.AlignCenter)

        # sliders for changing RGB LED brighness
        mSlider1 = QSlider()
        mSlider1.setOrientation(Qt.Horizontal)
        mSlider1.setRange(0, 100)
        mSlider1.valueChanged.connect(self.slider1Val)
        self.mSliTxt1 = QLabel("---")
        self.mSliTxt1.setAlignment(Qt.AlignCenter)

        mSlider2 = QSlider()
        mSlider2.setOrientation(Qt.Horizontal)
        mSlider2.setRange(0, 100)
        mSlider2.valueChanged.connect(self.slider2Val)
        self.mSliTxt2 = QLabel("---")
        self.mSliTxt2.setAlignment(Qt.AlignCenter)

        mSlider3 = QSlider()
        mSlider3.setOrientation(Qt.Horizontal)
        mSlider3.setRange(0, 100)
        mSlider3.valueChanged.connect(self.slider3Val)
        self.mSliTxt3 = QLabel("---")
        self.mSliTxt3.setAlignment(Qt.AlignCenter)

        # progress bars for showing adc values
        self.mProBar = QProgressBar()
        self.mProBar2 = QProgressBar()

        # led on led off buttons
        ledOnBtn = QPushButton("LED YAK")
        ledOnBtn.clicked.connect(self.btn1Click)

        ledOffBtn = QPushButton("LED Söndür")
        ledOffBtn.clicked.connect(self.btn2Click)

        # create girdlayout and place widgets on to it
        mGrid = QGridLayout()

        mGrid.addWidget(lblPort, 1, 1)
        mGrid.addWidget(lnbBaud, 1, 2)
        mGrid.addWidget(self.connStatLbl, 1, 3)

        mGrid.addWidget(self.mComLabel, 2, 1)
        mGrid.addWidget(self.mBaudLabel, 2, 2)
        mGrid.addWidget(mConnBtn, 2, 3)

        mGrid.addWidget(mSlider1, 3, 1, 1, 2)
        mGrid.addWidget(self.mSliTxt1, 3, 3)

        mGrid.addWidget(mSlider2, 4, 1, 1, 2)
        mGrid.addWidget(self.mSliTxt2, 4, 3)

        mGrid.addWidget(mSlider3, 5, 1, 1, 2)
        mGrid.addWidget(self.mSliTxt3, 5, 3)

        mGrid.addWidget(self.mProBar, 6, 1, 1, 3)
        self.mProBar.setValue(0)

        mGrid.addWidget(self.mProBar2, 7, 1, 1, 3)
        self.mProBar2.setValue(0)

        mGrid.addWidget(ledOnBtn, 8, 1)
        mGrid.addWidget(ledOffBtn, 8, 3)

        self.mBaudLabel.setText("115200")
        self.mComLabel.setText("COM5")

        self.setLayout(mGrid)
        # self.setGeometry(300, 300, 300, 300)
        self.show()

    # callbacl section

    # serial connection button
    # if connection is already open close connection
    def serialPortBtn(self):

        # check is serial port open
        # if serial port is open, close serial port
        if self.mySerial.isPortOpen():
            self.mySerial.terminate()               # stop serial port thread
            self.mySerial.portClose()
            self.connStatLbl.setText("Bağlı Değil")
        else:   # if serial port is close, open serial port
            try:
                mBaud = int(self.mBaudLabel.text())
            except:
                print("Baud Rate Hatası")
                return

            mPortName = self.mComLabel.text()
            self.mySerial.setBaudRate(mBaud)
            self.mySerial.setPortName(mPortName)
            if self.mySerial.portOpen():
                self.connStatLbl.setText("Bağlandı")
                self.mySerial.start()               # start serial port thread

    # slider 1 value changed callback
    def slider1Val(self, pVal):
        self.mSliTxt1.setText(str(pVal))
        _packet = [0xA5, 0x5A, 0x02, 0x80]      # data packet 
        _packet.append(pVal)                    # append data to data packet
        sendPacket = bytearray(_packet)
        self.mySerial.sendBytes(sendPacket)

    # slider 2 value changed callback
    def slider2Val(self, pVal):
        self.mSliTxt2.setText(str(pVal))
        _packet = [0xA5, 0x5A, 0x02, 0x81]
        _packet.append(pVal)
        sendPacket = bytearray(_packet)
        self.mySerial.sendBytes(sendPacket)

    # slider 3 value changed callback
    def slider3Val(self, pVal):
        self.mSliTxt3.setText(str(pVal))
        _packet = [0xA5, 0x5A, 0x02, 0x82]
        _packet.append(pVal)
        sendPacket = bytearray(_packet)
        self.mySerial.sendBytes(sendPacket)

    # serial data receive callback 
    def serialEventHandler(self, pData):
        mData = pData.split('|')
        if mData[0] == '#':
            self.mProBar.setValue(int(mData[1]))
            self.mProBar2.setValue(int(mData[2]))

    # button 1 click callback
    def btn1Click(self):
        _packet = [0xA5, 0x5A, 0x02, 0x84]
        _packet.append(0x01)
        sendPacket = bytearray(_packet)
        self.mySerial.sendBytes(sendPacket)

    # button 2 click callback
    def btn2Click(self):
        _packet = [0xA5, 0x5A, 0x02, 0x84]
        _packet.append(0x00)
        sendPacket = bytearray(_packet)
        self.mySerial.sendBytes(sendPacket)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    ex = MainApp()
    sys.exit(app.exec_())