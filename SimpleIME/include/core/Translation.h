#pragma once

#include <SimpleIni.h>
#include <filesystem>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class Translation
{
public:
    Translation()                                   = default;
    ~Translation()                                  = default;
    Translation(const Translation &other)           = delete;
    Translation(Translation &&other)                = delete;
    Translation operator=(const Translation &other) = delete;
    Translation operator=(Translation &&other)      = delete;

    auto GetTranslateLanguages(const std::filesystem::path &path, std::vector<std::string> &languages) -> bool;
    auto UseLanguage(const char *language) -> bool;
    auto UseSection(const char *section) -> void;
    auto Get(const char *key) const -> const char *;
    auto Has(const char *key) const -> bool;

    auto operator[](const char *key) const -> const char *
    {
        return Get(key);
    }

private:
    auto        LoadLanguage(const std::filesystem::path &path) -> bool;
    void        LoadDefaultTranslate();
    auto        IndexTranslateLanguages(const std::filesystem::path &path) -> bool;
    CSimpleIniA m_ini{};
    std::string m_section = "Tool Window";

    std::unordered_map<std::string, std::filesystem::path> m_translateFilePathCache;
};
}
}