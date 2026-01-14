#pragma once

#include "common/imgui/ThemesLoader.h"
#include "common/utils.h"
#include "core/State.h"
#include "core/Translation.h"
#include "ime/ITextService.h"
#include "imgui.h"
#include "tsf/LangProfileUtil.h"
#include "ui/Settings.h"
#include "utils/FontManager.h"

#include <vector>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class ImeWnd;

constexpr auto MAX_ERROR_MESSAGE_COUNT = 10;

class ImeUI
{
    using State = Core::State;

public:
    explicit ImeUI(ImeWnd *pImeWnd, ITextService *pTextService)
        : m_pImeWnd(pImeWnd), m_pTextService(pTextService),
          m_themesLoader(CommonUtils::GetInterfaceFile(ImGuiUtil::ThemesLoader::DEFAULT_THEME_FILE)),
          m_fontBuilderView(m_translation)
    {
    }

    ~ImeUI();
    ImeUI(const ImeUI &other)                = delete;
    ImeUI(ImeUI &&other) noexcept            = delete;
    ImeUI &operator=(const ImeUI &other)     = delete;
    ImeUI &operator=(ImeUI &&other) noexcept = delete;

    bool Initialize(LangProfileUtil *pLangProfileUtil, const Settings &settings);
    void ApplyAppearanceSettings(Settings &settings);
    void NewFrame();
    void Draw(const Settings &settings);
    void DrawToolWindow(Settings &settings);
    void ShowToolWindow();

    bool IsShowingToolWindow() const
    {
        return m_fShowToolWindow;
    }

    bool IsPinedToolWindow() const
    {
        return m_fPinToolWindow;
    }

private:
    void DrawInputMethodsCombo() const;
    void DrawSettings(Settings &settings);
    void DrawModConfig(Settings &settings);
    void DrawFontConfig(Settings &settings);
    void DrawFeatures(Settings &settings);
    void DrawSettingsContent(Settings &settings);
    void DrawStates() const;
    void DrawWindowPosUpdatePolicy(Settings &settings);
    void DrawCompWindow(const Settings &settings) const;
    void DrawCandidateWindows() const;
    auto Translate(const char *label) const -> const char *;

    static auto UpdateImeWindowPos(const Settings &settings, ImVec2 &windowPos) -> void;
    static auto UpdateImeWindowPosByCaret(ImVec2 &windowPos) -> void;
    auto        IsImeNeedRelayout() const -> bool;
    /**
     * @brief Ensure the current ImGui window is fully within the viewport.
     */
    static void ClampWindowToViewport(const ImVec2 &windowSize, ImVec2 &windowPos);

    static void FillCommonStyleFields(ImGuiStyle &style, const Settings &settings);

    static bool DrawCombo(const char *label, const std::vector<std::string> &values, std::string &selected);

    static constexpr auto TOOL_WINDOW_NAME = std::span("ToolWindow##SimpleIME");

    ImeWnd                  *m_pImeWnd         = nullptr;
    ITextService            *m_pTextService    = nullptr;
    LangProfileUtil         *m_langProfileUtil = nullptr;
    Translation              m_translation;
    std::vector<std::string> m_translateLanguages;
    ImVec2                   m_imeWindowSize = ImVec2(0, 0);
    ImVec2                   m_imeWindowPos  = ImVec2(0, 0);
    ImGuiUtil::ThemesLoader  m_themesLoader;

    bool m_fShowToolWindow = false;
    bool m_fPinToolWindow  = false;

    /**
     * @brief Manages preview font data manually to prevent redundant memory copying.
     * @details This implementation sets @c FontDataOwnedByAtlas to false.
     * The allocated memory is explicitly released during the preview
     * font rebuild process.
     * @note Ensure that the Atlas remains valid as long as the font data is in use.
     */
    class PreviewFont
    {
        ImFont     *m_imFont = nullptr;
        std::string m_filePath;
        std::string m_fullName;
        bool        m_fontOwner  = false;
        bool        m_wantUpdate = false;

    public:
        constexpr bool IsCommittable() const
        {
            return m_imFont != nullptr && !m_filePath.empty();
        }

        constexpr bool IsWantUpdate() const
        {
            return m_wantUpdate;
        }

        [[nodiscard]] auto GetImFont() const -> ImFont *
        {
            return m_imFont;
        }

        [[nodiscard]] auto GetFilePath() const -> const std::string &
        {
            return m_filePath;
        }

        [[nodiscard]] auto GetFullName() const -> const std::string &
        {
            return m_fullName;
        }

        [[nodiscard]] bool IsFontOwner() const
        {
            return m_fontOwner;
        }

        void SetProperty(const std::string &a_fullName, const std::string &a_fontFilePath)
        {
            m_fullName   = a_fullName;
            m_filePath   = a_fontFilePath;
            m_wantUpdate = true;
        }

        void SetImFont(ImFont *imFont)
        {
            m_imFont     = imFont;
            m_fontOwner  = true;
            m_wantUpdate = false;
        }

        void SetPreviewImFont(ImFont *imFont)
        {
            m_imFont    = imFont;
            m_fontOwner = false;
        }

        void Reset()
        {
            m_imFont = nullptr;
            m_fullName.clear();
            m_filePath.clear();
            m_fontOwner  = false;
            m_wantUpdate = false;
        }
    };

    class FontBuilder
    {
    public:
        void Initialize()
        {
            m_fontManager.FindInstalledFonts();
        }

        void UpdatePreviewFont(const FontInfo &fontInfo);
        void BuildPreviewFont();

        void SetBaseFont();
        void MergeFont();
        void Preview();
        void SetAsDefault(Settings &settings);
        void Reset();

        constexpr bool IsBuilding() const
        {
            return m_baseFont != nullptr;
        }

        constexpr auto GetFontNames() -> const std::vector<std::string> &
        {
            return m_fontNames;
        }

        constexpr auto GetPreviewFont() -> const PreviewFont &
        {
            return m_previewFont;
        }

        constexpr auto GetFontManager() -> const FontManager &
        {
            return m_fontManager;
        }

    private:
        void ReleasePreviewFont();

        FontManager              m_fontManager = {};
        ImFont                  *m_baseFont    = nullptr;
        PreviewFont              m_previewFont;
        std::vector<std::string> m_fontNames;
        std::vector<std::string> m_fontPathList;
    } m_fontBuilder;

    class FontBuilderView
    {
        static constexpr auto TITLE_HELP     = "Help";
        static constexpr auto TITLE_WARNINGS = "Warnings";

    public:
        explicit FontBuilderView(Translation &translation) : m_translation(translation) {}

        void Draw(FontBuilder &fontBuilder, Settings &settings);

    private:
        bool DrawFontViewer(FontBuilder &fontBuilder, const Settings &settings);
        void DrawHelpModal() const;
        void DrawWarningsModal() const;

        ImGuiTextFilter m_filter = {};
        Translation    &m_translation;
    } m_fontBuilderView;
};
} // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
