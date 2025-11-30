#!/bin/bash
# This script starts a simple web server to serve the controller.html file.

# Get the directory of this script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# The port for the web server
PORT=8000

echo "Starting a simple web server..."
echo "You can access the controller at:"
echo "http://localhost:$PORT/controller.html"
echo ""
echo "Press Ctrl+C to stop the server."

# Check for python3 and run the server
if command -v python3 &> /dev/null
then
    python3 -m http.server $PORT --directory "$DIR"
elif command -v python &> /dev/null
then
    python -m SimpleHTTPServer $PORT
else
    echo "Error: Python is not installed. Please install Python to run the web server."
    exit 1
fi
