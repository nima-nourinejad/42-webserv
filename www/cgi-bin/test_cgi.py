#!/usr/bin/env python3
import os
import sys

# Read CONTENT_LENGTH to get the size of the request body
content_length = int(os.getenv('CONTENT_LENGTH', 0))
body = sys.stdin.read(content_length) if content_length > 0 else ""

print("Content-Type: text/plain")
print("")
print("Hello from CGI!")
print(f"Request Body: {body}")
print(f"Content-Length: {content_length}")
