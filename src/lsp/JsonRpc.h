#pragma once

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <optional>
#include <string>

namespace lighttex::lsp {

// Header-only Content-Length framing for JSON-RPC over stdio

inline QByteArray encodeMessage(const QJsonObject& msg) {
    QJsonDocument doc(msg);
    QByteArray content = doc.toJson(QJsonDocument::Compact);
    QByteArray header = "Content-Length: " +
                        QByteArray::number(content.size()) + "\r\n\r\n";
    return header + content;
}

inline QByteArray encodeRequest(int id, const QString& method,
                                 const QJsonObject& params = {}) {
    QJsonObject msg;
    msg["jsonrpc"] = "2.0";
    msg["id"] = id;
    msg["method"] = method;
    if (!params.isEmpty()) {
        msg["params"] = params;
    }
    return encodeMessage(msg);
}

inline QByteArray encodeNotification(const QString& method,
                                      const QJsonObject& params = {}) {
    QJsonObject msg;
    msg["jsonrpc"] = "2.0";
    msg["method"] = method;
    if (!params.isEmpty()) {
        msg["params"] = params;
    }
    return encodeMessage(msg);
}

// Buffer-based message parser
class MessageParser {
public:
    void feed(const QByteArray& data) {
        buffer_.append(data);
    }

    std::optional<QJsonObject> nextMessage() {
        while (true) {
            if (expectedLength_ < 0) {
                // Look for Content-Length header
                int headerEnd = buffer_.indexOf("\r\n\r\n");
                if (headerEnd < 0) return std::nullopt;

                QByteArray header = buffer_.left(headerEnd);
                int clIdx = header.indexOf("Content-Length: ");
                if (clIdx < 0) {
                    buffer_.remove(0, headerEnd + 4);
                    continue;
                }

                int valStart = clIdx + 16; // length of "Content-Length: "
                int valEnd = header.indexOf("\r\n", valStart);
                if (valEnd < 0) valEnd = header.size();

                bool ok = false;
                expectedLength_ = header.mid(valStart, valEnd - valStart)
                                      .trimmed().toInt(&ok);
                if (!ok || expectedLength_ <= 0) {
                    buffer_.remove(0, headerEnd + 4);
                    expectedLength_ = -1;
                    continue;
                }

                buffer_.remove(0, headerEnd + 4);
            }

            if (buffer_.size() < expectedLength_) return std::nullopt;

            QByteArray content = buffer_.left(expectedLength_);
            buffer_.remove(0, expectedLength_);
            expectedLength_ = -1;

            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(content, &err);
            if (doc.isObject()) {
                return doc.object();
            }
        }
    }

private:
    QByteArray buffer_;
    int expectedLength_ = -1;
};

} // namespace lighttex::lsp
