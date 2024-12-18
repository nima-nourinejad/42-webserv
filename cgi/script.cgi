#!/usr/bin/env python3

import cgi

# Create an instance of FieldStorage to parse the form data
form = cgi.FieldStorage()

# Get a value from the query string or form data
name = form.getvalue("name", "World")  # Default to "World" if no name is provided

# Print a greeting message
print(f"From CGI: Hello, {name}!")

