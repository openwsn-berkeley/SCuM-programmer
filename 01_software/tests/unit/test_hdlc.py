# built-in
import logging
import random
# third party
# local
import testUtils as utils
from scumprogrammer import OpenHdlc

#============================ logging ===============================

log = logging.getLogger(utils.getMyLoggerName())
log.addHandler(utils.NullHandler())

#============================ defines ===============================

#============================ fixtures ==============================

#============================ helpers ===============================

#============================ tests =================================

def test_hdlcify_dehdlcify(logFixture):
    _hdlc               = OpenHdlc.OpenHdlc()
    
    for run in range(10000):
        length          = random.randint(1,1000)
        data            = []
        for i in range(length):
            data       += [random.randint(0x00,0xff)]
        hdlc_data       = _hdlc.hdlcify(data)
        dehdlc_data     = _hdlc.dehdlcify(hdlc_data)
        assert dehdlc_data == data