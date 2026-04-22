#!/usr/bin/env python3


# version PC
# from http.server import HTTPServer, CGIHTTPRequestHandler
# import cgitb
# cgitb.enable()

# version RaspberryPi
import sys
sys.path.insert(0,'/usr/local/lib/python3.13/dist-packages')

from http.server import HTTPServer, CGIHTTPRequestHandler



server_address = ("", 8000)
handler = CGIHTTPRequestHandler
handler.cgi_directories = ["/cgi-bin"]

httpd = HTTPServer(server_address, handler)
httpd.serve_forever()