#include "commlink/core/streamframer.h"

#include <QtEndian>

QByteArray StreamFramer::encode(const QByteArray& payload) const {
    if (payload.size() > MaximumPayload)
        return {};
    if (mode == Length32) {
        QByteArray header(4, '\0');
        qToBigEndian<quint32>(static_cast<quint32>(payload.size()),
                              reinterpret_cast<uchar*>(header.data()));
        return header + payload;
    }
    if (mode == Newline)
        return payload + '\n';
    if (mode == Delimiter)
        return delimiter.isEmpty() || delimiter.size() > 32 ? QByteArray() : payload + delimiter;
    return payload;
}

QList<StreamFramer::Frame> StreamFramer::feed(const QByteArray& bytes, int maximumFrames) {
    QList<Frame> frames;
    error.clear();
    if (mode == Raw) {
        if (bytes.size() > MaximumPayload)
            error = "TCP chunk exceeds 8 MiB";
        else if (!bytes.isEmpty())
            frames.append({bytes, bytes});
        return frames;
    }
    buffer += bytes;
    const QByteArray separator = mode == Newline ? QByteArray("\n") : delimiter;
    maximumFrames = qBound(1, maximumFrames, 1024);
    while (!buffer.isEmpty() && frames.size() < maximumFrames) {
        int payloadSize = 0, wireSize = 0;
        if (mode == Length32) {
            if (buffer.size() < 4)
                break;
            const quint32 length =
                qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(buffer.constData()));
            if (length > MaximumPayload) {
                error = "TCP frame exceeds 8 MiB";
                break;
            }
            payloadSize = static_cast<int>(length);
            wireSize = payloadSize + 4;
            if (buffer.size() < wireSize)
                break;
            frames.append({buffer.mid(4, payloadSize), buffer.left(wireSize)});
        } else {
            if (separator.isEmpty() || separator.size() > 32) {
                error = "TCP delimiter cannot be empty";
                break;
            }
            payloadSize = buffer.indexOf(separator);
            if (payloadSize < 0)
                break;
            if (payloadSize > MaximumPayload) {
                error = "TCP frame exceeds 8 MiB";
                break;
            }
            wireSize = payloadSize + separator.size();
            frames.append({buffer.left(payloadSize), buffer.left(wireSize)});
        }
        buffer.remove(0, wireSize);
    }
    if (buffer.size() > MaximumPayload + (mode == Length32 ? 4 : separator.size()))
        error = "TCP receive buffer exceeds 8 MiB";
    if (!error.isEmpty())
        buffer.clear();
    return frames;
}
