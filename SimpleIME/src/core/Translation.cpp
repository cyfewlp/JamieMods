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
$Mod_Config = Mod
$Enable_Mod = Enable Mod
$Enable_Mod_Tooltip = Unchecked will disable all mod feature(Disable keyboard).
$Font_Size_Scale = Font Size Scale
$Languages = Language
$States = States
$Features = Features
$Ime_Enabled = IME Enabled
$Ime_Enabled_Tooltip = By default, IME only enable when exists text entry
$Ime_Focus = IME Focus
$Ime_Focus_Tooltip = IME only enabled when any Text Entry is active and has keyboard focus.
$Message_Filter_Enabled = Message Filter
$Message_Filter_Enabled_Tooltip = Intercept game events during IME activation
$Force_Focus_Ime = Force Focus Ime
$Ime_Follow_Ime = Ime follow cursor
$Ime_Follow_Ime_Tooltip = Ime window appear in cursor position.
$Keep_Ime_Open = Keep Ime Open
$Keep_Ime_Open_Tooltip = Usually used to enable IME for some text entry that not support IME
$Themes = Themes
$Focus_Manage = Focus Management Setting
$Focus_Manage_Permanent = Permanent Focus Management
$Focus_Manage_Permanent_Tooltip = More stable, focus management is fully handled by this Mod
$Focus_Manage_Temporary = Temporary
$Focus_Manage_Temporary_Tooltip = Better compatibility, Mod manages focus only when IME is enabled
$Enable_Unicode_Paste = Enable unicode pate
$Enable_Unicode_Paste_Tooltip = Enable or disable mod provided unicode paste feature
$Update_Ime_Window_Pos_By_Caret = Update IME window position by caret
$Update_Ime_Window_Pos_By_Caret_Tooltip = Auto update IME position based on the caret position of the active text field

[Ime Window Pos]
$Policy = IME window position update policy
$Update_By_Cursor = Based on cursor position
$Update_By_Cursor_Tooltip = Update the IME window position according to the current mouse position
$Update_By_Caret = Based on caret position
$Update_By_Caret_Tooltip = Automatically update the IME window based on the caret position of the currently active input box
$Update_By_None = None
$Update_By_None_Tooltip = IME window position is not updated automatically
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