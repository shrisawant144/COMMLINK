#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>
#include "../core/dataformat.h"

class UdpServer : public QObject {
    Q_OBJECT
public:
    explicit UdpServer(QObject *parent = nullptr);
    
    bool listen(quint16 port);
    void close();
    bool isListening() const { return m_listening; }
    void setFormat(DataFormatType format) { m_format = format; }

signals:
    void messageReceived(const DataMessage& message, const QString& source, const QString& timestamp);
    void errorOccurred(const QString& error);

private slots:
    void onReadyRead();

private:
    QUdpSocket *m_socket;
    bool m_listening;
    DataFormatType m_format;
};

#endif
