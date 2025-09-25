#!/usr/bin/env python3

import os

print("Content-Type: text/html")
print("")

print("""
<!DOCTYPE html>
<html>
<head>
    <title>Server Information Test</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .info { background-color: #f0f0f0; padding: 20px; border-radius: 5px; margin: 10px 0; }
        .label { font-weight: bold; color: #333; }
        .value { color: #0066cc; }
    </style>
</head>
<body>
    <h1>Server Information Test</h1>
    <p>This CGI script shows the server information passed from the webserver:</p>

    <div class="info">
        <div class="label">Server Name:</div>
        <div class="value">""" + os.environ.get('SERVER_NAME', 'Not set') + """</div>
    </div>

    <div class="info">
        <div class="label">Server Port:</div>
        <div class="value">""" + os.environ.get('SERVER_PORT', 'Not set') + """</div>
    </div>

    <div class="info">
        <div class="label">Server Software:</div>
        <div class="value">""" + os.environ.get('SERVER_SOFTWARE', 'Not set') + """</div>
    </div>

    <div class="info">
        <div class="label">Request Method:</div>
        <div class="value">""" + os.environ.get('REQUEST_METHOD', 'Not set') + """</div>
    </div>

    <div class="info">
        <div class="label">Script Name:</div>
        <div class="value">""" + os.environ.get('SCRIPT_NAME', 'Not set') + """</div>
    </div>

    <p><a href="/">Back to Home</a></p>
</body>
</html>
""")
