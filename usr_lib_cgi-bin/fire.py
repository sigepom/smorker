#!/usr/bin/python

import socket
import sys
import os
from contextlib import closing

host = '127.0.0.1'
port = 4000
command = os.environ['QUERY_STRING']

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
with closing(sock):
	sock.connect((host, port))
	sock.send(command)

print "Content-type: text/html"
print
print "<html>"
print "<head>"
print "<title>Result</title>"
print "<meta http-equiv=\"refresh\" content=\"3;URL=/\">"
print "</head>"
print "<body>Sending " + command + " command</body>"
print "</html>" 
