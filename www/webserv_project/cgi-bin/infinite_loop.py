#!/usr/bin/env python3
# Infinite loop CGI script for testing timeout functionality

import time

# Infinite loop BEFORE any output - this will cause the CGI to timeout
while True:
    time.sleep(0.1)

# This code should never be reached
print("Content-Type: text/html")
print("")
print("<html><body>")
print("<h1>This should never be seen!</h1>")
print("<p>If you see this, the timeout didn't work.</p>")
print("</body></html>")
