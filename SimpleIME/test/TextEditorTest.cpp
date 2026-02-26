
#include "ime/TextEditor.h"

#include <gtest/gtest.h>

TEST(TestEditorTest, ShouldSelectAll)
{
    Ime::TextEditor textEditor;

    constexpr std::wstring_view test_text = L"hello world";
    textEditor.InsertText(test_text);

    // action
    textEditor.SelectAll();

    // verify
    textEditor.InsertText(L"what");

    EXPECT_EQ(textEditor.GetTextSize(), std::wstring(L"what").size());
    EXPECT_EQ(textEditor.GetText(), L"what");
}

TEST(TestEditorTest, ShouldInertAtEnd)
{
    Ime::TextEditor textEditor;

    constexpr std::wstring_view test_text = L"hello world";
    textEditor.InsertText(test_text);

    // verify
    constexpr std::wstring_view test_insert_text = L"what";
    textEditor.InsertText(test_insert_text);

    EXPECT_EQ(textEditor.GetTextSize(), test_text.size() + test_insert_text.size());
    EXPECT_EQ(textEditor.GetText(), L"hello worldwhat");
}

TEST(TestEditorTest, ShouldInertAndReplaceSelectedText)
{
    Ime::TextEditor textEditor;

    constexpr std::wstring_view test_text = L"hello world";
    textEditor.InsertText(test_text);

    // action
    TS_SELECTION_ACP selection{};
    constexpr auto   test_replace_length = 3;
    selection.acpStart                   = 5;
    selection.acpEnd                     = selection.acpStart + test_replace_length;
    textEditor.Select(&selection);

    // verify
    constexpr std::wstring_view test_insert_text = L" what";
    textEditor.InsertText(test_insert_text);

    EXPECT_EQ(textEditor.GetTextSize(), test_text.size() - test_replace_length + test_insert_text.size());
    EXPECT_EQ(textEditor.GetText(), L"hello whatrld");
}
