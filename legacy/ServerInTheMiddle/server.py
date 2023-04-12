# Server di test, capace di POST
import json
from http.server import BaseHTTPRequestHandler, HTTPServer


class MyServer(BaseHTTPRequestHandler):

    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        data = json.loads(post_data.decode('utf-8'))
        with open('data.json', 'a') as f:
            print(data)
            json.dump(data, f)
            f.write('\n')

        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        response = {'message': 'Data received and saved'}
        self.wfile.write(json.dumps(response).encode('utf-8'))


def run(server_class=HTTPServer, handler_class=MyServer, port=8000):
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    print(f'Server running on port {port}')
    httpd.serve_forever()


if __name__ == '__main__':
    run()
