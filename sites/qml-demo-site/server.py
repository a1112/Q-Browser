"""Standalone QML site. Python 3.10+, standard library only."""
import argparse
import hashlib
import json
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

APPS = {
    "elisa": ("Elisa 音乐", "专辑浏览、搜索与真实音频播放", "♫"),
    "tokodon": ("Tokodon 社区", "本地信息流、互动与模拟发帖", "◎"),
    "coffee": ("Coffee Machine", "调整配方，体验咖啡制作流程", "☕"),
}
MIME = "application/vnd.qbrowser.site+json"


def create_handler(package_root):
    # Snapshot public signed artifacts once, so descriptors and archives agree.
    archives = {}
    descriptors = {}
    for name in APPS:
        app_id = f"com.qbrowser.demo.{name}"
        filename = f"{app_id}-1.0.0.qapkg"
        data = (Path(package_root) / filename).read_bytes()
        if len(data) > 64 * 1024 * 1024:
            raise ValueError(f"Package too large: {filename}")
        archives[f"/packages/{filename}"] = data
        descriptors[f"/demos/{name}"] = json.dumps({
            "schemaVersion": 1, "appId": app_id, "route": f"/demos/{name}",
            "packageUrl": f"/packages/{filename}",
            "sha256": hashlib.sha256(data).hexdigest(),
        }).encode()
    html = (Path(__file__).parent / "index.html").read_bytes()

    class Handler(BaseHTTPRequestHandler):
        def do_GET(self):
            if self.path in archives:
                return self.send_content(archives[self.path], "application/octet-stream")
            if self.path in descriptors and MIME in self.headers.get("Accept", ""):
                return self.send_content(descriptors[self.path], MIME)
            if self.path == "/" or self.path in descriptors:
                return self.send_content(html, "text/html; charset=utf-8")
            self.send_error(404)

        def send_content(self, data, content_type):
            self.send_response(200)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(data)))
            self.send_header("Vary", "Accept")
            self.send_header("Cache-Control", "no-store")
            self.send_header("X-Content-Type-Options", "nosniff")
            self.send_header("Content-Security-Policy", "default-src 'none'; style-src 'unsafe-inline'; script-src 'unsafe-inline'")
            self.end_headers()
            self.wfile.write(data)

    return Handler


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--packages", required=True, type=Path, help="Directory of public signed .qapkg archives")
    parser.add_argument("--bind", default="127.0.0.1")
    parser.add_argument("--port", default=18880, type=int)
    args = parser.parse_args()
    server = ThreadingHTTPServer((args.bind, args.port), create_handler(args.packages))
    print(f"QML site: http://{args.bind}:{server.server_port}/", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
