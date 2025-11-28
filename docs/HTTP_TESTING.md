# HTTP Testing Guide - Localhost

## Quick Test Setup

### Test 1: CommLink HTTP Server + curl Client

**Step 1: Start CommLink HTTP Server**
```bash
# Run CommLink
./build/bin/commlink
```

In CommLink GUI:
1. Go to "Server Configuration"
2. Select Protocol: **HTTP**
3. Enter Port: **8080**
4. Select Format: **JSON**
5. Click **Start Server**

**Step 2: Send Request with curl**
```bash
# POST request
curl -X POST http://localhost:8080/api/test \
  -H "Content-Type: application/json" \
  -d '{"message":"Hello from curl","value":123}'

# GET request
curl -X GET http://localhost:8080/api/data

# PUT request
curl -X PUT http://localhost:8080/api/update \
  -H "Content-Type: application/json" \
  -d '{"id":1,"status":"updated"}'

# DELETE request
curl -X DELETE http://localhost:8080/api/delete/1
```

**Expected Result:**
- CommLink shows received message in "Received Messages" tab
- Logs show: `[RECV] Received HTTP message from 127.0.0.1:xxxxx [POST /api/test]`
- curl receives JSON response: `{"status":"received","method":"POST","path":"/api/test"}`

---

### Test 2: CommLink HTTP Client + Python Server

**Step 1: Start Python HTTP Server**
```bash
# Create test server
cat > test_server.py << 'EOF'
from http.server import HTTPServer, BaseHTTPRequestHandler
import json

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        response = {"status": "success", "method": "GET", "message": "Hello from Python"}
        self.wfile.write(json.dumps(response).encode())
    
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        body = self.rfile.read(content_length)
        print(f"Received: {body.decode()}")
        
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        response = {"status": "received", "data": body.decode()}
        self.wfile.write(json.dumps(response).encode())

HTTPServer(('localhost', 8080), Handler).serve_forever()
EOF

# Run server
python3 test_server.py
```

**Step 2: Use CommLink HTTP Client**

In CommLink GUI:
1. Go to "Client Configuration"
2. Select Protocol: **HTTP**
3. Select Method: **POST**
4. Enter URL: **http://localhost:8080/api**
5. Select Format: **JSON**
6. Enter Message: `{"test":"data","number":42}`
7. Click **Connect** then **Send**

**Expected Result:**
- Python server prints: `Received: {"test":"data","number":42}`
- CommLink shows response in "Received Messages"
- Logs show: `[HTTP] HTTP POST request sent to http://localhost:8080/api`

---

### Test 3: CommLink to CommLink (Two Instances)

**Terminal 1: Server Instance**
```bash
./build/bin/commlink
```
- Server Protocol: HTTP
- Port: 8080
- Format: JSON
- Start Server

**Terminal 2: Client Instance**
```bash
./build/bin/commlink
```
- Client Protocol: HTTP
- Method: POST
- URL: http://localhost:8080/test
- Format: JSON
- Message: `{"from":"client","data":"test"}`
- Connect → Send

**Expected Result:**
- Server instance shows received message
- Client instance shows response
- Both instances log the transaction

---

## Testing All HTTP Methods

### Using curl

```bash
# GET
curl http://localhost:8080/users

# POST
curl -X POST http://localhost:8080/users \
  -H "Content-Type: application/json" \
  -d '{"name":"John","age":30}'

# PUT
curl -X PUT http://localhost:8080/users/1 \
  -H "Content-Type: application/json" \
  -d '{"name":"John","age":31}'

# DELETE
curl -X DELETE http://localhost:8080/users/1

# PATCH
curl -X PATCH http://localhost:8080/users/1 \
  -H "Content-Type: application/json" \
  -d '{"age":32}'

# HEAD
curl -I http://localhost:8080/users

# OPTIONS
curl -X OPTIONS http://localhost:8080/users
```

### Using CommLink Client

Test each method by changing the dropdown:
1. GET - No body needed
2. POST - Add JSON body
3. PUT - Add JSON body
4. DELETE - Optional body
5. PATCH - Add JSON body
6. HEAD - No body needed
7. OPTIONS - No body needed

---

## Testing Different Data Formats

### JSON (Default)
```bash
curl -X POST http://localhost:8080/json \
  -H "Content-Type: application/json" \
  -d '{"key":"value"}'
```

### XML
```bash
curl -X POST http://localhost:8080/xml \
  -H "Content-Type: application/xml" \
  -d '<root><key>value</key></root>'
```

### Plain Text
```bash
curl -X POST http://localhost:8080/text \
  -H "Content-Type: text/plain" \
  -d 'Hello World'
```

### CSV
```bash
curl -X POST http://localhost:8080/csv \
  -H "Content-Type: text/csv" \
  -d 'name,age,city
John,30,NYC'
```

---

## Verify Logging

Check CommLink logs for:
```
[INFO] Started server on port 8080 via HTTP
[HTTP] HTTP POST request sent to http://localhost:8080/api
[HTTP-SEND] Sent via HTTP POST: {"data":"test"}
[RECV] Received ... from http://localhost:8080/api [HTTP 200]
```

---

## Verify History

1. Click "History" tab in CommLink
2. Filter by Protocol: HTTP
3. Check entries show:
   - Timestamp
   - Protocol: HTTP-POST, HTTP-GET, etc.
   - Direction: sent/received
   - Source: URL or IP:Port
   - Message content

---

## Verify Export

1. Click "Export Messages"
2. Choose format (TXT/CSV/JSON)
3. Save file
4. Open and verify HTTP messages are included

---

## Quick Test Script

```bash
#!/bin/bash
# test_http.sh

echo "Testing CommLink HTTP Server..."
echo "Make sure CommLink HTTP server is running on port 8080"
echo ""

echo "Test 1: POST request"
curl -X POST http://localhost:8080/test \
  -H "Content-Type: application/json" \
  -d '{"test":1}' && echo ""

echo "Test 2: GET request"
curl http://localhost:8080/data && echo ""

echo "Test 3: PUT request"
curl -X PUT http://localhost:8080/update \
  -H "Content-Type: application/json" \
  -d '{"test":2}' && echo ""

echo "Test 4: DELETE request"
curl -X DELETE http://localhost:8080/delete && echo ""

echo ""
echo "Check CommLink GUI for received messages!"
```

Save and run:
```bash
chmod +x test_http.sh
./test_http.sh
```

---

## Troubleshooting

### Port Already in Use
```bash
# Check what's using port 8080
sudo lsof -i :8080

# Use different port
# In CommLink: Enter 8081 instead
```

### Connection Refused
- Verify server is started in CommLink
- Check firewall: `sudo ufw allow 8080`
- Try 127.0.0.1 instead of localhost

### No Response
- Check CommLink logs for errors
- Verify correct URL format
- Ensure server is listening

---

## Success Indicators

✅ Server shows "Listening (HTTP)" status
✅ Client shows "Connected (HTTP)" or "Ready" status
✅ Logs show request/response messages
✅ Received Messages tab shows incoming data
✅ History tab records all transactions
✅ Export includes HTTP messages
✅ curl/Python receives JSON responses

---

## Next Steps

1. Test with real APIs (after localhost works)
2. Try HTTPS endpoints
3. Add custom headers
4. Test timeout scenarios
5. Load test with multiple clients
