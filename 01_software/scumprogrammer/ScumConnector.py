# built-in
import threading
import time
# third party
import serial
# local
from scumprogrammer import ScumUtils as u

class ScumConnector(threading.Thread):
    
    def __init__(self,serialport=None):
        
        # store params
        self.serialport      = serialport
        
        # local variables
        self.datalock        = threading.RLock()
        self.isconnected     = False
        
        # initialize thread
        super(ScumConnector, self).__init__()
        self.name            = 'ScumConnector'
        self.daemon          = True
        self.start()
    
    def run(self):
        try:
            while True:
                time.sleep(1)
                ser = serial.Serial(self.serialport,19200)
        except Exception as err:
            u.handleCrash(self.name,err)
    
    #======================== public ==========================================
    
    def set_serialport(self,serialport):
        with self.datalock:
            self.serialport = serialport
    
    #======================== private =========================================