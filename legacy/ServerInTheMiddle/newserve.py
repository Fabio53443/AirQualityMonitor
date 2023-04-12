from http.server import BaseHTTPRequestHandler, HTTPServer
import cgi

class RequestHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        ctype, pdict = cgi.parse_header(self.headers.get('content-type'))
        if ctype == 'multipart/form-data':
            pdict['boundary'] = bytes(pdict['boundary'], "utf-8")
            fields = cgi.parse_multipart(self.rfile, pdict)
            messagecontent = fields.get('message')
        else:
            length = int(self.headers.get('content-length'))
            messagecontent = self.rfile.read(length).decode("utf-8")
        self.send_response(200)
        self.end_headers()
        response = bytes("This is the POST request. Data received: " + str(messagecontent), "utf-8")
        self.wfile.write(response)

def run():
    server_address = ('', 9080)
    httpd = HTTPServer(server_address, RequestHandler)
    print('Starting server on port 9080...')
    httpd.serve_forever()

if __name__ == '__main__':
    run()
