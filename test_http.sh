#!/bin/bash
# Quick HTTP Test Script for CommLink

echo "╔════════════════════════════════════════╗"
echo "║  CommLink HTTP Server Test Script     ║"
echo "╚════════════════════════════════════════╝"
echo ""
echo "Prerequisites:"
echo "  1. Start CommLink: ./build/bin/commlink"
echo "  2. Select Server Protocol: HTTP"
echo "  3. Enter Port: 8080"
echo "  4. Click 'Start Server'"
echo ""
read -p "Press Enter when server is ready..."
echo ""

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Test 1: HTTP POST with JSON"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
curl -X POST http://localhost:8080/api/test \
  -H "Content-Type: application/json" \
  -d '{"message":"Hello from test script","value":123,"timestamp":"'$(date +%s)'"}'
echo -e "\n"

sleep 1

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Test 2: HTTP GET"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
curl http://localhost:8080/api/data
echo -e "\n"

sleep 1

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Test 3: HTTP PUT"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
curl -X PUT http://localhost:8080/api/update \
  -H "Content-Type: application/json" \
  -d '{"id":1,"status":"updated","name":"Test User"}'
echo -e "\n"

sleep 1

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Test 4: HTTP DELETE"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
curl -X DELETE http://localhost:8080/api/delete/1
echo -e "\n"

sleep 1

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Test 5: HTTP PATCH"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
curl -X PATCH http://localhost:8080/api/patch \
  -H "Content-Type: application/json" \
  -d '{"field":"value"}'
echo -e "\n"

echo ""
echo "╔════════════════════════════════════════╗"
echo "║  Tests Complete!                       ║"
echo "╚════════════════════════════════════════╝"
echo ""
echo "Check CommLink GUI:"
echo "  ✓ Received Messages tab - Should show 5 messages"
echo "  ✓ Logs tab - Should show [RECV] entries"
echo "  ✓ History tab - Should have HTTP entries"
echo ""
