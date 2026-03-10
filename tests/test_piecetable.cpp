#include "core/PieceTable.h"
#include <gtest/gtest.h>

using namespace lighttex::core;

TEST(PieceTable, NewBufferIsEmpty) {
  PieceTable pt;
  EXPECT_EQ(pt.length(), 0u);
  EXPECT_TRUE(pt.empty());
  EXPECT_EQ(pt.text(), "");
}

TEST(PieceTable, FromText) {
  PieceTable pt("Hello\nWorld");
  EXPECT_EQ(pt.length(), 11u);
  EXPECT_EQ(pt.text(), "Hello\nWorld");
  EXPECT_EQ(pt.lineCount(), 2u);
}

TEST(PieceTable, InsertAtBeginning) {
  PieceTable pt("World");
  pt.insert(0, "Hello ");
  EXPECT_EQ(pt.text(), "Hello World");
}

TEST(PieceTable, InsertAtMiddle) {
  PieceTable pt("Hllo");
  pt.insert(1, "e");
  EXPECT_EQ(pt.text(), "Hello");
}

TEST(PieceTable, InsertAtEnd) {
  PieceTable pt("Hello");
  pt.insert(5, " World");
  EXPECT_EQ(pt.text(), "Hello World");
}

TEST(PieceTable, DeleteRange) {
  PieceTable pt("Hello World");
  pt.erase(5, 6);
  EXPECT_EQ(pt.text(), "Hello");
}

TEST(PieceTable, ReplaceRange) {
  PieceTable pt("Hello World");
  pt.replace(6, 5, "C++");
  EXPECT_EQ(pt.text(), "Hello C++");
}

TEST(PieceTable, CharToLineCol) {
  PieceTable pt("Hello\nWorld\nFoo");
  // H=0, e=1, l=2, l=3, o=4, \n=5, W=6
  EXPECT_EQ(pt.charToLineCol(0), std::make_pair(size_t{0}, size_t{0}));
  EXPECT_EQ(pt.charToLineCol(5), std::make_pair(size_t{0}, size_t{5}));
  EXPECT_EQ(pt.charToLineCol(6), std::make_pair(size_t{1}, size_t{0}));
  EXPECT_EQ(pt.charToLineCol(11), std::make_pair(size_t{1}, size_t{5}));
  EXPECT_EQ(pt.charToLineCol(12), std::make_pair(size_t{2}, size_t{0}));
}

TEST(PieceTable, LineColToChar) {
  PieceTable pt("Hello\nWorld\nFoo");
  EXPECT_EQ(pt.lineColToChar(0, 0), 0u);
  EXPECT_EQ(pt.lineColToChar(1, 0), 6u);
  EXPECT_EQ(pt.lineColToChar(2, 0), 12u);
  // Clamp col to line length
  EXPECT_EQ(pt.lineColToChar(2, 100), 15u);
}

TEST(PieceTable, GetLine) {
  PieceTable pt("Hello\nWorld\nFoo");
  EXPECT_EQ(pt.line(0), "Hello\n");
  EXPECT_EQ(pt.line(1), "World\n");
  EXPECT_EQ(pt.line(2), "Foo");
  EXPECT_EQ(pt.line(10), "");
}

TEST(PieceTable, UnicodeHandling) {
  PieceTable pt("caf\xc3\xa9"); // "cafe" with accent
  EXPECT_EQ(pt.text(), "caf\xc3\xa9");
  EXPECT_EQ(pt.length(), 5u); // byte length

  PieceTable pt2("Hello \xf0\x9f\x8c\x8d"); // "Hello " + globe emoji
  EXPECT_EQ(pt2.text(), "Hello \xf0\x9f\x8c\x8d");
}

TEST(PieceTable, MultipleInserts) {
  PieceTable pt;
  pt.insert(0, "a");
  pt.insert(1, "c");
  pt.insert(1, "b");
  EXPECT_EQ(pt.text(), "abc");
}

TEST(PieceTable, DeleteAll) {
  PieceTable pt("Hello");
  pt.erase(0, 5);
  EXPECT_EQ(pt.text(), "");
  EXPECT_TRUE(pt.empty());
}

TEST(PieceTable, Substr) {
  PieceTable pt("Hello World");
  EXPECT_EQ(pt.substr(0, 5), "Hello");
  EXPECT_EQ(pt.substr(6, 5), "World");
  EXPECT_EQ(pt.substr(0, 11), "Hello World");
}

TEST(PieceTable, LineCount) {
  PieceTable pt("");
  EXPECT_EQ(pt.lineCount(), 1u); // Empty text has 1 line

  PieceTable pt2("a\nb\nc");
  EXPECT_EQ(pt2.lineCount(), 3u);

  PieceTable pt3("a\nb\nc\n");
  EXPECT_EQ(pt3.lineCount(), 4u); // trailing newline = extra line
}

TEST(PieceTable, InsertNewlines) {
  PieceTable pt("ab");
  pt.insert(1, "\n");
  EXPECT_EQ(pt.text(), "a\nb");
  EXPECT_EQ(pt.lineCount(), 2u);
  EXPECT_EQ(pt.line(0), "a\n");
  EXPECT_EQ(pt.line(1), "b");
}

TEST(PieceTable, LargeInserts) {
  PieceTable pt;
  std::string big(10000, 'x');
  pt.insert(0, big);
  EXPECT_EQ(pt.length(), 10000u);
  EXPECT_EQ(pt.text(), big);
}
