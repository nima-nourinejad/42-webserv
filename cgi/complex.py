#!/usr/bin/env python3

import os
import cgi
import cgitb
import json
import sys

# Enable debugging
cgitb.enable()

import time

def wait_and_print():
    time.sleep(8)
    print("This is printed after 8 seconds!")

# Call the function
wait_and_print()

# HTML Template
HTML_TEMPLATE = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Python CGI Test</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            margin: 20px;
            line-height: 1.6;
        }}
        pre {{
            background: #f4f4f4;
            padding: 10px;
            border: 1px solid #ddd;
            overflow: auto;
        }}
    </style>
</head>
<body>
    <h1>Python CGI Test Page</h1>
    <p><strong>Method:</strong> {method}</p>
    <p><strong>Query String:</strong> {query_string}</p>
    <p><strong>Environment Variables:</strong></p>
    <pre>{env_vars}</pre>
    <p><strong>Form Data:</strong></p>
    <pre>{form_data}</pre>
    <p><strong>Uploaded File Info:</strong></p>
    <pre>{file_info}</pre>
    <p><strong>JSON Output:</strong></p>
    <pre>{json_output}</pre>
</body>
</html>
"""

# Collect CGI environment variables
env_vars = json.dumps({k: v for k, v in os.environ.items()}, indent=4)

# Parse query string and form data
form = cgi.FieldStorage()
query_string = os.environ.get("QUERY_STRING", "")
form_data = {key: form.getvalue(key) for key in form.keys()}

# Handle file uploads
file_info = {}
if "file" in form:
    file_item = form["file"]
    if file_item.filename:
        file_data = file_item.file.read()
        file_size = len(file_data)
        file_info = {
            "filename": file_item.filename,
            "size": file_size,
            "first_100_bytes": file_data[:100].decode(errors="ignore"),
        }

# Handle different HTTP methods
method = os.environ.get("REQUEST_METHOD", "UNKNOWN")
json_output = {}

if method == "POST":
    json_output = {
        "status": "success",
        "message": "POST request received",
        "form_data": form_data,
        "file_info": file_info,
    }
elif method == "GET":
    json_output = {
        "status": "success",
        "message": "GET request received",
        "query_string": query_string,
    }
else:
    json_output = {"status": "error", "message": f"Unsupported method: {method}"}

# Render HTML
print(
    HTML_TEMPLATE.format(
        method=method,
        query_string=query_string,
        env_vars=env_vars,
        form_data=json.dumps(form_data, indent=4),
        file_info=json.dumps(file_info, indent=4),
        json_output=json.dumps(json_output, indent=4),
    )
)
