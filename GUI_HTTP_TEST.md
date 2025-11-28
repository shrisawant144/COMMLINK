# HTTP Testing - Current GUI Layout

## 📍 GUI Layout Overview

```
┌─────────────────────────────────────────────────────────────────┐
│ CommLink - Network Communication Tool                           │
├─────────────────────────────────────────────────────────────────┤
│ [View] Menu → Appearance (Light/Dark/System)                    │
├──────────────────────────┬──────────────────────────────────────┤
│  LEFT PANEL              │  RIGHT PANEL                         │
│                          │                                      │
│ ┌──────────────────────┐ │ ┌──────────────────────────────────┐│
│ │ Connection Status    │ │ │ [Message] [Received] [History]   ││
│ │ Client: Disconnected │ │ │ [Logs]                           ││
│ │ Server: Stopped      │ │ │                                  ││
│ └──────────────────────┘ │ │  Message Input Area              ││
│                          │ │  (Enter JSON/XML/CSV/Text here)  ││
│ ┌──────────────────────┐ │ │                                  ││
│ │ Client Configuration │ │ │                                  ││
│ │ Protocol: [HTTP ▼]   │ │ │                                  ││
│ │ HTTP Method:[POST▼]  │ │ │  [Load] [Save] [Send]            ││
│ │ Host: localhost      │ │ └──────────────────────────────────┘│
│ │ Port: 8080           │ │                                      │
│ │ [Connect]            │ │                                      │
│ └──────────────────────┘ │                                      │
│                          │                                      │
│ ┌──────────────────────┐ │                                      │
│ │ Server Configuration │ │                                      │
│ │ Protocol: [HTTP ▼]   │ │                                      │
│ │ Port: 8080           │ │                                      │
│ │ [Start Server]       │ │                                      │
│ │ [Stop Server]        │ │                                      │
│ └──────────────────────┘ │                                      │
│                          │                                      │
│ ┌──────────────────────┐ │                                      │
│ │ Data Format: [JSON▼] │ │                                      │
│ └──────────────────────┘ │                                      │
└──────────────────────────┴──────────────────────────────────────┘
```

---

## 🎯 Step-by-Step: Same Device HTTP Test

### **STEP 1: Start HTTP Server**

**Location:** LEFT PANEL → "Server Configuration" section

1. Find **"Server Configuration"** group box (bottom left)
2. Click **Protocol dropdown** → Select **"HTTP"**
3. **Port field** → Type **8080**
4. Click **"Start Server"** button
5. ✅ **Connection Status** (top left) shows: **"Server: Listening (HTTP)"** in GREEN

---

### **STEP 2: Prepare HTTP Client**

**Location:** LEFT PANEL → "Client Configuration" section

1. Find **"Client Configuration"** group box (middle left)
2. Click **Protocol dropdown** → Select **"HTTP"**
3. 🆕 **HTTP Method dropdown appears** → Select **"POST"**
4. **Host field** → Clear and type: **http://localhost:8080/test**
5. Click **"Connect"** button
6. ✅ Button changes to **"Ready"**
7. ✅ **Connection Status** shows: **"Client: Connected (HTTP)"** in GREEN

---

### **STEP 3: Send Message**

**Location:** RIGHT PANEL → "Message" tab

1. Click **"Message"** tab (top right)
2. In the large text area, type:
   ```json
   {"message":"Hello from same device","test":true,"value":123}
   ```
3. **Data Format dropdown** (bottom left) → Ensure **"JSON"** is selected
4. Click **"Send"** button (bottom right)

---

### **STEP 4: View Results**

**Location:** RIGHT PANEL → Multiple tabs

#### **Received Messages Tab:**
1. Click **"Received Messages"** tab
2. You'll see:
   ```
   [2025-11-28 16:46:00] ← HTTP from 127.0.0.1:xxxxx [POST /test]:
   {"message":"Hello from same device","test":true,"value":123}
   ```

#### **Logs Tab:**
1. Click **"Logs"** tab
2. You'll see:
   ```
   [HTTP-SEND] Sent via HTTP POST: {"message":"Hello from same device"...}
   [RECV] Received HTTP message from 127.0.0.1:xxxxx [POST /test]
   ```

#### **History Tab:**
1. Click **"History"** tab
2. Shows database records:
   - Protocol: HTTP-POST
   - Direction: sent / received
   - Timestamp
   - Full message

---

## 🔄 Test Different HTTP Methods

In **Client Configuration** → **HTTP Method dropdown**, try:

- **GET** → No body needed, just click Send
- **POST** → Add JSON body, click Send
- **PUT** → Add JSON body, click Send
- **DELETE** → Optional body, click Send
- **PATCH** → Add JSON body, click Send
- **HEAD** → No body, click Send
- **OPTIONS** → No body, click Send

Each time:
1. Change method in dropdown
2. Click Send
3. Check "Received Messages" tab
4. See method in logs: `[POST /test]`, `[GET /test]`, etc.

---

## 📊 Visual Indicators

### Connection Status (Top Left):
```
✅ Client: Connected (HTTP)     [GREEN]
✅ Server: Listening (HTTP)     [GREEN]

❌ Client: Disconnected         [RED]
❌ Server: Stopped              [RED]
```

### Buttons:
```
[Connect]     → Changes to → [Ready]      (HTTP client)
[Disconnect]  → Changes to → [Connect]    (Other protocols)

[Start Server] → Disabled when running
[Stop Server]  → Enabled when running
```

---

## 🎨 Current GUI Features for HTTP

### Client Section Shows:
- ✅ Protocol dropdown with "HTTP" option
- ✅ HTTP Method dropdown (GET/POST/PUT/DELETE/PATCH/HEAD/OPTIONS)
- ✅ Host field accepts full URLs: `http://localhost:8080/path`
- ✅ Port field hidden when HTTP selected (port in URL)
- ✅ Connect button → "Ready" state for HTTP

### Server Section Shows:
- ✅ Protocol dropdown with "HTTP" option
- ✅ Port field for listening port
- ✅ Start/Stop server buttons
- ✅ Status indicator

### Right Panel Shows:
- ✅ Message tab - Input area
- ✅ Received Messages tab - Shows incoming HTTP requests
- ✅ History tab - Database records
- ✅ Logs tab - Detailed logging with [HTTP] tags

---

## 🧪 Quick Verification Checklist

After sending message, verify:

- [ ] **Connection Status** shows both Client and Server as connected/listening
- [ ] **Logs tab** shows `[HTTP-SEND]` and `[RECV]` entries
- [ ] **Received Messages tab** shows the message with timestamp
- [ ] **History tab** has entries for both sent and received
- [ ] Server response visible in Received Messages
- [ ] HTTP method visible in logs: `[POST /test]`
- [ ] Status code visible: `[HTTP 200]`

---

## 💡 Tips

1. **Same Window Testing:** You can run both server and client in ONE CommLink window
2. **URL Format:** Always use `http://localhost:PORT/path` format
3. **Port Numbers:** Server port and client URL port must match
4. **Method Selection:** HTTP Method dropdown only appears when HTTP protocol selected
5. **Real-time Updates:** All tabs update immediately when messages sent/received

---

## 🐛 Troubleshooting

**"Connection Refused"**
→ Make sure "Start Server" is clicked first

**"Port field not visible"**
→ Port field hides for HTTP client (port is in URL)
→ Port field shows for HTTP server (listening port)

**"HTTP Method dropdown not visible"**
→ Select "HTTP" in Protocol dropdown first

**"No response in Received Messages"**
→ Check Logs tab for errors
→ Verify URL is `http://localhost:8080/...`
→ Ensure server is started

---

## 📸 What You Should See

**Before Starting:**
```
Client: Disconnected [RED]
Server: Stopped [RED]
```

**After Starting Server:**
```
Client: Disconnected [RED]
Server: Listening (HTTP) [GREEN]
```

**After Connecting Client:**
```
Client: Connected (HTTP) [GREEN]
Server: Listening (HTTP) [GREEN]
```

**After Sending Message:**
```
Received Messages tab shows:
[2025-11-28 16:46:43] ← HTTP from 127.0.0.1:54321 [POST /test]:
{"message":"Hello from same device","test":true,"value":123}

Logs tab shows:
[HTTP-SEND] Sent via HTTP POST: {"message":"Hello from same device"...}
[RECV] Received HTTP message from 127.0.0.1:54321 [POST /test]
```

---

**Your CommLink GUI is ready! Just follow the steps above to test HTTP on the same device.**
