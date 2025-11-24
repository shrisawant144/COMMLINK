#!/bin/bash

echo "Testing JsonSender application..."

# Start the application in background
cd /home/miko/qt_projects/JsonSender/build
./bin/JsonSender &
APP_PID=$!

# Give it time to start
sleep 2

# Check if process is running
if ps -p $APP_PID > /dev/null; then
    echo "✅ Application started successfully (PID: $APP_PID)"
    
    # Test with netcat if available
    if command -v nc >/dev/null 2>&1; then
        echo "Testing UDP connection..."
        echo '{"test":"message"}' | nc -u 127.0.0.1 5001 &
        sleep 1
        
        echo "Testing TCP connection..."
        echo '{"test":"message"}' | nc 127.0.0.1 5001 &
        sleep 1
    fi
    
    # Kill the application
    kill $APP_PID
    echo "✅ Test completed"
else
    echo "❌ Application failed to start"
fi