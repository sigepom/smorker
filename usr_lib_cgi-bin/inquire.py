#!/usr/bin/python

import socket
import sys
import os
from contextlib import closing

host = '127.0.0.1'
port = 4000
command = 'Inquire'

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
with closing(sock):
	sock.connect((host, port))
	sock.send(command)
	status = sock.recv(1024)

print "Content-type: text/html"
print
print "<html>"
print "<head>"
print "<title>Result</title>"
print "</head>"
print "<body>Status " + status
print "</html>" 
