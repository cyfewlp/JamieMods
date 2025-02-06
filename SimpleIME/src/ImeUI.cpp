﻿//
// Created by jamie on 25-1-21.
//

#include "ImeUI.h"
#include "Configs.h"
#include "ImeApp.h"
#include "LangProfileUtil.h"
#include "WCharUtils.h"
#include "ime_cmodes.h"
#include "imgui.h"
#include "imm.h"
#include <algorithm>
#include <clocale>
#include <cstdint>
#include <tchar.h>

#define LAST_SIZE(vector) (vector.size() - 1)

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {

        static size_t   langProfileSelected = 0;
        constexpr ULONG ONE                 = 1;

        ImeUI::ImeUI()
            : m_pHeap(HeapCreate(HEAP_GENERATE_EXCEPTIONS, IMEUI_HEAP_INIT_SIZE, IMEUI_HEAP_MAX_SIZE)),
              m_pCompStr(new WcharBuf(m_pHeap, WCHAR_BUF_INIT_SIZE)),
              m_pCompResult(new WcharBuf(m_pHeap, WCHAR_BUF_INIT_SIZE))
        {
            _tsetlocale(LC_ALL, _T(""));
        }

        ImeUI::~ImeUI()
        {
            HeapDestroy(m_pHeap);
            m_pHeap = nullptr;
            delete m_pCompStr;
            delete m_pCompResult;
        }

        void ImeUI::StartComposition()
        {
            log_trace("IME Start Composition");
            m_pCompStr->Clear();
            m_pCompResult->Clear();
            m_imeState.set(ImeState::IN_COMPOSITION);
        }

        void ImeUI::EndComposition()
        {
            log_trace("IME End Composition");
            m_pCompStr->Clear();
            m_pCompResult->Clear();
            m_imeState.reset(ImeState::IN_COMPOSITION);
        }

        void ImeUI::CompositionString(HIMC hIMC, LPARAM compFlag)
        {
            if (GetCompStr(hIMC, compFlag, GCS_COMPSTR, m_pCompStr))
            {
                if (spdlog::should_log(spdlog::level::trace))
                {
                    auto str = WCharUtils::ToString(m_pCompStr->szStr);
                    log_debug("IME Composition String: {}", str.c_str());
                }
            }
            if (GetCompStr(hIMC, compFlag, GCS_RESULTSTR, m_pCompResult))
            {
                SendResultStringToSkyrim();
                if (spdlog::should_log(spdlog::level::trace))
                {
                    auto str = WCharUtils::ToString(m_pCompResult->szStr);
                    log_trace("IME Composition Result String: {}", str.c_str());
                }
            }
        }

        void ImeUI::QueryAllInstalledIME()
        {
            log_info("Query os installed IME...");
            m_imeProfiles.clear();
            _tsetlocale(LC_ALL, _T(""));
            LangProfileUtil::LoadIme(m_imeProfiles);

            LangProfile engProfile = {};
            ZeroMemory(&engProfile, sizeof(engProfile));
            engProfile.clsid       = CLSID_NULL;
            engProfile.langid      = LANGID_ENG; // english keyboard
            engProfile.guidProfile = GUID_NULL;
            engProfile.desc        = std::string("ENG");
            m_imeProfiles.push_back(engProfile);
        }

        void ImeUI::SendResultStringToSkyrim()
        {
            if (!IsEnabled())
            {
                return;
            }

            log_debug("Ready result string to Skyrim...");
            auto       *pInterfaceStrings = RE::InterfaceStrings::GetSingleton();
            auto       *pFactoryManager   = RE::MessageDataFactoryManager::GetSingleton();
            const auto *pFactory =
                pFactoryManager->GetCreator<RE::BSUIScaleformData>(pInterfaceStrings->bsUIScaleformData);

            // Start send message
            RE::BSFixedString menuName = pInterfaceStrings->topMenu;
            for (size_t i = 0; i < m_pCompResult->dwSize; i++)
            {
                uint32_t const code = m_pCompResult->szStr[i];
                if (code == ASCII_GRAVE_ACCENT || code == ASCII_MIDDLE_DOT)
                {
                    continue;
                }
                auto *pCharEvent            = new GFxCharEvent(code, 0);
                auto *pScaleFormMessageData = (pFactory != nullptr) ? pFactory->Create() : nullptr;
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
                LONG bufLen = ImmGetCompositionStringW(hIMC, static_cast<DWORD>(flagToCheck), (void *)nullptr,
                                                       static_cast<DWORD>(0));
                if (bufLen > 0)
                {
                    if (pWcharBuf->TryReAlloc(bufLen + 2))
                    {
                        ImmGetCompositionStringW(hIMC, static_cast<DWORD>(flagToCheck), pWcharBuf->szStr, bufLen);
                        DWORD const size       = bufLen / sizeof(WCHAR);
                        pWcharBuf->szStr[size] = '\0';
                        pWcharBuf->dwSize      = size;
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

            if (m_imeState.none(ImeState::IN_CANDCHOOSEN, ImeState::IN_COMPOSITION))
            {
                return;
            }

            ImGui::Begin("SimpleIME", (bool *)false, windowFlags);

            ImGui::SameLine();
            RenderCompWindow(m_pCompStr);

            ImGui::Separator();
            // render ime status window: language,
            if (m_imeState.any(ImeState::IN_CANDCHOOSEN))
            {
                RenderCandWindows();
            }
            ImGui::End();
        }

        void ImeUI::ShowToolWindow()
        {
            toolWindowFlags &= ~ImGuiWindowFlags_NoInputs;
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

            ImGui::Begin(TOOL_WINDOW_NAME.data(), &m_showToolWindow, toolWindowFlags);
            auto       &profile        = m_imeProfiles[langProfileSelected];
            const char *previewImeName = profile.desc.c_str();
            if (ImGui::Button("\xf0\x9f\x93\x8c"))
            {
                toolWindowFlags |= ImGuiWindowFlags_NoInputs;
                m_pinToolWindow                = true;
                ImGui::GetIO().MouseDrawCursor = false;
            }
            ImGui::SameLine();
            if (ImGui::BeginCombo("###InstalledIME", previewImeName))
            {
                for (size_t idx = 0; idx < m_imeProfiles.size(); idx++)
                {
                    auto       langProfile = m_imeProfiles[idx];
                    bool const isSelected  = (langProfileSelected == idx);
                    auto       label       = fmt::format("{}##{}", langProfile.desc, idx);
                    if (ImGui::Selectable(label.c_str()))
                    {
                        langProfileSelected = idx;
                        LangProfileUtil::ActivateProfile(langProfile);
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            if (m_imeState.all(ImeState::IN_ALPHANUMERIC))
            {
                ImGui::Text("ENG");
                ImGui::SameLine();
            }
            if (!m_pinToolWindow && !ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
            {
                ShowToolWindow();
            }
            ImGui::End();
        }

        static const ImU32 HIGHLIGHT_TEXT_COLOR{IM_COL32(93, 199, 255, 255)};

        void               ImeUI::RenderCompWindow(WcharBuf *compStrBuf)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, HIGHLIGHT_TEXT_COLOR);
            if (compStrBuf->IsEmpty())
            {
                ImGui::Dummy({10, ImGui::GetTextLineHeight()});
            }
            else
            {
                auto str = WCharUtils::ToString(compStrBuf->szStr);
                ImGui::Text("%s", str.c_str());
            }
            ImGui::PopStyleColor(1);
        }

        void ImeUI::RenderCandWindows() const
        {
            DWORD index = 0;
            for (auto &imeCandidate : m_imeCandidates)
            {
                index++;
                if (imeCandidate == nullptr)
                {
                    continue;
                }
                auto candList = imeCandidate->getCandList();
                index         = 0;
                for (const auto &item : candList)
                {
                    std::string const fmt = std::format("{} {}", index + 1, item);
                    if (index == imeCandidate->getDwSelecttion())
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, HIGHLIGHT_TEXT_COLOR);
                    }
                    ImGui::Text("%s", fmt.c_str());
                    if (index == imeCandidate->getDwSelecttion())
                    {
                        ImGui::PopStyleColor();
                    }
                    ImGui::SameLine();
                    index++;
                }
            }
        }

        void ImeUI::UpdateLanguage()
        {
            log_info("Update Input Language...");
            HKL          keyboard_layout = ::GetKeyboardLayout(0);
            LANGID const langId          = LOWORD(HandleToUlong(keyboard_layout));
            WCHAR        localeName[LOCALE_NAME_MAX_LENGTH];
            LCIDToLocaleName(MAKELCID(langId, SORT_DEFAULT), localeName, LOCALE_NAME_MAX_LENGTH, 0);

            // Retrieve keyboard code page, required for handling of non-Unicode Windows.
            LCID const keyboard_lcid = MAKELCID(HIWORD(keyboard_layout), SORT_DEFAULT);
            if (::GetLocaleInfoA(keyboard_lcid, (LOCALE_RETURN_NUMBER | LOCALE_IDEFAULTANSICODEPAGE),
                                 reinterpret_cast<LPSTR>(&keyboardCodePage), sizeof(keyboardCodePage)) == 0)
            {
                keyboardCodePage = CP_ACP; // Fallback to default ANSI code page when fails.
            }
        }

        void ImeUI::UpdateActiveLangProfile()
        {
            GUID activeGuid = {};
            if (LangProfileUtil::LoadActiveIme(activeGuid))
            {
                for (size_t idx = 0; idx < m_imeProfiles.size(); idx++)
                {
                    if (activeGuid == m_imeProfiles[idx].guidProfile)
                    {
                        auto str = m_imeProfiles[idx].desc;
                        log_debug("Active language profile: {}", str.c_str());
                        langProfileSelected = idx;
                        return;
                    }
                }
            }
        }

        auto ImeUI::ImeNotify(HWND hwnd, WPARAM wParam, LPARAM lParam) -> bool
        {
            log_debug("ImeNotify {:#x}", wParam);
            switch (wParam)
            {
                case IMN_SETCANDIDATEPOS:
                case IMN_OPENCANDIDATE: {
                    m_imeState.set(ImeState::IN_CANDCHOOSEN);
                    auto const context = ImmContextGuard(hwnd);
                    if (context.get())
                    {
                        OpenCandidate(context.get(), lParam);
                    }
                    return true;
                }
                case IMN_CLOSECANDIDATE: {
                    m_imeState.reset(ImeState::IN_CANDCHOOSEN);
                    CloseCandidate(lParam);
                    return true;
                }
                case IMN_CHANGECANDIDATE: {
                    HIMC hIMC = ImmGetContext(hwnd);
                    if (hIMC != nullptr)
                    {
                        ChangeCandidate(hIMC, lParam);
                        ImmReleaseContext(hwnd, hIMC);
                    }
                    return true;
                }
                case IMN_SETCONVERSIONMODE: {
                    HIMC hIMC = ImmGetContext(hwnd);
                    if (hIMC != nullptr)
                    {
                        UpdateConversionMode(hIMC);
                        ImmReleaseContext(hwnd, hIMC);
                    }
                    return true;
                }
                case IMN_SETOPENSTATUS: {
                    HIMC hIMC = ImmGetContext(hwnd);
                    if (hIMC != nullptr)
                    {
                        OnSetOpenStatus(hIMC);
                        ImmReleaseContext(hwnd, hIMC);
                    }
                    return true;
                }
                default:
                    break;
            }
            return false;
        }

        void ImeUI::UpdateConversionMode(HIMC hIMC)
        {
            DWORD conversion = 0;
            DWORD sentence   = 0;
            if (ImmGetConversionStatus(hIMC, &conversion, &sentence) != 0)
            {
                switch (conversion & IME_CMODE_LANGUAGE)
                {
                    case IME_CMODE_ALPHANUMERIC:
                        m_imeState.set(ImeState::IN_ALPHANUMERIC);
                        log_debug("CMODE:ALPHANUMERIC, Disable IME");
                        break;
                    default:
                        m_imeState.reset(ImeState::IN_ALPHANUMERIC);
                        break;
                }
            }
        }

        void ImeUI::OnSetOpenStatus(HIMC hIMC)
        {
            if (ImmGetOpenStatus(hIMC) != 0)
            {
                m_imeState.set(ImeState::OPEN);
                UpdateConversionMode(hIMC);
            }
            else
            {
                m_imeState.reset(ImeState::OPEN);
                langProfileSelected = LAST_SIZE(m_imeProfiles);
            }
            UpdateActiveLangProfile();
        }

        // when ImeUi enabled, the Game Input will be block and composition string will be sent
        // to Game text field;
        auto ImeUI::IsEnabled() const -> bool
        {
            return m_imeState.any(ImeState::IME_ALL);
        }

        auto ImeUI::GetImeState() const -> Enumeration<ImeState>
        {
            return m_imeState;
        }

        void ImeUI::OpenCandidate(HIMC hIMC, LPARAM candListFlag)
        {
            for (size_t index = 0; index < CandWindowProp::MAX_COUNT; ++index)
            {
                if ((candListFlag & (ONE << index)) == 0U)
                {
                    continue;
                }
                ChangeCandidateAt(hIMC, index);
            }
        }

        void ImeUI::CloseCandidate(LPARAM candListFlag)
        {
            for (uint8_t index = 0; index < CandWindowProp::MAX_COUNT; ++index)
            {
                if ((candListFlag & (ONE << index)) != 0U)
                {
                    auto &imeCandiDate = m_imeCandidates.at(index);
                    if (imeCandiDate == nullptr)
                    {
                        continue;
                    }

                    log_debug("Close candidate window #{}", index);
                    imeCandiDate.reset();
                }
            }
        }

        void ImeUI::ChangeCandidate(HIMC hIMC, LPARAM candListFlag)
        {
            DWORD dwIndex = 0;
            for (; dwIndex < CandWindowProp::MAX_COUNT; dwIndex++)
            {
                if ((candListFlag & (ONE << dwIndex)) != 0U)
                {
                    break;
                }
            }
            if (dwIndex == CandWindowProp::MAX_COUNT)
            {
                return;
            }

            ChangeCandidateAt(hIMC, dwIndex);
        }

        void ImeUI::ChangeCandidateAt(HIMC hIMC, DWORD dwIndex)
        {
            log_debug("Update candidate window #{}", dwIndex);
            DWORD bufLen = ImmGetCandidateListW(hIMC, dwIndex, nullptr, 0);
            if (bufLen == 0)
            {
                return;
            }
            HGLOBAL hGlobal = GlobalAlloc(LPTR, bufLen);
            if (hGlobal == nullptr)
            {
                log_warn("Global alloc {} failed.", bufLen);
                return;
            }
            auto &currentCandidate = m_imeCandidates.at(dwIndex);
            if (currentCandidate == nullptr)
            {
                currentCandidate = std::make_unique<ImeCandidateList>();
            }
            LPCANDIDATELIST lpCandList = static_cast<LPCANDIDATELIST>(GlobalLock(hGlobal));
            if (lpCandList == nullptr)
            {
                log_error("Candidate window #{} alloc memory failed.", dwIndex);
                GlobalFree(hGlobal);
                currentCandidate.reset();
                return;
            }
            ImmGetCandidateListW(hIMC, dwIndex, lpCandList, bufLen);
            currentCandidate->Flush(lpCandList);
            log_debug("Candidate window #{}, count: {}", dwIndex, lpCandList->dwCount);
            GlobalUnlock(hGlobal);
            GlobalFree(hGlobal);
        }

        ImeUI::WcharBuf::WcharBuf(HANDLE heap, DWORD initSize)
            : szStr((LPWSTR)HeapAlloc(heap, HEAP_GENERATE_EXCEPTIONS, initSize)), dwCapacity(initSize), dwSize(0),
              m_heap(heap)
        {

            szStr[0] = '\0'; // should pass because HeapAlloc use HEAP_GENERATE_EXCEPTIONS
        }

        ImeUI::WcharBuf::~WcharBuf()
        {
            // HeapFree(m_heap, 0, szStr;
        }

        auto ImeUI::WcharBuf::TryReAlloc(DWORD bufLen) -> bool
        {
            if (bufLen > dwCapacity)
            {
                LPVOID hMem = static_cast<LPWSTR>(HeapReAlloc(m_heap, 0, szStr, bufLen));
                if (hMem == nullptr)
                {
                    log_error("Try re-alloc to {} failed", bufLen);
                    return false;
                }
                dwCapacity = bufLen;
                szStr      = static_cast<LPWSTR>(hMem);
            }
            return true;
        }

        void ImeUI::WcharBuf::Clear()
        {
            szStr[0] = '\0';
            dwSize   = 0;
        }

        auto ImeUI::WcharBuf::IsEmpty() const -> bool
        {
            return dwSize == 0;
        }

        void ImeCandidateList::Flush(LPCANDIDATELIST lpCandList)
        {
            setPageSize(lpCandList->dwPageSize);
            DWORD dwStartIndex = lpCandList->dwPageStart;
            DWORD dwEndIndex   = dwStartIndex + dwPageSize;
            dwEndIndex         = std::min(dwEndIndex, lpCandList->dwCount);
            candList.clear();
            auto pCandidates = reinterpret_cast<std::uintptr_t>(lpCandList);
            for (; dwStartIndex < dwEndIndex; dwStartIndex++)
            {
                auto        address         = pCandidates + lpCandList->dwOffset[dwStartIndex];
                auto        candidate       = std::wstring_view(reinterpret_cast<WCHAR *>(address));
                auto        AnsiSizeInBytes = WCharUtils::CharLength(candidate.data());
                std::string ansiStr(AnsiSizeInBytes, 0);
                WCharUtils::ToString(candidate.data(), ansiStr.data(), AnsiSizeInBytes);
                candList.push_back(ansiStr);
            }
            dwSelecttion = lpCandList->dwSelection % dwPageSize;
        }
    } // namespace  SimpleIME
} // namespace LIBC_NAMESPACE_DECL