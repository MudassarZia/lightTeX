#include "lsp/JsonRpc.h"

#include <gtest/gtest.h>

using namespace lighttex::lsp;

TEST(JsonRpcTest, EncodeRequest) {
    QByteArray data = encodeRequest(1, "initialize");
    EXPECT_TRUE(data.contains("Content-Length:"));
    EXPECT_TRUE(data.contains("\"jsonrpc\":\"2.0\""));
    EXPECT_TRUE(data.contains("\"id\":1"));
    EXPECT_TRUE(data.contains("\"method\":\"initialize\""));
}

TEST(JsonRpcTest, EncodeRequestWithParams) {
    QJsonObject params;
    params["rootUri"] = "file:///tmp";
    QByteArray data = encodeRequest(42, "textDocument/completion", params);
    EXPECT_TRUE(data.contains("\"id\":42"));
    EXPECT_TRUE(data.contains("\"rootUri\":\"file:///tmp\""));
}

TEST(JsonRpcTest, EncodeNotification) {
    QByteArray data = encodeNotification("initialized");
    EXPECT_TRUE(data.contains("\"method\":\"initialized\""));
    EXPECT_FALSE(data.contains("\"id\""));
}

TEST(JsonRpcTest, MessageParserSingleMessage) {
    MessageParser parser;

    QJsonObject msg;
    msg["jsonrpc"] = "2.0";
    msg["id"] = 1;
    msg["result"] = QJsonObject{{"key", "value"}};

    QByteArray encoded = encodeMessage(msg);
    parser.feed(encoded);

    auto result = parser.nextMessage();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->value("id").toInt(), 1);
    EXPECT_EQ(result->value("result").toObject().value("key").toString(),
              "value");
}

TEST(JsonRpcTest, MessageParserMultipleMessages) {
    MessageParser parser;

    QJsonObject msg1;
    msg1["jsonrpc"] = "2.0";
    msg1["id"] = 1;
    msg1["result"] = QJsonObject();

    QJsonObject msg2;
    msg2["jsonrpc"] = "2.0";
    msg2["id"] = 2;
    msg2["result"] = QJsonObject();

    parser.feed(encodeMessage(msg1) + encodeMessage(msg2));

    auto r1 = parser.nextMessage();
    ASSERT_TRUE(r1.has_value());
    EXPECT_EQ(r1->value("id").toInt(), 1);

    auto r2 = parser.nextMessage();
    ASSERT_TRUE(r2.has_value());
    EXPECT_EQ(r2->value("id").toInt(), 2);

    auto r3 = parser.nextMessage();
    EXPECT_FALSE(r3.has_value());
}

TEST(JsonRpcTest, MessageParserPartialData) {
    MessageParser parser;

    QJsonObject msg;
    msg["jsonrpc"] = "2.0";
    msg["id"] = 1;
    msg["result"] = QJsonObject();

    QByteArray encoded = encodeMessage(msg);

    // Feed first half
    parser.feed(encoded.left(encoded.size() / 2));
    EXPECT_FALSE(parser.nextMessage().has_value());

    // Feed second half
    parser.feed(encoded.mid(encoded.size() / 2));
    auto result = parser.nextMessage();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->value("id").toInt(), 1);
}
