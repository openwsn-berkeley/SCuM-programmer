# built-in
# third party
# local
from scumprogrammer import ScumUtils as u
from scumprogrammer import OpenHdlc

class ScumConnector(object):
    
    CHUNK_SIZE               = 50
    
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
    
    def __init__(self,serialport):
        
        # store params
        self.serialport      = serialport
        
        # local variables
        self.hdlc            = OpenHdlc.OpenHdlc(
            serialport       = serialport,
            rx_frame_cb      = self._hdlc_rx_frame_cb,
        )
    
    #======================== public ==========================================
    
    #=== admin
    
    def set_serialport(self,serialport):
        self.hdlc.set_serialport(serialport)
    
    def get_serialport(self):
        return self.hdlc.get_serialport()
    
    def get_isconnected(self):
        return self.hdlc.get_isconnected()
    
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
        self.hdlc.send([self.CMD_CLEAR])
        
        # send chunks
        for chunk in chunks:
            self.hdlc.send([self.CMD_CHUNK]+chunk)
        
        # send load
        self.hdlc.send([self.CMD_LOAD])
    
    def reset(self):
        self.hdlc.send([self.CMD_RESET])
    
    def GPIOcal(self,isOn):
        if isOn:
            state = 0x01
        else:
            state = 0x00
        self.hdlc.send([self.CMD_GPIOCAL,state])
    
    #======================== private =========================================
    
    def _hdlc_rx_frame_cb(self,frame):
        print('_hdlc_rx_frame_cb {}'.format(frame))