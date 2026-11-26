#pragma once
#include <QByteArray>
#include <QList>
#include <QString>

class StreamFramer {
public:
    enum Mode { Raw, Newline, Delimiter, Length32 };
    struct Frame { QByteArray payload; QByteArray wire; };
    Mode mode = Raw;
    QByteArray delimiter = "\n";
    static constexpr int MaximumPayload = 8 * 1024 * 1024;
    QList<Frame> feed(const QByteArray &bytes);
    QByteArray encode(const QByteArray &payload) const;
    QString error;
    void reset() { buffer.clear(); error.clear(); }
private:
    QByteArray buffer;
};
