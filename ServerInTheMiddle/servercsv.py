import csv
from http.server import BaseHTTPRequestHandler, HTTPServer

class MyServer(BaseHTTPRequestHandler):
    
    def do_POST(self):
        content_type = self.headers['Content-Type']
        if content_type == 'text/csv':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            lines = post_data.decode('utf-8').splitlines()
            reader = csv.DictReader(lines)
            with open('data.csv', 'a') as f:
                writer = csv.DictWriter(f, fieldnames=reader.fieldnames)
                if f.tell() == 0:
                    writer.writeheader()
                for row in reader:
                    writer.writerow(row)
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            response = {'message': 'CSV file received and saved'}
            self.wfile.write(json.dumps(response).encode('utf-8'))
        else:
            self.send_response(400)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            response = {'message': 'Unsupported media type'}
            self.wfile.write(json.dumps(response).encode('utf-8'))

def run(server_class=HTTPServer, handler_class=MyServer, port=8000):
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    print(f'Server running on port {port}')
    httpd.serve_forever()

if __name__ == '__main__':
    run()
