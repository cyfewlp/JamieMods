//
// Created by jamie on 2025/5/11.
//

#ifndef IMGUISCOP_H
#define IMGUISCOP_H

#include "common/config.h"
#include "imgui.h"

namespace
LIBC_NAMESPACE_DECL
{
namespace ImGuiScope
{
struct ScopeObject
{
    bool state = false;
    ScopeObject()                               = default;
    ScopeObject(const ScopeObject &)            = delete;
    ScopeObject &operator=(const ScopeObject &) = delete;

    ScopeObject(ScopeObject &&other) noexcept : state(other.state)
    {
        other.state = false;
    }

    operator bool() const
    {
        return state;
    }
};

struct Combo : ScopeObject
{
    Combo(const std::string &&label, const std::string &&preview_value, const ImGuiComboFlags flags = 0)
    {
        state = ImGui::BeginCombo(label.c_str(), preview_value.c_str(), flags);
    }

    Combo(const char *label, const char *preview_value, const ImGuiComboFlags flags = 0)
    {
        state = ImGui::BeginCombo(label, preview_value, flags);
    }

    ~Combo()
    {
        if (state)
        {
            ImGui::EndCombo();
        }
    }
};

struct TabBar : ScopeObject
{
    TabBar(const char *str_id, ImGuiTabBarFlags flags = 0)
    {
        state = ImGui::BeginTabBar(str_id, flags);
    }

    ~TabBar()
    {
        if (state)
        {
            ImGui::EndTabBar();
        }
    }
};

struct TabItem : ScopeObject
{
    TabItem(const std::string &&label, bool *p_open = nullptr, const ImGuiTabItemFlags flags = 0)
    {
        state = ImGui::BeginTabItem(label.c_str(), p_open, flags);
    }

    ~TabItem()
    {
        if (state)
        {
            ImGui::EndTabItem();
        }
    }
};

struct MenuBar : ScopeObject
{
    MenuBar()
    {
        state = ImGui::BeginMenuBar();
    }

    ~MenuBar()
    {
        if (state)
        {
            ImGui::EndMenuBar();
        }
    }
};

struct Table : ScopeObject
{
    Table(const char *str_id, int columns, ImGuiTableFlags flags = 0, const ImVec2 &outer_size = ImVec2(0.0f, 0.0f),
          float       inner_width                                = 0.0f)
    {
        state = ImGui::BeginTable(str_id, columns, flags, outer_size, inner_width);
    }

    ~Table()
    {
        if (state)
        {
            ImGui::EndTable();
        }
    }
};

struct ItemWidth : ScopeObject
{
    ItemWidth(float itemWidth)
    {
        ImGui::PushItemWidth(itemWidth);
    }

    ~ItemWidth()
    {
        ImGui::PopItemWidth();
    }
};

struct FontSize : ScopeObject
{
    FontSize(float fontSize)
    {
        ImGui::PushFontSize(fontSize);
    }

    ~FontSize()
    {
        ImGui::PopFontSize();
    }
};


struct PushId : ScopeObject
{
    PushId(size_t id)
    {
        ImGui::PushID(id);
    }

    explicit PushId(const void *ptr)
    {
        ImGui::PushID(ptr);
    }

    ~PushId()
    {
        ImGui::PopID();
    }
};

struct Popup : ScopeObject
{
    Popup(const char *str_id = nullptr, ImGuiPopupFlags popup_flags = 1)
    {
        state = ImGui::BeginPopup(str_id, popup_flags);
    }

    ~Popup()
    {
        if (state)
        {
            ImGui::EndPopup();
        }
    }
};

struct PopupContextItem : ScopeObject
{
    PopupContextItem(const char *str_id = nullptr, ImGuiPopupFlags popup_flags = 1)
    {
        state = ImGui::BeginPopupContextItem(str_id, popup_flags);
    }

    ~PopupContextItem()
    {
        if (state)
        {
            ImGui::EndPopup();
        }
    }
};

struct Disabled : ScopeObject
{
    Disabled(bool condition)
    {
        ImGui::BeginDisabled(condition);
    }

    ~Disabled()
    {
        ImGui::EndDisabled();
    }
};

struct Group : ScopeObject
{
    Group()
    {
        ImGui::BeginGroup();
    }

    ~Group()
    {
        ImGui::EndGroup();
    }
};

struct Child : ScopeObject
{
    Child(const char *     str_id, const ImVec2 &size = ImVec2(0, 0), const ImGuiChildFlags child_flags = 0,
          ImGuiWindowFlags window_flags               = 0)
    {
        state = ImGui::BeginChild(str_id, size, child_flags, window_flags);
    }

    ~Child()
    {
        ImGui::EndChild();
    }
};
}
}

#endif //IMGUISCOP_H