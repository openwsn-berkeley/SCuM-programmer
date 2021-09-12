# built-in
import threading
import time
# third party
# local

class InterfaceCli(threading.Thread):
    BANNER = [
    '   _____  _____      __  __ ',
    '  / ____|/ ____|    |  \/  |',
    ' | (___ | |    _   _| \  / |',
    '  \___ \| |   | | | | |\/| |',
    '  ____) | |___| |_| | |  | |',
    ' |_____/ \_____\__,_|_|  |_|',
    '         www.crystalfree.org',
    ]
    def __init__(self, *a, **kw):
        super(InterfaceCli, self).__init__(*a, **kw)
        self.name = 'InterfaceCli'
        self.start()
    
    def run(self):
        try:
            self._playIntro()
            while True:
                cmd = input()
                cmd = cmd.strip()
                if cmd=='q':
                    break
        except Exception as err:
            print(type(err))
            print(err)
    
    def _playIntro(self):
        print('\n'.join(self.BANNER))
