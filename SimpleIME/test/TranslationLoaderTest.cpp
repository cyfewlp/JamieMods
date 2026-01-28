#include "i18n/TranslationLoader.h"

#include "ui/Settings.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <random>
#include <spdlog/common.h>

using namespace LIBC_NAMESPACE::Ime;

namespace fs = std::filesystem;

TEST(TranslationLoaderTest, ShouldStoreAsFullQualifyKey)
{
    const std::filesystem::path tempTranslateFile("translate_fakeLang.toml");
    std::ofstream               tempFile(tempTranslateFile);
    tempFile << "[Section1]" << "\n";
    tempFile << R"(k1 = "v1")" << "\n";
    tempFile << "[Section1.ChildSection]\n";
    tempFile << R"(k2 = "v2")" << "\n";
    tempFile.close();

    TranslationLoader loader(fs::absolute(fs::path(".")), "Section1");

    auto translatorOpt = loader.LoadFrom("fakeLang");

    ASSERT_TRUE(translatorOpt.has_value());
    const auto &translator = translatorOpt.value();
    ASSERT_EQ(translator.Translate(LIBC_NAMESPACE::i18n::HashKey("Section1.k1"), ""), "v1");
    ASSERT_EQ(translator.Translate(LIBC_NAMESPACE::i18n::HashKey("Section1.ChildSection.k2"), ""), "v2");

    std::filesystem::remove(tempTranslateFile);
}

TEST(TranslationLoaderTest, ShouldScanAllTranslateFiles)
{
    std::ofstream fakeLang1("translate_fakeLang1.toml");
    fakeLang1.close();

    std::ofstream fakeLang2("translate_fakeLang2.toml");
    fakeLang2.close();

    std::ofstream fakeLang3("translate_fakeLang3.toml");
    fakeLang3.close();

    std::vector<std::string> languages;
    TranslationLoader::ScanLanguages(fs::path("."), languages);

    ASSERT_EQ(languages.size(), 3);
    ASSERT_EQ(languages[0], "fakeLang1");
    ASSERT_EQ(languages[1], "fakeLang2");
    ASSERT_EQ(languages[2], "fakeLang3");

    std::filesystem::remove("translate_fakeLang1.toml");
    std::filesystem::remove("translate_fakeLang2.toml");
    std::filesystem::remove("translate_fakeLang3.toml");
}