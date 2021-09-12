# built-in
# third party
# local
import FancyCli

#========================== classes =========================================

#=== app

class ScumProgrammer(object):
    def __init__(self):
        fancyCli = FancyCli.FancyCli()

#========================== main ==============================================

def main():
    print('This is the SCuM program')
    scumProgrammer = ScumProgrammer()

if __name__=='__main__':
    main()
