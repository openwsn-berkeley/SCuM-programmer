# built-in
import threading
import time
# third party
import serial
# local
from scumprogrammer import ScumUtils as u
from scumprogrammer import OpenHdlc

class ScumConnector(threading.Thread):
    
    CHUNK_SIZE               = 100
    
    CMD_CLEAR                = 0x01
    CMD_CHUNK                = 0x02
    CMD_LOAD                 = 0x03
    CMD_RESET                = 0x04
    CMD_GPIOCAL              = 0x05
    CMD_ALL                  = [
        CMD_CLEAR,
        CMD_CHUNK,
        CMD_LOAD,
        CMD_RESET,
        CMD_GPIOCAL,
    ]
    
    def __init__(self,serialport=None):
        
        # store params
        self.serialport      = serialport
        
        # local variables
        self.hdlc            = OpenHdlc.OpenHdlc()
        self.datalock        = threading.RLock()
        self.serialTxLock    = threading.RLock()
        self.isconnected     = False
        
        # initialize thread
        super(ScumConnector, self).__init__()
        self.name            = 'ScumConnector'
        self.daemon          = True
        self.start()
    
    def run(self):
        while True:
            try:
                with self.datalock:
                    self.ser           = serial.Serial(self.serialport,1000000)
                    self.isconnected   = True
                while True:
                    c = self.ser.read(1)
                    if len(c)==0:
                       raise Exception()
            except Exception as err:
                with self.datalock:
                    self.isconnected   = False
                time.sleep(3)
    
    #======================== public ==========================================
    
    #=== admin
    
    def set_serialport(self,serialport):
        with self.datalock:
            self.serialport = serialport
    
    def get_serialport(self):
        with self.datalock:
            return self.serialport
    
    def get_isconnected(self):
       with self.datalock:
            return self.isconnected
    
    #=== commands
    
    def bootload(self,binfile):
    
        # read file
        with open(binfile,'rb') as f:
            bindata = [b for b in f.read()]
        print('{} bytes'.format(len(bindata)))
        
        # cut in chunks
        chunks = []
        i=0
        while i<len(bindata):
            chunks+= [bindata[i:i+self.CHUNK_SIZE]]
            i     += self.CHUNK_SIZE
        print('{} chunks of {} bytes each'.format(len(chunks),self.CHUNK_SIZE))
        print('start')
        # send clear
        self._serialsend([self.CMD_CLEAR])
        
        # send chunks
        for chunk in chunks:
            self._serialsend([self.CMD_CHUNK]+chunk)
        
        # send load
        self._serialsend([self.CMD_LOAD])
        print('done')
    
    def reset(self):
        self._serialsend([self.CMD_RESET])
    
    def GPIOcal(self,isOn):
        if isOn:
            state = 0x01
        else:
            state = 0x00
        self._serialsend([self.CMD_GPIOCAL,state])
    
    #======================== private =========================================
    
    def _serialsend(self,msg):
        # FIXME: ACK and retry
        msg = self.hdlc.hdlcify(msg)
        with self.serialTxLock:
            self.ser.write(msg)