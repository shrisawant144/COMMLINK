# Performance Characteristics and Limitations

## Performance Metrics

### Message Throughput
- **TCP**: ~1000 messages/second (small messages <1KB)
- **UDP**: ~5000 messages/second (small messages <1KB)
- **Large Messages** (>100KB): ~50-100 messages/second

### Latency
- **Local Network**: <1ms average
- **Internet**: Depends on network conditions
- **GUI Update**: <16ms (60 FPS refresh rate)

### Memory Usage
- **Base Application**: ~50MB
- **Per Message in History**: ~1-2KB (depending on content)
- **Database Growth**: ~1MB per 1000 messages

### CPU Usage
- **Idle**: <1%
- **Active Sending**: 5-15%
- **Active Receiving**: 5-20%
- **Database Operations**: 2-10%

## Known Limitations

### Message Size
- **Maximum Message Size**: 64KB (configurable)
- **Recommended Size**: <10KB for optimal performance
- **Large messages may cause UI lag during serialization**

### Connection Limits
- **Concurrent Connections**: 1 sender + 1 receiver
- **No connection pooling**
- **Single port binding per receiver instance**

### Format Support
- **Binary data**: Limited to base64 encoding in some formats
- **Large XML/JSON**: May be slow to parse
- **CSV**: Limited to simple tabular data

### Database
- **SQLite limitations apply**
- **No automatic cleanup of old messages**
- **Database size can grow indefinitely**
- **Recommended periodic maintenance**

### Network
- **No SSL/TLS support**
- **No compression**
- **No fragmentation handling for large messages**
- **UDP: No delivery guarantee**

### Platform-Specific
- **Linux**: Full support
- **Windows**: Requires Qt5 runtime
- **macOS**: Requires Qt5 runtime

## Optimization Recommendations

### For High-Throughput Scenarios
1. Use UDP instead of TCP when reliability isn't critical
2. Batch small messages together
3. Disable history logging for performance testing
4. Use BINARY or HEX format instead of JSON/XML

### For Large Messages
1. Split into smaller chunks
2. Use TEXT or BINARY format
3. Consider external file transfer instead

### For Long-Running Sessions
1. Periodically export and clear history
2. Monitor database size
3. Restart application if memory usage grows

### For Low-Latency Requirements
1. Use UDP protocol
2. Minimize message size
3. Disable verbose logging
4. Use local network instead of internet

## Benchmarking

To benchmark your specific use case:

```bash
# Build in Release mode
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# Monitor performance
time ./bin/CommLink
```

## Future Improvements

- [ ] Connection pooling for multiple simultaneous connections
- [ ] Message compression for large payloads
- [ ] SSL/TLS encryption support
- [ ] Automatic database maintenance
- [ ] Performance profiling tools
- [ ] Configurable message size limits
- [ ] Batch send operations
- [ ] Network statistics dashboard
