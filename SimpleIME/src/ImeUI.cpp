//
// Created by jamie on 25-1-21.
//

#include "ImeUI.h"
#include "common/WCharUtils.h"
#include "configs/Configs.h"
#include "tsf/LangProfileUtil.h"
#include "tsf/TextStore.h"

#include "imgui.h"
#include <clocale>
#include <cstdint>
#include <tchar.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        ImeUI::ImeUI(const AppUiConfig &uiConfig, InputMethod *pInputMethod) : m_pUiConfig(uiConfig)
        {
            _tsetlocale(LC_ALL, _T(""));
            m_pHeap        = HeapCreate(HEAP_GENERATE_EXCEPTIONS, IME_UI_HEAP_INIT_SIZE, IME_UI_HEAP_MAX_SIZE);
            m_pCompStr     = new WcharBuf(m_pHeap, WCHAR_BUF_INIT_SIZE);
            m_pCompResult  = new WcharBuf(m_pHeap, WCHAR_BUF_INIT_SIZE);
            m_pInputMethod = pInputMethod;
            m_pTextEditor  = pInputMethod->GetTextEditor();
        }

        ImeUI::~ImeUI()
        {
            HeapDestroy(m_pHeap);
            m_pHeap = nullptr;
            delete m_pCompStr;
            delete m_pCompResult;
        }

        bool ImeUI::Initialize(LangProfileUtil *pLangProfileUtil)
        {
            log_debug("Initializing ImeUI...");
            m_langProfileUtil = pLangProfileUtil;
            if (!m_langProfileUtil->LoadAllLangProfiles())
            {
                log_error("Failed load lang profiles");
                return false;
            }
            if (!m_langProfileUtil->LoadActiveIme())
            {
                log_error("Failed load active ime");
                return false;
            }
            UpdateLanguage();
            return true;
        }

        void ImeUI::SetHWND(const HWND hWnd)
        {
            m_hWndIme = hWnd;
        }

        void ImeUI::CompositionString(HIMC hIMC, LPARAM compFlag) const
        {
            if (GetCompStr(hIMC, compFlag, GCS_COMPSTR, m_pCompStr))
            {
                if (spdlog::should_log(spdlog::level::trace))
                {
                    const auto str = WCharUtils::ToString(m_pCompStr->Data());
                    log_trace("IME Composition String: {}", str.c_str());
                }
            }
            if (GetCompStr(hIMC, compFlag, GCS_RESULTSTR, m_pCompResult))
            {
                SendResultStringToSkyrim();
                if (spdlog::should_log(spdlog::level::trace))
                {
                    const auto str = WCharUtils::ToString(m_pCompResult->Data());
                    log_trace("IME Composition Result String: {}", str.c_str());
                }
            }
        }

        void ImeUI::SendResultStringToSkyrim() const
        {
            log_debug("Ready result string to Skyrim...");
            auto *pInterfaceStrings = RE::InterfaceStrings::GetSingleton();
            auto *pFactoryManager   = RE::MessageDataFactoryManager::GetSingleton();
            if (pInterfaceStrings == nullptr || pFactoryManager == nullptr)
            {
                log_warn("Can't send string to Skyrim may game already close?");
                return;
            }

            const auto *pFactory =
                pFactoryManager->GetCreator<RE::BSUIScaleformData>(pInterfaceStrings->bsUIScaleformData);
            if (pFactory == nullptr)
            {
                log_warn("Can't send string to Skyrim may game already close?");
                return;
            }

            // Start send message
            RE::BSFixedString menuName   = pInterfaceStrings->topMenu;
            auto              stringSize = m_pCompResult->Size();
            const auto       *pwChar     = m_pCompResult->Data();
            for (size_t i = 0; i < stringSize; i++)
            {
                uint32_t const code = pwChar[i];
                if (code == ASCII_GRAVE_ACCENT || code == ASCII_MIDDLE_DOT)
                {
                    continue;
                }
                auto *pCharEvent            = new GFxCharEvent(code, 0);
                auto *pScaleFormMessageData = pFactory->Create();
                if (pScaleFormMessageData == nullptr)
                {
                    log_error("Unable create BSTDerivedCreator.");
                    return;
                }
                pScaleFormMessageData->scaleformEvent = pCharEvent;
                log_debug("send code {:#x} to Skyrim", code);
                RE::UIMessageQueue::GetSingleton()->AddMessage(menuName, RE::UI_MESSAGE_TYPE::kScaleformEvent,
                                                               pScaleFormMessageData);
            }
        }

        auto ImeUI::GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, WcharBuf *pWcharBuf) -> bool
        {
            if ((compFlag & flagToCheck) != 0)
            {
                LONG bufLen = ImmGetCompositionStringW(hIMC, static_cast<DWORD>(flagToCheck), nullptr, 0);
                if (bufLen > 0)
                {
                    if (pWcharBuf->TryReAlloc(bufLen + 2))
                    {
                        ImmGetCompositionStringW(hIMC, static_cast<DWORD>(flagToCheck), pWcharBuf->Data(), bufLen);
                        DWORD const size = bufLen / sizeof(WCHAR);
                        pWcharBuf->SetSize(size);
                        return true;
                    }
                }
            }
            else
            {
                pWcharBuf->Clear();
            }
            return false;
        }

        void ImeUI::RenderIme()
        {
            RenderToolWindow();

            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize;
            windowFlags |= ImGuiWindowFlags_NoDecoration;
            windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;

            auto &imeState = m_pInputMethod->GetState();
            if (imeState.none(InputMethod::State::IN_CANDCHOOSEN, InputMethod::State::IN_COMPOSITION))
            {
                return;
            }

            ImGui::PushStyleColor(ImGuiCol_WindowBg, m_pUiConfig.WindowBgColor());
            ImGui::PushStyleColor(ImGuiCol_Border, m_pUiConfig.WindowBorderColor());
            ImGui::PushStyleColor(ImGuiCol_Text, m_pUiConfig.TextColor());
            ImGui::Begin("SimpleIME", nullptr, windowFlags);

            ImGui::SameLine();
            RenderCompWindow();

            ImGui::Separator();
            // render ime status window: language,
            if (imeState.any(InputMethod::State::IN_CANDCHOOSEN))
            {
                RenderCandidateWindows();
            }
            ImGui::End();
            ImGui::PopStyleColor(3);
        }

        void ImeUI::ShowToolWindow()
        {
            m_toolWindowFlags &= ~ImGuiWindowFlags_NoInputs;
            if (m_pinToolWindow)
            {
                m_pinToolWindow                = false;
                ImGui::GetIO().MouseDrawCursor = true;
                ImGui::SetWindowFocus(TOOL_WINDOW_NAME.data());
            }
            else
            {
                m_showToolWindow               = !m_showToolWindow;
                ImGui::GetIO().MouseDrawCursor = m_showToolWindow;
                if (m_showToolWindow)
                {
                    ImGui::SetWindowFocus(TOOL_WINDOW_NAME.data());
                }
            }
        }

        void ImeUI::RenderToolWindow()
        {
            if (!m_showToolWindow)
            {
                return;
            }

            ImGui::PushStyleColor(ImGuiCol_WindowBg, m_pUiConfig.WindowBgColor());
            ImGui::PushStyleColor(ImGuiCol_Border, m_pUiConfig.WindowBorderColor());
            ImGui::PushStyleColor(ImGuiCol_Text, m_pUiConfig.TextColor());
            auto btnCol     = m_pUiConfig.BtnColor();
            btnCol          = (btnCol & 0x00FFFFFF) | 0x9A000000;
            auto hoveredCol = (btnCol & 0x00FFFFFF) | 0x66000000;
            auto activeCol  = (btnCol & 0x00FFFFFF) | 0xAA000000;
            ImGui::PushStyleColor(ImGuiCol_Button, btnCol);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredCol);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeCol);
            ImGui::PushStyleColor(ImGuiCol_Header, btnCol);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hoveredCol);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, activeCol);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, btnCol);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, hoveredCol);
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, activeCol);

            ImGui::Begin(TOOL_WINDOW_NAME.data(), &m_showToolWindow, m_toolWindowFlags);

            if (!m_errorMessages.empty())
            {
                for (const auto &errorMessage : m_errorMessages)
                {
                    static constexpr ImVec4 redColor = {1.0f, 0.0f, 0.0f, 1.0f};
                    ImGui::TextColored(redColor, "%s", errorMessage.c_str());
                }
            }

            if (!m_pinToolWindow)
            {
                ImGui::Text("Drag");
                ImGui::SameLine();
            }
            if (ImGui::Button("\xf0\x9f\x93\x8c"))
            {
                m_toolWindowFlags |= ImGuiWindowFlags_NoInputs;
                m_pinToolWindow                = true;
                ImGui::GetIO().MouseDrawCursor = false;
            }

            auto        activatedGuid     = m_langProfileUtil->GetActivatedLangProfile();
            auto        installedProfiles = m_langProfileUtil->GetLangProfiles();
            auto       &profile           = installedProfiles[activatedGuid];
            const char *previewImeName    = profile.desc.c_str();
            ImGui::SameLine();
            if (ImGui::BeginCombo("###InstalledIME", previewImeName))
            {
                uint32_t idx = 0;
                for (const std::pair<GUID, LangProfile> pair : installedProfiles)
                {
                    auto       langProfile = pair.second;
                    bool const isSelected  = langProfile.guidProfile == activatedGuid;
                    auto       label       = std::format("{}##{}", langProfile.desc, idx);
                    if (ImGui::Selectable(label.c_str()))
                    {
                        // SendMessageW()  // wait handle message
                        // PostMessageW(); // not wait
                        SendMessageW(m_hWndIme, CM_ACTIVATE_PROFILE, 0, reinterpret_cast<LPARAM>(&pair.first));
                        activatedGuid = m_langProfileUtil->GetActivatedLangProfile();
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                    idx++;
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            if (m_pInputMethod->GetState().all(InputMethod::State::IN_ALPHANUMERIC))
            {
                ImGui::Text("ENG");
                ImGui::SameLine();
            }
            if (!m_pinToolWindow && !ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
            {
                ShowToolWindow();
            }
            ImGui::End();
            ImGui::PopStyleColor(12);
        }

        void ImeUI::RenderCompWindow() const
        {
            ImGui::PushStyleColor(ImGuiCol_Text, m_pUiConfig.HighlightTextColor());
            const auto &editorText = m_pTextEditor->GetText();
            const auto  str        = WCharUtils::ToString(editorText.c_str());
            ImGui::Text("%s", str.c_str());
            ImGui::PopStyleColor(1);
        }

        void ImeUI::RenderCandidateWindows() const
        {
            DWORD index = 0;
            for (const auto &candidateUi = m_pInputMethod->GetCandidateUi();
                 const auto &item : candidateUi.CandidateList())
            {
                if (index == candidateUi.Selection())
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, m_pUiConfig.HighlightTextColor());
                }
                ImGui::Text("%s", item.c_str());
                if (index == candidateUi.Selection())
                {
                    ImGui::PopStyleColor();
                }
                ImGui::SameLine();
                index++;
            }
        }

        void ImeUI::UpdateLanguage()
        {
            HKL          keyboard_layout = ::GetKeyboardLayout(0);
            LANGID const langId          = LOWORD(HandleToUlong(keyboard_layout));
            WCHAR        localeName[LOCALE_NAME_MAX_LENGTH];
            LCIDToLocaleName(MAKELCID(langId, SORT_DEFAULT), localeName, LOCALE_NAME_MAX_LENGTH, 0);

            // Retrieve keyboard code page, required for handling of non-Unicode Windows.
            LCID const keyboard_lcid = MAKELCID(HIWORD(keyboard_layout), SORT_DEFAULT);
            if (::GetLocaleInfoA(keyboard_lcid, (LOCALE_RETURN_NUMBER | LOCALE_IDEFAULTANSICODEPAGE),
                                 reinterpret_cast<LPSTR>(&m_keyboardCodePage), sizeof(m_keyboardCodePage)) == 0)
            {
                m_keyboardCodePage = CP_ACP; // Fallback to default ANSI code page when fails.
            }
        }

        template <typename... Args>
        void ImeUI::PushErrorMessage(std::format_string<Args...> fmt, Args &&...args)
        {
            if (m_errorMessages.size() >= MAX_ERROR_MESSAGE_COUNT)
            {
                auto deleteCount = m_errorMessages.size() - MAX_ERROR_MESSAGE_COUNT + 1;
                m_errorMessages.erase(m_errorMessages.begin(), m_errorMessages.begin() + deleteCount);
            }
            m_errorMessages.push_back(std::format(fmt, std::forward<Args>(args)...));
        }

    } // namespace  SimpleIME
} // namespace LIBC_NAMESPACE_DECL