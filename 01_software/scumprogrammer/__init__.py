# built-in
# third party
# local
from scumprogrammer import VERSION
from scumprogrammer import ScumConnector
from scumprogrammer import InterfaceWeb
from scumprogrammer import InterfaceCli

#========================== classes =========================================

#========================== main ==============================================

def main_app():
    scumConnector = ScumConnector.ScumConnector()
    interfaceWeb  = InterfaceWeb.InterfaceWeb()
    interfaceCli  = InterfaceCli.InterfaceCli()