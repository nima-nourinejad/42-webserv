#!/usr/bin/env python3

# print("Content-Type: text/html")
# print()  # Empty line separates headers from the body
print("""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Simple CGI Test</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            line-height: 1.6;
        }
        h1 {
            color: #333;
        }
    </style>
</head>
<body>
    <h1>Simple CGI Test Page</h1>
    <p>This is a basic CGI script written in Python.</p>
    <p>If you can see this, your CGI setup is working correctly!</p>
</body>
</html>
""")
