#!/bin/bash
# Script to kill process using a specific port

if [ $# -eq 0 ]; then
    echo "Usage: $0 <port_number>"
    echo "Example: $0 5000"
    exit 1
fi

PORT=$1

echo "Checking for processes using port $PORT..."

# Find and kill process using the port
PID=$(lsof -ti:$PORT)

if [ -z "$PID" ]; then
    echo "No process found using port $PORT"
    exit 0
fi

echo "Found process $PID using port $PORT"
echo "Killing process..."

kill -9 $PID

if [ $? -eq 0 ]; then
    echo "Successfully killed process $PID"
    echo "Port $PORT is now free"
else
    echo "Failed to kill process. You may need sudo privileges."
    echo "Try: sudo $0 $PORT"
    exit 1
fi
