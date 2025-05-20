#include "core/Translation.h"

#include "SimpleIni.h"
#include "common/WCharUtils.h"
#include "common/log.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{

auto Translation::GetTranslateLanguages(const std::filesystem::path &path, std::vector<std::string> &languages) -> bool
{
    if (m_translateFilePathCache.empty() && !IndexTranslateLanguages(path))
    {
        log_error("Can't index translate language in path {}", path.string());
        return false;
    }
    for (const auto &entry : m_translateFilePathCache)
    {
        languages.push_back(entry.first);
    }
    return true;
}

auto Translation::UseLanguage(const char *language) -> bool
{
    if (!m_translateFilePathCache.contains(language))
    {
        log_error("Can't use language {}, please first call GetTranslateLanguages", language);
        return false;
    }

    std::filesystem::path translateFilePath(m_translateFilePathCache[language]);
    if (!exists(translateFilePath) || is_directory(translateFilePath))
    {
        log_error("Can't open language {} translation file {}", language, translateFilePath.string());
        return false;
    }

    if (!LoadLanguage(translateFilePath))
    {
        log_error("Can't load language {} translate file, fallback to default.", translateFilePath.string());
        LoadDefaultTranslate();
        return false;
    }
    return true;
}

auto Translation::UseSection(const char *section) -> void
{
    m_section.assign(section);
}

auto Translation::Get(const char *key) const -> const char *
{
    return m_ini.GetValue(m_section.c_str(), key, key);
}

auto Translation::Has(const char *key) const -> bool
{
    return m_ini.KeyExists(m_section.c_str(), key);
}

auto Translation::LoadLanguage(const std::filesystem::path &path) -> bool
{
    if (!m_ini.IsEmpty())
    {
        m_ini.Reset();
    }
    return m_ini.LoadFile(path.string().c_str()) == SI_OK;
}

void Translation::LoadDefaultTranslate()
{
    m_ini.LoadData(R"(
[Tool Window]
$Settings = Settings
[Settings]
$Enable_Mod = Enable Mod
$Enable_Mod_Tooltip = Unchecked will disable all mod feature(Disable keyboard).
$States = States
$Ime_Enabled = IME Enabled
$Ime_Focus = IME Focus
$Ime_Focus_Tooltip = IME only enabled when any Text Entry is active and has keyboard focus.
$Force_Focus_Ime = Force Focus Ime
$Ime_Follow_Ime = Ime follow cursor
$Ime_Follow_Ime_Tooltip = Ime window appear in cursor position.
$Keep_Ime_Open = Keep Ime Open
$Keep_Ime_Open_Tooltip = Check KeepImeOpen when no any text entry menu will cause fatal bug!.\n Please only checked it temporary when some text entry not support IME
$Themes = Themes
$Themes_Apply = Apply
)");
}

auto Translation::IndexTranslateLanguages(const std::filesystem::path &path) -> bool
{
    namespace fs = std::filesystem;
    const std::regex pattern(R"(translate_(\w+)\.ini)", std::regex_constants::icase);
    std::smatch      themeNameMatch;

    if (!exists(path) || !is_directory(path))
    {
        return false;
    }

    for (const auto &entry : fs::directory_iterator(path))
    {
        if (entry.is_regular_file())
        {
            const std::string filename = entry.path().filename().string();
            if (std::regex_match(filename, themeNameMatch, pattern))
            {
                if (themeNameMatch.size() > 1)
                {
                    m_translateFilePathCache[themeNameMatch[1].str()] = entry.path();
                }
            }
        }
    }
    return true;
}
}
}