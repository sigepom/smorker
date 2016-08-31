#!/usr/bin/python
# -*- coding: utf-8 -*-

import socket
import sys
import os
import json
from contextlib import closing

host = '127.0.0.1'
port = 4000
command = 'Log'

# receive status from socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
with closing(sock):
	sock.connect((host, port))
	sock.send(command)
	status = sock.recv(32768)

jsonData = json.loads(status);

# print out html

print "Content-type: text/html"
print
print "<html>"
print "<head>"
print "<title>Result</title>"
print "<meta http-equiv=\"refresh\" content=\"5\">"
print "</head>"
print "<body>"
print "<div id=\"graphdiv\"></div>"
print "<script type=\"text/javascript\" src=\"../dygraph-combined.js\">"
print "</script>"
print "<script type=\"text/javascript\">"
print "container = document.getElementById(\"graphdiv\");"
print "g = new Dygraph(container,\"Time,Temperature\\n\"",
for tim in jsonData.keys():
	print "+\"" + str(tim) + "," + str(jsonData[tim]) + "\\n\"",
print ",{",
print "labels: [\"Time\", \"Temperature\"],",
print "xValueFormatter: Dygraph.dateString_, xValueParser: function(x) {return 1000*parseInt(x);}, xTicker: Dygraph.dateTicker,",
#print "axes: {x: {axisLabelFormatter: function(d, gran) {var d=new Date(d.getTime()); return d.strftime(\"%H:00\");}}}",
print "axes: {x: {axisLabelFormatter: function(d, gran) {var d=new Date(d.getTime()); return ('0'+d.getHours()).slice(-2)+':'+('0'+d.getMinutes()).slice(-2);}}}",
print "}",
print ");"
print "</script>"
print "</body>"
print "</html>"

