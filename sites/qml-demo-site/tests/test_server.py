import importlib.util
import json
import tempfile
import threading
import unittest
from http.server import ThreadingHTTPServer
from pathlib import Path
from urllib.error import HTTPError
from urllib.request import Request, urlopen

spec = importlib.util.spec_from_file_location("qml_site", Path(__file__).parents[1] / "server.py")
site = importlib.util.module_from_spec(spec)
spec.loader.exec_module(site)


class SiteTest(unittest.TestCase):
    def test_content_negotiation_and_no_directory_exposure(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            for name in site.APPS:
                (root / f"com.qbrowser.demo.{name}-1.0.0.qapkg").write_bytes(b"test archive")
            (root / "private.pem").write_text("must never be served")
            server = ThreadingHTTPServer(("127.0.0.1", 0), site.create_handler(root))
            thread = threading.Thread(target=server.serve_forever, daemon=True)
            thread.start()
            origin = f"http://127.0.0.1:{server.server_port}"
            try:
                with urlopen(origin + "/demos/elisa") as response:
                    self.assertIn("text/html", response.headers["Content-Type"])
                    self.assertEqual(response.headers["Vary"], "Accept")
                with urlopen(Request(origin + "/demos/elisa", headers={"Accept": site.MIME})) as response:
                    descriptor = json.load(response)
                    self.assertEqual(descriptor["appId"], "com.qbrowser.demo.elisa")
                with urlopen(origin + descriptor["packageUrl"]) as response:
                    self.assertEqual(response.read(), b"test archive")
                for path in ("/packages/", "/private.pem", "/packages/../private.pem", "/server.py"):
                    with self.assertRaises(HTTPError) as error:
                        urlopen(origin + path)
                    self.assertEqual(error.exception.code, 404)
            finally:
                server.shutdown()
                server.server_close()
                thread.join()


if __name__ == "__main__":
    unittest.main()
