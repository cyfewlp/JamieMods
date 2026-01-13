#ifndef IMEUI_H
#define IMEUI_H

#pragma once

#include "common/imgui/ThemesLoader.h"
#include "common/utils.h"
#include "configs/AppConfig.h"
#include "core/State.h"
#include "core/Translation.h"
#include "ime/ITextService.h"
#include "imgui.h"
#include "tsf/LangProfileUtil.h"
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
    explicit ImeUI(AppUiConfig const &uiConfig, ImeWnd *pImeWnd, ITextService *pTextService)
        : m_uiConfig(uiConfig), m_pImeWnd(pImeWnd), m_pTextService(pTextService),
          m_themesLoader(CommonUtils::GetInterfaceFile(ImGuiUtil::ThemesLoader::DEFAULT_THEME_FILE))
    {
    }

    ~ImeUI();
    ImeUI(const ImeUI &other)                = delete;
    ImeUI(ImeUI &&other) noexcept            = delete;
    ImeUI &operator=(const ImeUI &other)     = delete;
    ImeUI &operator=(ImeUI &&other) noexcept = delete;

    bool Initialize(LangProfileUtil *pLangProfileUtil);
    void NewFrame();
    void Draw(const Settings &settings);
    void DrawToolWindow(Settings &settings);
    void ShowToolWindow();
    void ApplyUiSettings(Settings &settings);

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
    void DrawFontBuilder(const Settings &settings);
    auto DrawFontViewer(const Settings &settings) -> bool;

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

    AppUiConfig              m_uiConfig;
    ImeWnd                  *m_pImeWnd         = nullptr;
    ITextService            *m_pTextService    = nullptr;
    LangProfileUtil         *m_langProfileUtil = nullptr;
    Translation              m_translation;
    std::vector<std::string> m_translateLanguages;
    ImVec2                   m_imeWindowSize = ImVec2(0, 0);
    ImVec2                   m_imeWindowPos  = ImVec2(0, 0);
    ImGuiUtil::ThemesLoader  m_themesLoader;
    FontManager              m_fontManager = {};

    bool m_fShowToolWindow = false;
    bool m_fPinToolWindow  = false;

    /**
     * @brief Manages preview font data manually to prevent redundant memory copying.
     * @details This implementation sets @c FontDataOwnedByAtlas to false.
     * The allocated memory is explicitly released during the preview
     * font rebuild process.
     * @note Ensure that the Atlas remains valid as long as the font data is in use.
     */
    struct PreviewFont
    {
        ImFont     *imFont = nullptr;
        std::string filePath;
        std::string fullName;
        bool        fontOwner  = false;
        bool        wantUpdate = false;

        constexpr bool IsCommittable() const
        {
            return imFont != nullptr && !filePath.empty();
        }

        void Set(const std::string &a_fullName, const std::string &a_fontFilePath)
        {
            fullName   = a_fullName;
            filePath   = a_fontFilePath;
            wantUpdate = true;
        }

        void Reset()
        {
            imFont = nullptr;
            fullName.clear();
            filePath.clear();
            fontOwner  = false;
            wantUpdate = false;
        }
    };

    struct FontBuilder
    {
        void UpdatePreviewFont(const FontInfo &fontInfo);
        void BuildPreviewFont();

        void SetBaseFont();
        void MergeFont();
        void Preview();
        void SetAsDefault();
        void Reset();

        constexpr bool IsBuilding() const
        {
            return baseFont != nullptr;
        }

        constexpr auto GetFontNames() -> const std::vector<std::string> &
        {
            return fontNames;
        }

        constexpr auto GetPreviewFont() -> const PreviewFont &
        {
            return previewFont;
        }

    private:
        void ReleasePreviewFont();

        std::vector<std::string> fontNames;
        ImFont                  *baseFont = nullptr;
        PreviewFont              previewFont;
    } m_fontBuilder;
};
} // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif
