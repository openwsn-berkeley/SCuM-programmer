# built-in
import threading
import time
# third party
import blessed

class FancyCli(threading.Thread):
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
        self.term = blessed.Terminal()
        super(FancyCli, self).__init__(*a, **kw)
        self.name = 'FancyCli'
        self.start()
    
    def run(self):
        try:
            self._playIntro()
            self._createPanels()
            curserOn = True
            with self.term.cbreak():
                val = ''
                while val.lower() != 'q':
                    val = self.term.inkey(timeout=0.5)
                    if val:
                        print(self.term.gold_on_blue(val)+self.term.move_up(1))
                        print(self.term.move_right(2))
                    else:
                        curserOn = not curserOn
                        with self.term.location():
                            if curserOn:
                                print(self.term.on_gold(' '))
                            else:
                                print(self.term.on_blue(' '))
        except:
            print('CRITICAL!!!')
    
    def _playIntro(self):
        with self.term.hidden_cursor():
            print(self.term.home + self.term.gold_on_blue + self.term.clear + self.term.move_y((self.term.height // 2)-(len(self.BANNER)//2+1)))
            for line in self.BANNER:
                print(self.term.center(line))
                time.sleep(0.15)
                #print(self.term.move_down(1))
            time.sleep(0.5)
            print(self.term.home + self.term.clear)
    
    def _createPanels(self):
        pass