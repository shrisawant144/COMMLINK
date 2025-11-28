#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include "../core/dataformat.h"

class UdpClient : public QObject {
    Q_OBJECT
public:
    explicit UdpClient(QObject *parent = nullptr);
    
    bool connectToHost(const QString& host, quint16 port);
    void disconnect();
    void sendMessage(const DataMessage& message);
    bool isConnected() const { return m_connected; }
    void setFormat(DataFormatType format) { m_format = format; }

signals:
    void connected();
    void disconnected();
    void messageReceived(const DataMessage& message, const QString& source, const QString& timestamp);
    void errorOccurred(const QString& error);

private slots:
    void onReadyRead();

private:
    QUdpSocket *m_socket;
    QHostAddress m_host;
    quint16 m_port;
    bool m_connected;
    DataFormatType m_format;
};

#endif
