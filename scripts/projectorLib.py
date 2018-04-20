
from socket import socket
import hashlib

# Protocol format:
#    ftp://ftp.panasonic.com/pub/Panasonic/Drivers/PBTS/manuals/OM_PT-D232C.pdf
#    https://panasonic.net/cns/prodisplays/support/download/pdf/LAN_Protocol_exp.pdf

class Projector(object):

    def __init__(self,host,port,user,pword):
        self._sock = socket()
        self._sock.connect((host,port))
        self._f = self._sock.makefile('rwb',newline="\r")

        r = self._f.read(20)
        d = r.split()

        authHash = hashlib.md5()
        hashIn = user.encode('utf-8') + b":" + pword.encode('utf-8') + b":" + d[2]
        authHash.update(hashIn)
        self._token = authHash.hexdigest().encode('utf-8')

    def readline(self):
        data = []
        c = self._f.read(1)
        while c:

            if c == b'\r':
                if len(data) > 2:
                    break;
            else:
                #print("Got {}".format(c))
                data.append(c)
            c = self._f.read(1)

        return b''.join(data[2:])

    def cmd(self,cmd):
        d = self._token + b"00" + cmd.encode('utf-8') + b"\r"
        self._f.write(d)
        self._f.flush()
        return self.readline()

    def getPower(self):
        return self.cmd("QPW") == b"001"

    def setPower(self,on):
        if on:
            self.cmd("PON")
        else:
            self.cmd("POF")

    def getShutter(self):
        return self.cmd("QSH") == b"1"

    def setShutter(self,on):
        if on:
            self.cmd("OSH:1")
        else:
            self.cmd("OSH:0")

