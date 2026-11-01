#!/usr/bin/env python3
"""Web de teste: serve um updatelist próprio e repassa o resto ao web do jogo que já está no ar.

uso: serve_updatelist.py <updatelist> [--port 30092] [--upstream http://localhost:30091]
Escuta só em 127.0.0.1. O cliente usa este web quando JP983_WEB aponta para ele.
"""
import argparse
import http.server
import urllib.request


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('updatelist')
    ap.add_argument('--port', type=int, default=30092)
    ap.add_argument('--upstream', default='http://localhost:30091')
    args = ap.parse_args()

    class Handler(http.server.BaseHTTPRequestHandler):
        def do_GET(self):
            if self.path.split('?')[0].endswith('/patch/updatelist'):
                with open(args.updatelist, 'rb') as f:
                    body, code = f.read(), 200
            else:
                try:
                    r = urllib.request.urlopen(args.upstream + self.path, timeout=10)
                    body, code = r.read(), r.status
                except OSError:
                    body, code = b'', 404
            self.send_response(code)
            self.send_header('Content-Type', 'application/octet-stream')
            self.send_header('Content-Length', str(len(body)))
            self.send_header('Connection', 'close')
            self.end_headers()
            self.wfile.write(body)

        do_HEAD = do_GET

        def log_message(self, *a):
            pass

    http.server.ThreadingHTTPServer(('127.0.0.1', args.port), Handler).serve_forever()


if __name__ == '__main__':
    main()
