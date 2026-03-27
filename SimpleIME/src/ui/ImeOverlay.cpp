//
// Created by jamie on 2026/3/10.
//

#include "ui/ImeOverlay.h"

#include "i18n/translator_manager.h"
#include "menu/MenuNames.h"
#include "path_utils.h"
#include "utils/Utils.h"

namespace Ime::UI
{
namespace
{
void TogglePinned(bool &pinned, bool &showing)
{
    if (pinned)
    {
        pinned = false;
    }
    else if (showing)
    {
        showing = false;
    }
    else
    {
        showing = true;
    }
}

void HandleRequestAndSyncOverlayState(Settings::RuntimeData &runtimeData)
{
    if (!runtimeData.overlayShowing)
    {
        runtimeData.toolWindowShowing = false;
    }

    if (runtimeData.requestCloseTopWindow)
    {
        runtimeData.requestCloseTopWindow = false;
        if (runtimeData.toolWindowShowing)
        {
            runtimeData.toolWindowShowing = false;
        }
        else
        {
            // When there is no tool window to close, interpret "close top window" as a request
            // to close the overlay itself. This means pressing the close shortcut while the
            // settings/tool window is not open will close the entire overlay.
            runtimeData.overlayShowing = false;
        }
    }

    if (std::exchange(runtimeData.requestShowOverlay, false))
    {
        runtimeData.overlayShowing = true;
    }
    else if (std::exchange(runtimeData.requestHideOverlay, false) && !runtimeData.overlayPinned)
    {
        runtimeData.overlayShowing = false;
    }
}
} // namespace

ImeOverlay::ImeOverlay(ImGuiKeyChord shortcut, std::string_view language) : m_shortcut(shortcut)
{
    i18n::SetTranslator(&m_translator);
    i18n::UpdateTranslator(language, "english", utils::GetInterfacePath() / SIMPLE_IME);
}

ImeOverlay::~ImeOverlay()
{
    i18n::SetTranslator(nullptr);
}

auto ImeOverlay::Draw(const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles, Settings &settings) -> void
{
    auto &runtimeData = settings.runtimeData;
    if (ImGui::IsKeyChordPressed(m_shortcut))
    {
        TogglePinned(runtimeData.overlayPinned, runtimeData.overlayShowing);
    }

    // Handle before overlay render: avoid override the user request.
    HandleRequestAndSyncOverlayState(runtimeData);

    const auto notPinnedOverlay = !runtimeData.overlayPinned && runtimeData.overlayShowing;
    if (notPinnedOverlay)
    {
        if (m_toolWindow == nullptr)
        {
            m_toolWindow = std::make_unique<UI::ToolWindow>();
        }
        if (runtimeData.toolWindowShowing)
        {
            m_toolWindow->Draw(settings);
        }
    }
    else if (m_toolWindow != nullptr)
    {
        m_toolWindow.reset();
    }

    // Drawing after ToolWindow to avoid override the user `close top window` request: May reopen tool window.
    if (runtimeData.overlayShowing)
    {
        // May change runtimeData.toolWindowShowing, but apply it will be deferred to next frame.
        LanguageBar::Draw(runtimeData.overlayPinned, runtimeData.toolWindowShowing, activeLangProfile, langProfiles);
    }
}
} // namespace Ime::UI
