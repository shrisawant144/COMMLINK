#!/usr/bin/env python3
"""
Simple HTTP test server for CommLink HTTP client testing
Usage: python3 test_server.py
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import json
from datetime import datetime

class CommLinkTestHandler(BaseHTTPRequestHandler):
    
    def log_request_info(self, method):
        print(f"\n{'='*50}")
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}]")
        print(f"Method: {method}")
        print(f"Path: {self.path}")
        print(f"Headers: {dict(self.headers)}")
        
    def send_json_response(self, status_code, data):
        self.send_response(status_code)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(data, indent=2).encode())
    
    def do_GET(self):
        self.log_request_info('GET')
        response = {
            "status": "success",
            "method": "GET",
            "path": self.path,
            "message": "Hello from Python test server",
            "timestamp": datetime.now().isoformat()
        }
        self.send_json_response(200, response)
        print(f"Response: {response}")
    
    def do_POST(self):
        self.log_request_info('POST')
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length).decode('utf-8')
        print(f"Body: {body}")
        
        try:
            body_json = json.loads(body) if body else {}
        except:
            body_json = {"raw": body}
        
        response = {
            "status": "received",
            "method": "POST",
            "path": self.path,
            "received_data": body_json,
            "timestamp": datetime.now().isoformat()
        }
        self.send_json_response(200, response)
        print(f"Response: {response}")
    
    def do_PUT(self):
        self.log_request_info('PUT')
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length).decode('utf-8')
        print(f"Body: {body}")
        
        response = {
            "status": "updated",
            "method": "PUT",
            "path": self.path,
            "timestamp": datetime.now().isoformat()
        }
        self.send_json_response(200, response)
        print(f"Response: {response}")
    
    def do_DELETE(self):
        self.log_request_info('DELETE')
        response = {
            "status": "deleted",
            "method": "DELETE",
            "path": self.path,
            "timestamp": datetime.now().isoformat()
        }
        self.send_json_response(200, response)
        print(f"Response: {response}")
    
    def do_PATCH(self):
        self.log_request_info('PATCH')
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length).decode('utf-8')
        print(f"Body: {body}")
        
        response = {
            "status": "patched",
            "method": "PATCH",
            "path": self.path,
            "timestamp": datetime.now().isoformat()
        }
        self.send_json_response(200, response)
        print(f"Response: {response}")
    
    def do_OPTIONS(self):
        self.log_request_info('OPTIONS')
        self.send_response(200)
        self.send_header('Allow', 'GET, POST, PUT, DELETE, PATCH, OPTIONS')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, PATCH, OPTIONS')
        self.end_headers()

if __name__ == '__main__':
    port = 8080
    server = HTTPServer(('localhost', port), CommLinkTestHandler)
    print(f"""
╔════════════════════════════════════════╗
║  CommLink HTTP Test Server             ║
╚════════════════════════════════════════╝

Server running on: http://localhost:{port}

Test with CommLink:
  1. Open CommLink
  2. Select Protocol: HTTP
  3. Select Method: POST (or any method)
  4. Enter URL: http://localhost:{port}/api/test
  5. Enter JSON message
  6. Click Connect → Send

Press Ctrl+C to stop server
{'='*50}
""")
    
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n\nServer stopped.")
        server.shutdown()
