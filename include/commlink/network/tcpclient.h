#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include "../core/dataformat.h"

/**
 * @brief TCP client for connection-oriented network communication
 * 
 * @section tcp_flow TCP Client Flow
 * 
 * @subsection connect_flow Connection Flow
 * 
 * 1. connectToHost(host, port) called
 * 2. QTcpSocket::connectToHost() called (asynchronous)
 * 3. Connection timer started (3 second timeout)
 * 4. When connected: onConnected() slot called
 * 5. connected() signal emitted
 * 6. When timeout: onConnectionTimeout() aborts connection
 * 
 * @subsection send_flow Send Message Flow
 * 
 * 1. sendMessage(DataMessage) called
 * 2. DataMessage::serialize() converts to QByteArray
 * 3. QTcpSocket::write() queues bytes for transmission
 * 4. QTcpSocket::flush() forces immediate send
 * 
 * @subsection receive_flow Receive Message Flow
 * 
 * 1. Data arrives on socket
 * 2. QTcpSocket::readyRead() signal emitted
 * 3. onReadyRead() slot called
 * 4. socket->readAll() reads all available bytes
 * 5. DataMessage::deserialize() converts bytes to DataMessage
 * 6. messageReceived() signal emitted with DataMessage
 * 
 * @note All operations are asynchronous and non-blocking
 */
class TcpClient : public QObject {
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    
    /**
     * @brief Connects to TCP server
     * 
     * @param host Server hostname or IP address
     * @param port Server port number
     * 
     * Flow:
     * 1. Calls QTcpSocket::connectToHost() (asynchronous)
     * 2. Starts 3-second connection timeout timer
     * 3. Returns immediately (non-blocking)
     * 4. When connected, onConnected() is called automatically
     * 5. connected() signal is emitted
     * 
     * @note Connection happens asynchronously in Qt's event loop
     */
    void connectToHost(const QString& host, quint16 port);
    
    /**
     * @brief Disconnects from server
     * 
     * Stops connection timer and closes socket
     */
    void disconnect();
    
    /**
     * @brief Sends a message to the connected server
     * 
     * @param message DataMessage to send
     * 
     * Flow:
     * 1. Serializes DataMessage to QByteArray using message.serialize()
     * 2. Writes bytes to socket using QTcpSocket::write()
     * 3. Flushes socket to ensure immediate transmission
     * 
     * @note write() is asynchronous and queues data
     * @note flush() forces immediate send
     */
    void sendMessage(const DataMessage& message);
    
    /**
     * @brief Checks if connected to server
     * @return true if connected, false otherwise
     */
    bool isConnected() const;
    
    /**
     * @brief Sets data format for serialization/deserialization
     * @param format Data format type (JSON, XML, CSV, etc.)
     */
    void setFormat(DataFormatType format) { m_format = format; }

signals:
    void connected();
    void disconnected();
    void messageReceived(const DataMessage& message, const QString& source, const QString& timestamp);
    void errorOccurred(const QString& error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onConnectionTimeout();

private:
    QTcpSocket *m_socket;
    QTimer *m_connectionTimer;
    DataFormatType m_format;
    bool m_connected;
    static const int CONNECTION_TIMEOUT_MS = 3000;
};

#endif
