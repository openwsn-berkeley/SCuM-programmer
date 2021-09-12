# built-in
import threading
# third party
# local
import VERSION
import ScumUtils as u

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
            self._printBanner()
            while True:
                cmd = input()
                cmd = cmd.strip()
                if cmd=='q':
                    break
        except Exception as err:
            u.handleCrash(self.name,err)
    
    def _printBanner(self):
        output = self.BANNER+ \
        [
            '',
            'running version {}'.format(VERSION.VERSION),
            '',
            'enter \'q\' to exit',
        ]
        print('\n'.join(output))
