# HTTP Protocol Support

CommLink provides complete HTTP/HTTPS client and server implementation with full integration into the application ecosystem.

## Features

### HTTP Client
- **Methods**: GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS
- **Protocols**: HTTP and HTTPS
- **Data Formats**: JSON, XML, CSV, TEXT, BINARY, HEX
- **Custom Headers**: Add custom HTTP headers
- **Timeout Control**: Configurable request timeout
- **Auto Content-Type**: Automatic content-type based on data format

### HTTP Server
- **RESTful API**: Handles all HTTP methods
- **Multi-client**: Supports multiple concurrent connections
- **Request Parsing**: Full HTTP request parsing (method, path, headers, body)
- **Auto Response**: Automatic JSON response generation
- **CORS Support**: Cross-Origin Resource Sharing enabled

## Client Usage

### GUI Configuration
1. Select "HTTP" from Protocol dropdown
2. Choose HTTP method (GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS)
3. Enter URL: `http://host:port/path` or `https://host:port/path`
4. Select data format (JSON, XML, CSV, etc.)
5. Enter message body in the message field
6. Click "Connect" to prepare client
7. Click "Send" to send request

### Example URLs
```
http://localhost:8080/api/data
https://api.example.com/v1/users
http://192.168.1.100:3000/endpoint
```

### Supported Methods

#### GET
Retrieve data from server. Body is optional.

#### POST
Send data to create new resource. Body required.

#### PUT
Update existing resource. Body required.

#### DELETE
Remove resource. Body optional.

#### PATCH
Partial update of resource. Body required.

#### HEAD
Retrieve headers only, no body.

#### OPTIONS
Query supported methods for endpoint.

## Server Usage

### Starting HTTP Server
1. Select "HTTP" from Server Protocol dropdown
2. Enter port number (e.g., 8080)
3. Select data format for parsing incoming requests
4. Click "Start Server"

### Server Behavior
- Listens on specified port
- Accepts all HTTP methods
- Parses request body based on selected format
- Logs all incoming requests with method and path
- Sends JSON response: `{"status":"received","method":"...","path":"..."}`
- Supports multiple concurrent clients

## Data Format Support

### JSON (Default)
```json
{"key": "value", "number": 123}
```
Content-Type: `application/json`

### XML
```xml
<root><key>value</key></root>
```
Content-Type: `application/xml`

### CSV
```csv
name,age,city
John,30,NYC
```
Content-Type: `text/csv`

### TEXT
```
Plain text message
```
Content-Type: `text/plain`

### BINARY
Raw binary data
Content-Type: `application/octet-stream`

### HEX
```
48656C6C6F
```
Content-Type: `text/plain`

## Logging Integration

All HTTP operations are logged:

### Client Logs
- `[HTTP] HTTP GET request sent to http://...`
- `[HTTP-SEND] Sent via HTTP POST: {...}`
- `[RECV] Received ... from http://... [HTTP 200]`
- `[ERROR] HTTP Error 404: Not Found`

### Server Logs
- `[INFO] Started server on port 8080 via HTTP`
- `[RECV] Received HTTP message from 192.168.1.5:54321 [POST /api/data]`
- `[INFO] Stopped server`

## History Integration

All HTTP messages are saved to database:

### Client History
- Direction: "sent"
- Protocol: "HTTP-GET", "HTTP-POST", etc.
- Host: Full URL
- Port: 0 (not applicable)
- Message: Request body
- Timestamp: ISO format

### Server History
- Direction: "received"
- Protocol: "HTTP"
- Host: Client IP:Port
- Message: Request body with method and path
- Timestamp: ISO format

## Export Support

HTTP messages can be exported in all formats:
- TXT: Human-readable format with timestamps
- CSV: Structured data with protocol, direction, source
- JSON: Complete message data with metadata

## Custom Headers

Programmatic API for custom headers:
```cpp
httpClient->addHeader("Authorization", "Bearer token123");
httpClient->addHeader("X-Custom-Header", "value");
httpClient->sendRequest(url, HttpClient::POST, message);
httpClient->clearHeaders();
```

## Timeout Configuration

Default timeout: 30 seconds
```cpp
httpClient->setTimeout(60000); // 60 seconds
```

## Status Codes

HTTP responses include status code in source field:
- `http://example.com/api [HTTP 200]` - Success
- `http://example.com/api [HTTP 404]` - Not Found
- `http://example.com/api [HTTP 500]` - Server Error

## Security

### HTTPS Support
- Full SSL/TLS support via Qt Network
- Certificate validation
- Secure connections to HTTPS endpoints

### Best Practices
- Use HTTPS for sensitive data
- Validate server certificates
- Implement authentication headers
- Use appropriate HTTP methods
- Handle errors gracefully

## Testing

### Test HTTP Client
```bash
# Start CommLink HTTP server on port 8080
# Use curl to test:
curl -X POST http://localhost:8080/test \
  -H "Content-Type: application/json" \
  -d '{"message":"Hello from curl"}'
```

### Test HTTP Server
```bash
# Start external HTTP server
# Use CommLink HTTP client to send requests
# Monitor logs and received messages
```

## Troubleshooting

### Connection Refused
- Check if server is running
- Verify correct host and port
- Check firewall settings

### Timeout Errors
- Increase timeout value
- Check network connectivity
- Verify server is responding

### Parse Errors
- Ensure data format matches content
- Validate JSON/XML syntax
- Check content-type header

## API Reference

### HttpClient Class
```cpp
enum Method { GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS };

void sendRequest(const QString& url, Method method, const DataMessage& message);
void setFormat(DataFormatType format);
void setMethod(Method method);
void addHeader(const QString& key, const QString& value);
void clearHeaders();
void setTimeout(int msecs);
bool isConnected() const;
void disconnect();

signals:
void responseReceived(const DataMessage& message, const QString& source, const QString& timestamp);
void errorOccurred(const QString& error);
void requestSent(const QString& method, const QString& url);
```

### HttpServer Class
```cpp
bool startServer(quint16 port);
void stopServer();
bool isListening() const;
void setFormat(DataFormatType format);

signals:
void clientConnected(const QString& clientInfo);
void clientDisconnected(const QString& clientInfo);
void messageReceived(const DataMessage& message, const QString& source, const QString& timestamp);
void errorOccurred(const QString& error);
```
