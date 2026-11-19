import http.cookies
import json
import urllib.error
import urllib.request


class Client:
    def __init__(self, base):
        self.base = base
        self.cookies = {}

    def call(self, path, data=None, expected=200, headers=None):
        request_headers = {"Cookie": "; ".join(f"{k}={v}" for k, v in self.cookies.items())}
        body = None
        if data is not None:
            body = json.dumps(data).encode()
            request_headers["Content-Type"] = "application/json"
        request_headers.update(headers or {})
        request = urllib.request.Request(self.base + "/api/" + path, body, request_headers)
        try:
            reply = urllib.request.urlopen(request, timeout=8)
        except urllib.error.HTTPError as error:
            reply = error
        with reply:
            assert reply.status == expected, (path, expected, reply.status)
            for value in reply.headers.get_all("Set-Cookie", []):
                for name, cookie in http.cookies.SimpleCookie(value).items():
                    assert cookie["httponly"] and cookie["secure"]
                    assert cookie["samesite"] == "Strict" and cookie["path"] == "/"
                    if cookie["max-age"] == "0":
                        self.cookies.pop(name, None)
                    else:
                        self.cookies[name] = cookie.value
            return json.load(reply) if reply.headers.get_content_type() == "application/json" else None
