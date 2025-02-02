//
// Created by jamie on 25-1-21.
//

#include "ImeUI.h"
#include "ImeApp.h"
#include "WCharUtils.h"
#include <locale.h>
#include <tchar.h>

#include <algorithm>

#define LAST_SIZE(vector) vector.size() - 1

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {

        static size_t     langProfileSelected = 0;
        constexpr ULONG64 ONE                 = 1;

        static void       HelpMarker(const char *desc)
        {
            ImGui::Text("\xe2\x9d\x94"); // white ?
            if (ImGui::BeginItemTooltip())
            {
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0F);
                ImGui::TextUnformatted(desc);
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        }

        ImeUI::ImeUI()
        {
            _tsetlocale(LC_ALL, _T(""));
            m_pHeap      = HeapCreate(HEAP_GENERATE_EXCEPTIONS, IMEUI_HEAP_INIT_SIZE, IMEUI_HEAP_MAX_SIZE);
            m_compStr    = new WcharBuf(m_pHeap, WCHAR_BUF_INIT_SIZE);
            m_compResult = new WcharBuf(m_pHeap, WCHAR_BUF_INIT_SIZE);
            m_fontSize   = ImGui::GetFontSize();
        }

        ImeUI::~ImeUI()
        {
            HeapDestroy(m_pHeap);
            m_pHeap = nullptr;
            delete m_compStr;
            delete m_compResult;
            for (int i = 0; i < CAND_MAX_WINDOW_COUNT; ++i)
            {
                if (m_imeCandidates[i] != nullptr)
                {
                    delete m_imeCandidates[i];
                    m_imeCandidates[i] = nullptr;
                }
            }
        }

        void ImeUI::StartComposition()
        {
            logv(trace, "IME Start Composition");
            m_compStr->Clear();
            m_compResult->Clear();
            m_imeState |= IME_IN_COMPOSITION;
        }

        void ImeUI::EndComposition()
        {
            logv(trace, "IME End Composition");
            m_compStr->Clear();
            m_compResult->Clear();
            m_imeState.reset(IME_IN_COMPOSITION);
        }

        void ImeUI::CompositionString(HIMC hIMC, LPARAM compFlag)
        {
            if (GetCompStr(hIMC, compFlag, GCS_COMPSTR, m_compStr))
            {
                if (spdlog::should_log(spdlog::level::trace))
                {
                    auto str = WCharUtils::ToString(m_compStr->szStr);
                    logv(debug, "IME Composition String: {}", str.c_str());
                }
            }
            if (GetCompStr(hIMC, compFlag, GCS_RESULTSTR, m_compResult))
            {
                SendResultString();
                if (spdlog::should_log(spdlog::level::trace))
                {
                    auto str = WCharUtils::ToString(m_compResult->szStr);
                    logv(debug, "IME Composition Result String: {}", str.c_str());
                }
            }
        }

        void ImeUI::QueryAllInstalledIME()
        {
            logv(info, "Query os installed IME...");
            m_imeProfiles.clear();
            _tsetlocale(LC_ALL, _T(""));
            langProfileUtil.LoadIme(m_imeProfiles);

            LangProfile engProfile = {};
            ZeroMemory(&engProfile, sizeof(engProfile));
            engProfile.clsid       = CLSID_NULL;
            engProfile.langid      = LANGID_ENG; // english keyboard
            engProfile.guidProfile = GUID_NULL;
            engProfile.desc        = std::string("ENG");
            m_imeProfiles.push_back(engProfile);
        }

        void ImeUI::SendResultString()
        {
            auto &io = ImGui::GetIO();
            for (size_t i = 0; i < m_compResult->dwSize; i++)
            {
                io.AddInputCharacterUTF16(m_compResult->szStr[i]);
            }
            SendResultStringToSkyrim();
        }

        void ImeUI::SendResultStringToSkyrim()
        {
            if (!IsEnabled())
            {
                return;
            }

            logv(debug, "Ready result string to Skyrim...");
            auto       *pInterfaceStrings = RE::InterfaceStrings::GetSingleton();
            auto       *pFactoryManager   = RE::MessageDataFactoryManager::GetSingleton();
            const auto *pFactory =
                pFactoryManager->GetCreator<RE::BSUIScaleformData>(pInterfaceStrings->bsUIScaleformData);

            // Start send message
            RE::BSFixedString menuName = pInterfaceStrings->topMenu;
            for (size_t i = 0; i < m_compResult->dwSize; i++)
            {
                uint32_t code = m_compResult->szStr[i];
                if (code == ASCII_GRAVE_ACCENT || code == ASCII_MIDDLE_DOT)
                {
                    continue;
                }
                auto *pCharEvent            = new GFxCharEvent(code, 0);
                auto *pScaleFormMessageData = (pFactory != nullptr) ? pFactory->Create() : nullptr;
                if (pScaleFormMessageData == nullptr)
                {
                    logv(err, "Unable create BSTDerivedCreator.");
                    return;
                }
                pScaleFormMessageData->scaleformEvent = pCharEvent;
                logv(debug, "send code {:#x} to Skyrim", code);
                RE::UIMessageQueue::GetSingleton()->AddMessage(menuName, RE::UI_MESSAGE_TYPE::kScaleformEvent,
                                                               pScaleFormMessageData);
            }
        }

        bool ImeUI::GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, WcharBuf *pWcharBuf)
        {
            if ((compFlag & flagToCheck) != 0)
            {
                LONG bufLen;
                if ((bufLen = ImmGetCompositionStringW(hIMC, (DWORD)flagToCheck, (void *)nullptr, (DWORD)0)) > 0)
                {
                    if (pWcharBuf->TryReAlloc(bufLen + 2))
                    {
                        ImmGetCompositionStringW(hIMC, (DWORD)flagToCheck, pWcharBuf->szStr, bufLen);
                        DWORD size             = bufLen / sizeof(WCHAR);
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

            if (m_imeState.none(IME_IN_CANDCHOOSEN, IME_IN_COMPOSITION))
            {
                return;
            }

            ImGui::Begin("SimpleIME", (bool *)false, windowFlags);

            ImGui::SameLine();
            RenderCompWindow(m_compStr);

            ImGui::Separator();
            // render ime status window: language,
            if (m_imeState.any(IME_IN_CANDCHOOSEN))
            {
                ImVec2 pos{CAND_WINDOW_OFFSET_X, ImGui::GetTextLineHeight() + CAND_WINDOW_OFFSET_Y};
                RenderCandWindow(pos);
            }
            ImGui::End();
        }

        void ImeUI::ShowToolWindow()
        {
            m_showToolWindow = !m_showToolWindow;
            auto &io         = ImGui::GetIO();
            if (!m_showToolWindow)
            {
                io.MouseDrawCursor          = false;
                io.ConfigNavMoveSetMousePos = false;
                m_imeState.reset(IME_UI_FOCUSED);
                return;
            }
            io.MouseDrawCursor          = true;
            io.ConfigNavMoveSetMousePos = true;
            m_imeState.set(IME_UI_FOCUSED);
        }

        void ImeUI::RenderToolWindow()
        {
            if (!m_showToolWindow)
            {
                return;
            }

            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize;
            windowFlags |= ImGuiWindowFlags_NoCollapse;
            ImGui::Begin("ToolWindow##SimpleIME", &m_showToolWindow, windowFlags);
            auto       &profile        = m_imeProfiles[langProfileSelected];
            const char *previewImeName = profile.desc.c_str();
            if (ImGui::BeginCombo("###InstalledIME", previewImeName))
            {
                for (size_t idx = 0; idx < m_imeProfiles.size(); idx++)
                {
                    auto langProfile = m_imeProfiles[idx];
                    bool isSelected  = (langProfileSelected == idx);
                    auto label       = fmt::format("{}##{}", langProfile.desc, idx);
                    if (ImGui::Selectable(label.c_str()))
                    {
                        langProfileSelected = idx;
                        langProfileUtil.ActivateProfile(langProfile);
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();

            HelpMarker(R"(By default, keyboard in exclusive access by Skyrim.
So all IME & almost keyboard message is disabled.)");
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Text("Keyboard: ");
            ImGui::SameLine();

            static ImVec4 red{0.9F, 0.0F, 0.5F, 1.0F};
            static ImVec4 green{0.0F, 1.0F, 0.0F, 1.0F};
            if (!ImeApp::CheckAppState())
            {
                ImGui::TextColored(red, "No");
            }
            else
            {
                ImGui::TextColored(green, "Ok");
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset Keyboard"))
            {
                ImeApp::ResetExclusiveMode();
            }
            ImGui::EndGroup();
            ImGui::SetItemTooltip("Try reset to non-exclusive keyboard.");
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

        void ImeUI::RenderCandWindow(ImVec2 &wndPos) const
        {
            ImVec2           childSize(0.0F, m_fontSize * 2);
            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoInputs;
            windowFlags |= ImGuiWindowFlags_NoScrollbar;
            windowFlags |= ImGuiWindowFlags_NoBackground;
            ImGuiChildFlags childFlags = ImGuiChildFlags_AutoResizeY;
            for (int index = 0; index < CAND_MAX_WINDOW_COUNT; ++index)
            {
                auto *imeCandidate = m_imeCandidates[index];
                if (imeCandidate == nullptr)
                {
                    continue;
                }
                auto size   = imeCandidate->candList.size();
                childSize.x = imeCandidate->lineWidth + CAND_WINDOW_PADDING;
                childSize.y = m_fontSize;
                ImGui::SetNextWindowPos(wndPos);
                ImGui::BeginChild(fmt::format("CandidateWindow##{}", index).c_str(), childSize, childFlags,
                                  windowFlags);
                for (DWORD strIndex = 0; strIndex < size; ++strIndex)
                {
                    std::string fmt = std::format("{} {}", strIndex + 1, imeCandidate->candList[strIndex]);
                    if (strIndex == imeCandidate->dwSelecttion)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, HIGHLIGHT_TEXT_COLOR);
                    }
                    ImGui::Text("%s", fmt.c_str());
                    if (strIndex == imeCandidate->dwSelecttion)
                    {
                        ImGui::PopStyleColor();
                    }
                    ImGui::SameLine();
                }
                ImGui::EndChild();
            }
        }

        void ImeUI::UpdateLanguage()
        {
            logv(info, "Update Input Language...");
            HKL    keyboard_layout = ::GetKeyboardLayout(0);
            LANGID langId          = LOWORD(HandleToUlong(keyboard_layout));
            WCHAR  localeName[LOCALE_NAME_MAX_LENGTH];
            LCIDToLocaleName(MAKELCID(langId, SORT_DEFAULT), localeName, LOCALE_NAME_MAX_LENGTH, 0);

            // Retrieve keyboard code page, required for handling of non-Unicode Windows.
            LCID keyboard_lcid = MAKELCID(HIWORD(keyboard_layout), SORT_DEFAULT);
            if (::GetLocaleInfoA(keyboard_lcid, (LOCALE_RETURN_NUMBER | LOCALE_IDEFAULTANSICODEPAGE),
                                 (LPSTR)&keyboardCodePage, sizeof(keyboardCodePage)) == 0)
            {
                keyboardCodePage = CP_ACP; // Fallback to default ANSI code page when fails.
            }
        }

        void ImeUI::UpdateActiveLangProfile()
        {
            GUID activeGuid = {};
            if (langProfileUtil.LoadActiveIme(activeGuid))
            {
                for (size_t idx = 0; idx < m_imeProfiles.size(); idx++)
                {
                    if (activeGuid == m_imeProfiles[idx].guidProfile)
                    {
                        auto str = m_imeProfiles[idx].desc;
                        logv(debug, "Active language profile: {}", str.c_str());
                        langProfileSelected = idx;
                        return;
                    }
                }
            }
        }

        bool ImeUI::ImeNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
        {
            logv(debug, "ImeNotify {:#x}", wParam);
            switch (wParam)
            {
                case IMN_SETCANDIDATEPOS:
                case IMN_OPENCANDIDATE:
                    m_imeState |= IME_IN_CANDCHOOSEN;
                    MAKE_CONTEXT(hwnd, OpenCandidate, lParam);
                    return true;
                case IMN_CLOSECANDIDATE:
                    m_imeState.reset(IME_IN_CANDCHOOSEN);
                    CloseCandidate(lParam);
                    return true;
                case IMN_CHANGECANDIDATE:
                    MAKE_CONTEXT(hwnd, ChangeCandidate, lParam);
                    return true;
                case IMN_SETCONVERSIONMODE:
                    MAKE_CONTEXT(hwnd, UpdateConversionMode);
                    break;
                case IMN_SETOPENSTATUS:
                    MAKE_CONTEXT(hwnd, OnSetOpenStatus);
                    break;
            }
            return false;
        }

        void ImeUI::UpdateConversionMode(HIMC hIMC)
        {
            DWORD conversion;
            DWORD sentence;
            if (ImmGetConversionStatus(hIMC, &conversion, &sentence) != 0)
            {
                switch (conversion & IME_CMODE_LANGUAGE)
                {
                    case IME_CMODE_ALPHANUMERIC:
                        m_imeState |= IME_IN_ALPHANUMERIC;
                        logv(debug, "CMODE:ALPHANUMERIC, Disable IME");
                        break;
                    default:
                        m_imeState.reset(IME_IN_ALPHANUMERIC);
                        break;
                }
            }
        }

        void ImeUI::OnSetOpenStatus(HIMC hIMC)
        {
            if (ImmGetOpenStatus(hIMC) != 0)
            {
                m_imeState |= IME_OPEN;
                DWORD conversion;
                DWORD sentence;
                if (ImmGetConversionStatus(hIMC, &conversion, &sentence) != 0)
                {
                    switch (conversion & IME_CMODE_LANGUAGE)
                    {
                        case IME_CMODE_ALPHANUMERIC:
                            m_imeState |= IME_IN_ALPHANUMERIC;
                            logv(debug, "CMODE:ALPHANUMERIC, Disable IME");
                            break;
                        default:
                            m_imeState.reset(IME_IN_ALPHANUMERIC);
                            break;
                    }
                }
            }
            else
            {
                m_imeState.reset(IME_OPEN);
                langProfileSelected = LAST_SIZE(m_imeProfiles);
            }
            UpdateActiveLangProfile();
        }

        // when ImeUi enabled, the Game Input will be block and composition string will be sent
        // to Game text field;
        bool ImeUI::IsEnabled() const
        {
            return m_imeState.any(IME_ALL);
        }

        SKSE::stl::enumeration<ImeState> ImeUI::GetImeState() const
        {
            return m_imeState;
        }

        void ImeUI::OpenCandidate(HIMC hIMC, LPARAM candListFlag)
        {
            DWORD           bufLen;
            LPCANDIDATELIST lpCandList = nullptr;
            int             width      = 0;
            bufLen                     = ImmGetCandidateListW(hIMC, 0, nullptr, 0);
            logv(debug, "Candidate index 0 buf len {}", bufLen);
            for (int index = 0; index < CAND_MAX_WINDOW_COUNT; ++index)
            {
                if ((candListFlag & ONE << index) == 0U)
                {
                    continue;
                }
                if ((bufLen = ImmGetCandidateListW(hIMC, index, nullptr, 0)) == 0U)
                {
                    break;
                }

                logv(debug, "Open candidate window #{}", index);
                auto *imeCandidate = m_imeCandidates[index];
                if (imeCandidate == nullptr)
                {
                    m_imeCandidates[index] = new ImeCandidate();
                    imeCandidate           = m_imeCandidates[index];
                }
                HGLOBAL hGlobal;

                if ((hGlobal = GlobalAlloc(LPTR, bufLen)) == nullptr)
                {
                    break;
                }
                if ((lpCandList = (LPCANDIDATELIST)GlobalLock(hGlobal)) == nullptr)
                {
                    GlobalFree(hGlobal);
                    delete m_imeCandidates[index];
                    m_imeCandidates[index] = nullptr;
                    logv(err, "Candidate window #{} alloc memory failed.", index);
                    break;
                }
                ImmGetCandidateListW(hIMC, index, lpCandList, bufLen);
                UpdateImeCandidate(imeCandidate, lpCandList);
                logv(debug, "Candidate window #{}: width: , count: {}", index, width, lpCandList->dwCount);
                GlobalUnlock(hGlobal);
                GlobalFree(hGlobal);
            }
        }

        void ImeUI::CloseCandidate(LPARAM candListFlag)
        {
            for (uint8_t index = 0; index < CAND_MAX_WINDOW_COUNT; ++index)
            {
                if ((candListFlag & (ONE << index)) != 0U)
                {
                    auto *imeCandiDate = m_imeCandidates[index];
                    if (imeCandiDate == nullptr)
                    {
                        continue;
                    }

                    logv(debug, "Close candidate window #{}", index);
                    imeCandiDate->candList.clear();
                    delete imeCandiDate;
                    m_imeCandidates[index] = nullptr;
                }
            }
        }

        void ImeUI::ChangeCandidate(HIMC hIMC, LPARAM candListFlag)
        {
            DWORD bufLen;
            DWORD dwIndex = 0;
            for (; dwIndex < CAND_MAX_WINDOW_COUNT; dwIndex++)
            {
                if ((candListFlag & (ONE << dwIndex)) != 0U)
                {
                    break;
                }
            }
            if (dwIndex == CAND_MAX_WINDOW_COUNT)
            {
                return;
            }

            logv(debug, "Update candidate window #{}", dwIndex);
            if ((bufLen = ImmGetCandidateListW(hIMC, dwIndex, nullptr, 0)) != 0U)
            {
                HGLOBAL hGlobal;
                if ((hGlobal = GlobalAlloc(LPTR, bufLen)) == nullptr)
                {
                    return;
                }
                if (m_imeCandidates[dwIndex] == nullptr)
                {
                    m_imeCandidates[dwIndex] = new ImeCandidate();
                }
                LPCANDIDATELIST lpCandList = nullptr;
                if ((lpCandList = (LPCANDIDATELIST)GlobalLock(hGlobal)) == nullptr)
                {
                    GlobalFree(hGlobal);
                    delete m_imeCandidates[dwIndex];
                    m_imeCandidates[dwIndex] = nullptr;
                    return;
                }

                ImmGetCandidateListW(hIMC, dwIndex, lpCandList, bufLen);
                UpdateImeCandidate(m_imeCandidates[dwIndex], lpCandList);
                logv(debug, "Candidate window #{}: width: , count: {}", dwIndex, lpCandList->dwCount);
                GlobalUnlock(hGlobal);
                GlobalFree(hGlobal);
            }
        }

        void ImeUI::UpdateImeCandidate(ImeCandidate *candidate, LPCANDIDATELIST lpCandList) const
        {
            candidate->dwNumPerPage =
                (lpCandList->dwPageSize == 0U) ? CAND_DEFAULT_NUM_PER_PAGE : lpCandList->dwPageSize;
            LPWCH lpwStr;
            DWORD dwStartIndex = lpCandList->dwPageStart;
            DWORD dwEndIndex   = dwStartIndex + candidate->dwNumPerPage;
            dwEndIndex         = std::min(dwEndIndex, lpCandList->dwCount);
            candidate->candList.clear();
            float lineWidth = 0;
            for (; dwStartIndex < dwEndIndex; dwStartIndex++)
            {
                lpwStr = (LPWCH)((LPSTR)lpCandList + lpCandList->dwOffset[dwStartIndex]);
                std::wstring wstring(lpwStr);

                auto         stdStr = WCharUtils::ToString(wstring);
                float        width  = ((float)(wstring.size() + 1) * m_fontSize) + CAND_PADDING;
                lineWidth += width;
                candidate->candList.push_back(stdStr);
            }
            candidate->lineWidth    = lineWidth;
            candidate->dwSelecttion = lpCandList->dwSelection % candidate->dwNumPerPage;
        }

        ImeUI::WcharBuf::WcharBuf(HANDLE heap, DWORD initSize)
        {
            szStr      = (LPWSTR)HeapAlloc(heap, HEAP_GENERATE_EXCEPTIONS, initSize);
            dwCapacity = initSize;
            dwSize     = 0;
            szStr[0]   = '\0'; // should pass because HeapAlloc use HEAP_GENERATE_EXCEPTIONS
            m_heap     = heap;
        }

        ImeUI::WcharBuf::~WcharBuf()
        {
            // HeapFree(m_heap, 0, szStr;
        }

        bool ImeUI::WcharBuf::TryReAlloc(DWORD bufLen)
        {
            if (bufLen > dwCapacity)
            {
                LPVOID hMem = (LPWSTR)HeapReAlloc(m_heap, 0, szStr, bufLen);
                if (hMem == nullptr)
                {
                    logv(err, "Try re-alloc to {} failed", bufLen);
                    return false;
                }
                dwCapacity = bufLen;
                szStr      = (LPWSTR)hMem;
            }
            return true;
        }

        void ImeUI::WcharBuf::Clear()
        {
            szStr[0] = '\0';
            dwSize   = 0;
        }

        bool ImeUI::WcharBuf::IsEmpty() const
        {
            return dwSize == 0;
        }
    }
}