#include "snippets/SnippetSession.h"
#include "snippets/Snippets.h"

#include <gtest/gtest.h>

using namespace lighttex::snippets;

// --- SnippetManager tests ---

TEST(SnippetManagerTest, DefaultSnippetsLoaded) {
  SnippetManager mgr;
  EXPECT_GE(mgr.snippets().size(), 18u);
}

TEST(SnippetManagerTest, FindByTrigger) {
  SnippetManager mgr;
  auto *s = mgr.findByTrigger("\\begin");
  ASSERT_NE(s, nullptr);
  EXPECT_EQ(s->label, "Begin Environment");
}

TEST(SnippetManagerTest, FindByTriggerNotFound) {
  SnippetManager mgr;
  auto *s = mgr.findByTrigger("\\nonexistent");
  EXPECT_EQ(s, nullptr);
}

TEST(SnippetManagerTest, ExpandBodySimple) {
  std::string body = "\\textbf{${1:text}}$0";
  std::string expanded = SnippetManager::expandBody(body);
  EXPECT_EQ(expanded, "\\textbf{text}");
}

TEST(SnippetManagerTest, ExpandBodyMultipleTabstops) {
  std::string body = "\\frac{${1:num}}{${2:den}}$0";
  std::string expanded = SnippetManager::expandBody(body);
  EXPECT_EQ(expanded, "\\frac{num}{den}");
}

TEST(SnippetManagerTest, LoadFromToml) {
  SnippetManager mgr;
  std::string toml = R"(
[[snippet]]
trigger = "\\test"
label = "Test Snippet"
body = "\\test{${1:arg}}$0"
)";
  EXPECT_TRUE(mgr.loadFromToml(toml));
  EXPECT_EQ(mgr.snippets().size(), 1u);
  EXPECT_EQ(mgr.snippets()[0].trigger, "\\test");
}

TEST(SnippetManagerTest, LoadFromInvalidToml) {
  SnippetManager mgr;
  EXPECT_FALSE(mgr.loadFromToml("not valid {{{{ toml"));
}

// --- SnippetSession tests ---

TEST(SnippetSessionTest, StartWithTabstops) {
  SnippetSession session;
  EXPECT_TRUE(session.start("\\textbf{${1:text}}$0", 0));
  EXPECT_TRUE(session.isActive());
  EXPECT_EQ(session.expandedText(), "\\textbf{text}");
}

TEST(SnippetSessionTest, TabstopNavigation) {
  SnippetSession session;
  session.start("\\frac{${1:num}}{${2:den}}$0", 0);
  EXPECT_TRUE(session.isActive());
  // First tabstop: ${1:num}
  EXPECT_EQ(session.currentOffset(), 6); // \frac{ = 6 chars
  EXPECT_EQ(session.currentLength(), 3); // "num"

  // Move to next
  EXPECT_TRUE(session.nextTabStop());
  EXPECT_EQ(session.currentLength(), 3); // "den"

  // Move to $0 (final)
  EXPECT_TRUE(session.nextTabStop());
  EXPECT_FALSE(session.isActive()); // $0 deactivates
}

TEST(SnippetSessionTest, PrevTabStop) {
  SnippetSession session;
  session.start("\\frac{${1:num}}{${2:den}}$0", 0);
  session.nextTabStop();                 // move to $2
  EXPECT_TRUE(session.prevTabStop());    // back to $1
  EXPECT_EQ(session.currentLength(), 3); // "num"
}

TEST(SnippetSessionTest, CancelDeactivates) {
  SnippetSession session;
  session.start("${1:text}$0", 0);
  EXPECT_TRUE(session.isActive());
  session.cancel();
  EXPECT_FALSE(session.isActive());
}

TEST(SnippetSessionTest, NoTabstopsReturnsFalse) {
  SnippetSession session;
  EXPECT_FALSE(session.start("plain text", 0));
  EXPECT_FALSE(session.isActive());
}

TEST(SnippetSessionTest, InsertOffset) {
  SnippetSession session;
  session.start("${1:hello}$0", 10);
  EXPECT_EQ(session.currentOffset(), 10);
  EXPECT_EQ(session.currentLength(), 5);
}
