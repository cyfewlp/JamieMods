#include "i18n/translator_manager.h"
#include "ui/Settings.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <random>

namespace fs = std::filesystem;

TEST(TranslatorManagerTest, ShouldStoreAsFullQualifyKey)
{
    const std::filesystem::path tempTranslateFile("translate_fakeLang.toml");
    std::ofstream               tempFile(tempTranslateFile);
    tempFile << R"([Section1]
k1 = "v1"
[Section1.ChildSection]
k2 = "v2"
[Section2]
s2k1 = "s2v1"
[Section2.Child]
s2k2 = "s2v2"
)" << "\n";
    tempFile.close();

    Ime::i18n::UpdateTranslator("fakeLang", "english", fs::current_path());

    EXPECT_STREQ(Ime::Translate("Section1.k1").data(), "v1");
    EXPECT_STREQ(Ime::Translate("Section1.ChildSection.k2").data(), "v2");
    EXPECT_STREQ(Ime::Translate("Section2.s2k1").data(), "s2v1");
    EXPECT_STREQ(Ime::Translate("Section2.Child.s2k2").data(), "s2v2");

    std::filesystem::remove(tempTranslateFile);
    Ime::i18n::ReleaseTranslator();
}

TEST(TranslatorManagerTest, should_return_key_if_not_found)
{
    EXPECT_STREQ(Ime::Translate("Section1.k1").data(), "Section1.k1");
    EXPECT_STREQ(Ime::Translate("a invalid key").data(), "a invalid key");
}

TEST(TranslatorManagerTest, ShouldScanAllTranslateFiles)
{
    std::ofstream fakeLang1("translate_fakeLang1.toml");
    fakeLang1.close();

    std::ofstream fakeLang2("translate_fakeLang2.toml");
    fakeLang2.close();

    std::ofstream fakeLang3("translate_fakeLang3.toml");
    fakeLang3.close();

    std::vector<std::string> languages;
    Ime::i18n::ScanLanguages(fs::path("."), languages);

    ASSERT_EQ(languages.size(), 3);
    ASSERT_EQ(languages[0], "fakeLang1");
    ASSERT_EQ(languages[1], "fakeLang2");
    ASSERT_EQ(languages[2], "fakeLang3");

    std::filesystem::remove("translate_fakeLang1.toml");
    std::filesystem::remove("translate_fakeLang2.toml");
    std::filesystem::remove("translate_fakeLang3.toml");
}
