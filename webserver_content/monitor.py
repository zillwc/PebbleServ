#!/usr/bin/env python

"""
    PebbleServ
    This script should be ran as a service on your server
"""

import psutil
import socket
import json
import sys
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer

jSysStats = {"Data":{"cpu":0,"mem":0,"host":"DEFAULT"}}
defaultPort = 80
argc = len(sys.argv)

if (argc == 2):
    jPort = (int)(sys.argv[1])
elif (argc == 1):
    jPort = defaultPort
elif (argc > 2):
    print('USAGE: monitor.py <port>')
    exit(0)

try:
    server = HTTPServer(('', jPort), JSONHandler)
    print 'JSON server started on port %s' % jPort
    server.serve_forever()
except KeyboardInterrupt:
    print '^C received, shutting down server'
    server.socket.close()

class JSONHandler (BaseHTTPRequestHandler):
    def do_GET(self):
        try:
            if self.path.endswith(".json"):
                cpu = psutil.cpu_percent()
                mem = psutil.virtual_memory()[2]
                host = socket.gethostname()
                jSysStats["Data"]["cpu"] = cpu
                jSysStats["Data"]["mem"] = mem
                jSysStats["Data"]["host"] = host
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps(jSysStats))
                return
            return
        except IOError:
            self.send_error(404, 'Invalid extension. Only *.json supported: %s' % self.path)