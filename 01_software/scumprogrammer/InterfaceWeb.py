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
    DFLT_BINFILE             = 'dummy.bin'
    
    def __init__(self,scumConnector):
        
        # store params
        self.scumConnector   = scumConnector
        
        # local variables
        self.startTs         = time.time()
        self.binfile         = self.DFLT_BINFILE
        self.GPIOcalIsOn     = False
        
        # find data files
        #     (they are at different location when running from
        #      source code or after installing through pip)
        self.folder_views    = pkg_resources.resource_filename(__name__, 'views/')
        self.folder_static   = pkg_resources.resource_filename(__name__, 'static/')
        
        # start web server
        self.websrv          = bottle.Bottle()
        self.websrv.route('/',                   'GET',    self._webhandle_root_GET)
        self.websrv.route('/static/<filename>',  'GET',    self._webhandle_static_GET)
        self.websrv.route('/favicon.ico',        'GET',    self._webhandle_favicon_GET)
        
        self.websrv.route('/data.json',          'GET',    self._webhandle_data_GET)
        self.websrv.route('/action.json',        'POST',   self._webhandle_action_POST)
        self.webthread            = threading.Thread(
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
        self.webthread.name       = 'InterfaceWeb'
        self.webthread.daemon     = True
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
    
    def _webhandle_favicon_GET(self):
        return bottle.static_file('favicon.ico', root=self.folder_static)
    
    def _webhandle_data_GET(self):
        return {
            'cellText': {
                'versions':       'software: {}'.format(VERSION.VERSION),
                'serialport':     self.scumConnector.get_serialport(),
                'binfile':        self.binfile,
            },
            'isconnected':        self.scumConnector.get_isconnected(),
            'GPIOcalIsOn':        self.GPIOcalIsOn,
        }
    
    def _webhandle_action_POST(self):
        cmdjson = bottle.request.json
        elem    = cmdjson['element'][len("button_"):-len("_rect")]
        if   elem=='bootload':
            self.scumConnector.bootload(self.binfile)
        elif elem=='reset':
            self.scumConnector.reset()
        elif elem=='GPIOcal':
            self.scumConnector.GPIOcal(isOn=self.GPIOcalIsOn)
            self.GPIOcalIsOn = not self.GPIOcalIsOn
        return {}
    
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
