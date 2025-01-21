#include <ImeUI.hpp>
#include <imgui.h>
#include <iostream>
#include <memory>
#include <utils.hpp>

class GFxCharEvent : public RE::GFxEvent
{
public:
    GFxCharEvent() = default;
    GFxCharEvent(UINT32 a_wcharCode, UINT8 a_keyboardIndex = 0)
        : GFxEvent(RE::GFxEvent::EventType::kCharEvent), wcharCode(a_wcharCode), keyboardIndex(a_keyboardIndex)
    {
    }
    // @members
    std::uint32_t wcharCode;     // 04
    std::uint32_t keyboardIndex; // 08
};
static_assert(sizeof(GFxCharEvent) == 0x0C);

Transmogrify::ImeUI::ImeUI()
{
    m_pHeap           = HeapCreate(HEAP_GENERATE_EXCEPTIONS, IMEUI_HEAP_INIT, IMEUI_HEAP_MAX);
    m_CompStr         = new WcharBuf(m_pHeap, 64);
    m_CompResult      = new WcharBuf(m_pHeap, 64);
    m_enableSkyrimIME = false;

    WCHAR lang[9];
    GetLocaleInfoEx(LOCALE_NAME_SYSTEM_DEFAULT, LOCALE_SISO639LANGNAME2, lang, 9);
    m_langNameStr.assign(utils::WideStrToStrUTF8(lang));
}

Transmogrify::ImeUI::~ImeUI()
{
    m_pHeap = nullptr;
    HeapDestroy(m_pHeap);
}

void Transmogrify::ImeUI::StartComposition()
{
    m_CompStr->Clear();
    m_CompResult->Clear();
}

void Transmogrify::ImeUI::EndComposition()
{
    m_CompStr->Clear();
    m_CompResult->Clear();
}

void Transmogrify::ImeUI::CompositionString(HWND hWnd, LPARAM compFlag)
{
    HIMC hIMC;
    if (!(hIMC = ImmGetContext(hWnd))) return;

    GetCompStr(hIMC, compFlag, GCS_COMPSTR, m_CompStr);
    if (GetCompStr(hIMC, compFlag, GCS_RESULTSTR, m_CompResult))
    {
        SendResultString();
    }

    ImmReleaseContext(hWnd, hIMC);
}

void Transmogrify::ImeUI::SendResultString()
{
    auto &io = ImGui::GetIO();
    for (size_t i = 0; i < m_CompResult->dwSize; i++)
    {
        io.AddInputCharacterUTF16(m_CompResult->szStr[i]);
    }
    SendResultStringToSkyrim();
}

void Transmogrify::ImeUI::SendResultStringToSkyrim()
{
    // send to SKSE
    if (!m_enableSkyrimIME) return;

    LOG(debug, "Ready result string to Skyrim...");
    auto pInterfaceStrings = RE::InterfaceStrings::GetSingleton();
    auto pFactoryManager   = RE::MessageDataFactoryManager::GetSingleton();
    auto pFactory          = pFactoryManager->GetCreator<RE::BSUIScaleformData>(pInterfaceStrings->bsUIScaleformData);

    // Start send message
    RE::BSFixedString menuName = pInterfaceStrings->topMenu;
    LOG(debug, "menu name: {}", menuName.c_str());
    for (size_t i = 0; i < m_CompResult->dwSize; i++)
    {
        // uint32_t      code                  = m_CompResult->szStr[i];
        uint32_t      code                  = 0x2639;
        GFxCharEvent *pCharEvent            = new GFxCharEvent(code, 0);
        auto          pScaleFormMessageData = pFactory ? pFactory->Create() : nullptr;
        if (pScaleFormMessageData == nullptr)
        {
            LOG(err, "Unable create BSTDerivedCreator.");
            return;
        }
        auto ui    = RE::UI::GetSingleton();
        auto movie = ui->GetMovieView(pInterfaceStrings->console);
        if (movie != NULL)
        {
            auto state = movie.get()->GetState(RE::GFxState::StateType::kClipboard);
            (void)0;
        }
        pScaleFormMessageData->scaleformEvent = pCharEvent;

        LOG(debug, "send code {} to Skyrim", code);
        RE::UIMessageQueue::GetSingleton()->AddMessage(menuName, RE::UI_MESSAGE_TYPE::kScaleformEvent,
                                                       pScaleFormMessageData);
    }
}

void Transmogrify::ImeUI::RenderCompWindow(WcharBuf *compStrBuf)
{
    if (compStrBuf->IsEmpty()) return;

    // ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32_WHITE);
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_WHITE);
    ImVec2 caretPos = ImVec2(g_caretPos[0], g_caretPos[1]);
    ImGui::SetNextWindowPos(caretPos);

    ImVec2           childSize   = {(compStrBuf->dwSize >> 1) * ImGui::GetFontSize(), ImGui::GetTextLineHeight()};
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration;
    windowFlags |= ImGuiWindowFlags_NoInputs;
    ImGui::BeginChild("CompositionWindow", childSize, 0, windowFlags);
    // render a underline
    ImGui::GetForegroundDrawList()->AddLine({caretPos.x, caretPos.y + childSize.y},
                                            {caretPos.x + childSize.x, caretPos.y + childSize.y},
                                            IM_COL32(125, 125, 125, 255)); // gray
    auto str = Transmogrify::utils::WideStrToStrUTF8(compStrBuf->szStr);
    ImGui::Text(str.c_str());
    ImGui::EndChild();
    ImGui::PopStyleColor(1);
}

bool Transmogrify::ImeUI::GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, WcharBuf *pWcharBuf)
{
    if (compFlag & flagToCheck)
    {
        LONG bufLen;
        if ((bufLen = ImmGetCompositionStringW(hIMC, (DWORD)flagToCheck, (void *)NULL, (DWORD)0)) > 0)
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

void Transmogrify::ImeUI::RenderCompStr()
{
    static char buf[64] = "";
    ImGui::Checkbox("Enable SkyrimIME", &m_enableSkyrimIME);
    ImGui::Text("%s", m_langNameStr.c_str());
    ImGui::SameLine();
    ImGui::InputText("Search", buf, IM_ARRAYSIZE(buf));

    // comp window
    ImGui::Text("Caret pos x: %f, y: %f", g_caretPos[0], g_caretPos[1]);
    RenderCompWindow(m_CompStr);
}

void Transmogrify::ImeUI::UpdateLanguage(HKL hkl)
{
    LANGID langId = LOWORD(HandleToUlong(hkl));
    WCHAR  localeName[LOCALE_NAME_MAX_LENGTH];
    LCIDToLocaleName(MAKELCID(langId, SORT_DEFAULT), localeName, LOCALE_NAME_MAX_LENGTH, 0);

    // Get the ISO abbreviated language name (for example, "eng").
    WCHAR lang[9];
    GetLocaleInfoEx(localeName, LOCALE_SISO639LANGNAME2, lang, 9);
    m_langNameStr.assign(utils::WideStrToStrUTF8(lang));
}

void Transmogrify::ImeUI::UpdateCaretPos(float x, float y)
{
    g_caretPos[0] = x;
    g_caretPos[1] = y;
}

bool Transmogrify::ImeUI::IsSkyrimIMEEnabled()
{
    return m_enableSkyrimIME;
}

Transmogrify::ImeUI::WcharBuf::WcharBuf(HANDLE heap, DWORD initSize)
{
    szStr      = (LPWSTR)HeapAlloc(heap, HEAP_GENERATE_EXCEPTIONS, initSize);
    dwCapacity = initSize;
    dwSize     = 0;
    szStr[0]   = '\0';
    m_heap     = heap;
}

Transmogrify::ImeUI::WcharBuf::~WcharBuf()
{
    // HeapFree(m_heap, 0, szStr;
}

bool Transmogrify::ImeUI::WcharBuf::TryReAlloc(DWORD bufLen)
{
    if (bufLen > dwCapacity)
    {
        LPVOID hMem = (LPWSTR)HeapReAlloc(m_heap, 0, szStr, bufLen);
        if (!hMem)
        {
            LOG(err, "Try realloc to {} failed", bufLen);
            return false;
        }
        dwCapacity = bufLen;
        szStr      = (LPWSTR)hMem;
    }
    return true;
}

void Transmogrify::ImeUI::WcharBuf::Clear()
{
    szStr[0] = '\0';
    dwSize   = 0;
}

bool Transmogrify::ImeUI::WcharBuf::IsEmpty()
{
    return dwSize == 0;
}
