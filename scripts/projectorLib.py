
from socket import socket
import hashlib

# Protocol format:
#    ftp://ftp.panasonic.com/pub/Panasonic/Drivers/PBTS/manuals/OM_PT-D232C.pdf
#    https://panasonic.net/cns/prodisplays/support/download/pdf/LAN_Protocol_exp.pdf

class Projector(object):

    def __init__(self,host,port,user,pword):
        self._host  = host
        self._port  = port
        self._user  = user
        self._pword = pword
        self._token = None
        self._sock  = None
        self._f     = None

        self.connect()

    def connect(self):
        if self._token is None:

            self._sock = socket()
            self._sock.connect((self._host,self._port))
            self._f = self._sock.makefile('rwb',newline="\r")

            r = self._f.read(20)
            d = r.split()

            authHash = hashlib.md5()
            hashIn = self._user.encode('utf-8') + b":" + self._pword.encode('utf-8') + b":" + d[2]
            authHash.update(hashIn)
            self._token = authHash.hexdigest().encode('utf-8')

    def disconnect(self):
        self._sock.close()
        self._token = None
        self._sock  = None
        self._f     = None

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
        self.connect()
        d = self._token + b"00" + cmd.encode('utf-8') + b"\r"
        self._f.write(d)
        self._f.flush()
        ret = self.readline()
        self.disconnect()
        return ret

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

