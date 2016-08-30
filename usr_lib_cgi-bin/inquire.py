#!/usr/bin/python
# -*- coding: utf-8 -*-

import socket
import sys
import os
import json
from contextlib import closing

host = '127.0.0.1'
port = 4000
command = 'Inquire'

# receive status from socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
with closing(sock):
	sock.connect((host, port))
	sock.send(command)
	status = sock.recv(1024)

jsonData = json.loads(status);

# read count-down timer
f = open('cdt.js')
cdt_lines = f.readlines()
f.close()

# print out html
list_status = ["待機中", "強火", "弱火", "中火", "消火"]
list_thermo = ["オフ（95℃未満）",  "オン（95℃以上）"]

print "Content-type: text/html"
print
print "<html>"
print "<head>"
print "<title>Result</title>"
print "<meta http-equiv=\"refresh\" content=\"3\">"
print "</head>"
print "<body>"
if jsonData['state'] != 0:
	print "<font size=7 color=\"red\">"
else:
	print "<font size=7 color=\"blue\">"
print list_status[jsonData['state']] + "<br>"
print "</font>"
print "<font size=5>"
print "ダイアル位置 : " + str(jsonData['pos']) + "/10" + "<br>"
print "サーモスタット : " + list_thermo[jsonData['thermo']] + "<br>"
print "<div id=\"CDT\"></div>"
for line in cdt_lines:
	print line.replace('%time_limit%', str(jsonData['time'])),
print "</font>"

print "<div id=\"graphdiv\"></div>"
print "<script type=\"text/javascript\" src=\"../dygraph-combined.js\">"
print "</script>"
print "<script type=\"text/javascript\">"
print "container = document.getElementById(\"graphdiv\");";
print "g = new Dygraph(container,\"Time,Temperature\\n\"+";
print "\"1,1\\n\" +"
print "\"2,3\\n\""
print ");"
print "</script>"
print "</body>"
print "</html>"

