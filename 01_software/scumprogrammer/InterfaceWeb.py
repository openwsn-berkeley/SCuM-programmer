# built-in
import os
import webbrowser
# third party
import bottle
import threading
# local
import VERSION
import ScumUtils as u

class InterfaceWeb(object):
    
    TCPPORT                  = 8080
    
    def __init__(self):
        
        # store params
        
        # local variables
        
        # adapt template folder
        bottle.TEMPLATE_PATH.insert(0,os.path.join('scumprogrammer','views'))
        
        # start web server
        self.websrv          = bottle.Bottle()
        self.websrv.route('/', 'GET', self._webhandle_root_GET)
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
        self.webthread.name = 'InterfaceWeb'
        self.webthread.daemon= True
        self.webthread.start()
        
        # open browser
        webbrowser.open('http://127.0.0.1:{0}'.format(self.TCPPORT))
    
    #======================== public ==========================================
    
    #======================== private =========================================
    
    #=== web handlers
    
    def _webhandle_root_GET(self):
        return bottle.template(
            'index',
            version = VERSION.VERSION,
        )
    
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
