# built-in
import os
import webbrowser
import pkg_resources
import random
import time
# third party
import bottle
import threading
# local
from scumprogrammer import VERSION
from scumprogrammer import ScumUtils as u

class InterfaceWeb(object):
    
    TCPPORT                  = 8080
    
    def __init__(self):
        
        # store params
        
        # local variables
        self.startTs         = time.time()
        
        # find data files
        #     (they are at different location when running from
        #      source code or after installing through pip)
        self.folder_views    = pkg_resources.resource_filename(__name__, 'views/')
        self.folder_static   = pkg_resources.resource_filename(__name__, 'static/')
        
        # start web server
        self.websrv          = bottle.Bottle()
        self.websrv.route('/',                   'GET',    self._webhandle_root_GET)
        self.websrv.route('/static/<filename>',  'GET',    self._webhandle_static_GET)
        self.websrv.route('/data.json',  'GET',            self._webhandle_data_GET)
        self.webthread = threading.Thread(
            target = self._bottle_try_running_forever,
            args   = (self.websrv.run,),
            kwargs = {
                'host': '127.0.0.1',
                'port': self.TCPPORT,
                'quiet': True,
                'debug': False,
            }
        )
        bottle.TEMPLATE_PATH.insert(0,self.folder_views)
        self.webthread.name = 'InterfaceWeb'
        self.webthread.daemon= True
        self.webthread.start()
        
        # open browser
        webbrowser.open('http://127.0.0.1:{0}'.format(self.TCPPORT))
    
    #======================== public ==========================================
    
    #======================== private =========================================
    
    #=== web handlers
    
    def _webhandle_root_GET(self):
        return bottle.template('index')
    
    def _webhandle_static_GET(self,filename):
        return bottle.static_file(filename, root=self.folder_static)
    
    def _webhandle_data_GET(self):
    
        animdur = 20
        uptime  = (time.time()-self.startTs)%animdur
        chunks  = []
        print(uptime)
        if   uptime<(1/3)*animdur:
            chunks   = [0]*64
        elif uptime<(2/3)*animdur:
            progress = (uptime-(animdur/3))/(animdur/3)
            numright = int(64*progress)
            chunks   = [1]*numright+[0]*(64-numright)
        else:
            progress = (uptime-(2*animdur/3))/(animdur/3)
            numright  = int(64*progress)
            chunks   = [2]*numright+[1]*(64-numright)
        print(chunks)
        assert len(chunks)==64
    
        return {
            'statuspane':    {
                'labelcomputer': 'AllGpioToggle.bin',
                'labelgateway':  'scum-programmer 2.0.1<br/>version 2.0.3 available, <a>upgrade instructions</a>',
                'labelscum':     'AllGpioToggle.bin<br/>running for 53 s',
                'chunks':        chunks,
            },
            'uartpane':      {
                'messages': [
                    ['computer', 'test'],
                    ['SCuM',     'This is SCum!'],
                    ['SCuM',     'This is SCum!'],
                    ['SCuM',     'This is SCum!'],
                ],
            },
            'spectrumpane':  {
                'rssis': [random.randint(-100,-30) for _ in range(80)],
            },
            'versionlabel':  'scumprogrammer {} (up-to-date)'.format(VERSION.VERSION),
        }
    
    #=== web server admin
    
    def _bottle_try_running_forever(self,*args,**kwargs):
        RETRY_PERIOD = 3
        while True:
            try:
                args[0](**kwargs) # blocking
            except socket.error as err:
                if err[0]==10013:
                    print('FATAL: cannot open TCP port {0}.'.format(kwargs['port']))
                    print('    Is another application running on that port?')
                else:
                    u.handleCrash(self.webthread.name,err)
            except Exception as err:
                u.handleCrash(self.webthread.name,err)
            print('    Trying again in {0} seconds'.format(RETRY_PERIOD))
            for _ in range(RETRY_PERIOD):
                time.sleep(1)
                print('.')
            print('')
