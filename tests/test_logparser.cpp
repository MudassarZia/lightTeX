#include "compiler/LogParser.h"
#include <gtest/gtest.h>

#include <QFile>
#include <QTextStream>

using namespace lighttex::compiler;

static std::string readFixture(const std::string &name) {
  QFile file(QString(FIXTURE_DIR) + "/logs/" + QString::fromStdString(name));
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return "";
  QTextStream stream(&file);
  return stream.readAll().toStdString();
}

TEST(LogParser, ParseError) {
  std::string log = "./main.tex:10: ! Undefined control sequence.\n";
  auto messages = parseLog(log);
  ASSERT_EQ(messages.size(), 1u);
  EXPECT_EQ(messages[0].kind, MessageKind::Error);
  EXPECT_EQ(messages[0].message, "Undefined control sequence.");
  ASSERT_TRUE(messages[0].file.has_value());
  EXPECT_EQ(*messages[0].file, "./main.tex");
  ASSERT_TRUE(messages[0].line.has_value());
  EXPECT_EQ(*messages[0].line, 10);
}

TEST(LogParser, ParseErrorNoFile) {
  std::string log = "! Missing $ inserted.\n";
  auto messages = parseLog(log);
  ASSERT_EQ(messages.size(), 1u);
  EXPECT_EQ(messages[0].kind, MessageKind::Error);
  EXPECT_EQ(messages[0].message, "Missing $ inserted.");
  EXPECT_FALSE(messages[0].file.has_value());
}

TEST(LogParser, ParseWarning) {
  std::string log = "LaTeX Warning: Reference `fig:1' on page 1 undefined on "
                    "input line 15.\n";
  auto messages = parseLog(log);
  ASSERT_EQ(messages.size(), 1u);
  EXPECT_EQ(messages[0].kind, MessageKind::Warning);
  ASSERT_TRUE(messages[0].line.has_value());
  EXPECT_EQ(*messages[0].line, 15);
}

TEST(LogParser, ParsePackageWarning) {
  std::string log = "Package hyperref Warning: Token not allowed in a PDF "
                    "string on input line 20.\n";
  auto messages = parseLog(log);
  ASSERT_EQ(messages.size(), 1u);
  EXPECT_EQ(messages[0].kind, MessageKind::Warning);
  ASSERT_TRUE(messages[0].line.has_value());
  EXPECT_EQ(*messages[0].line, 20);
}

TEST(LogParser, ParseBadBox) {
  std::string log =
      "Overfull \\hbox (15.0pt too wide) in paragraph at lines 20--22\n";
  auto messages = parseLog(log);
  ASSERT_EQ(messages.size(), 1u);
  EXPECT_EQ(messages[0].kind, MessageKind::BadBox);
}

TEST(LogParser, ParseEmptyLog) {
  auto messages = parseLog("");
  EXPECT_TRUE(messages.empty());
}

TEST(LogParser, ParseSuccessLog) {
  std::string log = readFixture("success.log");
  if (log.empty())
    GTEST_SKIP() << "Fixture not found";
  auto messages = parseLog(log);
  // Success log should have no errors
  for (const auto &msg : messages) {
    EXPECT_NE(msg.kind, MessageKind::Error);
  }
}

TEST(LogParser, ParseMultipleErrors) {
  std::string log = readFixture("errors.log");
  if (log.empty())
    GTEST_SKIP() << "Fixture not found";
  auto messages = parseLog(log);
  // Should find multiple messages
  EXPECT_GE(messages.size(), 2u);

  int errorCount = 0;
  for (const auto &msg : messages) {
    if (msg.kind == MessageKind::Error)
      ++errorCount;
  }
  EXPECT_GE(errorCount, 2);
}
