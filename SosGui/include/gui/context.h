//
// Created by jamie on 2025/5/12.
//

#ifndef CONTEXT_H
#define CONTEXT_H

#include "common/config.h"
#include "gui/Popup.h"

#include <list>
#include <memory>

namespace LIBC_NAMESPACE_DECL
{
struct Context
{
    std::list<std::unique_ptr<Popup::ModalPopup>> popupList;

    static auto GetInstance() -> Context &
    {
        static Context instance;
        return instance;
    }

    void SetIconFont(ImFont *font)
    {
        iconFont = font;
    }

    auto GetIconFont() const -> ImFont *
    {
        return iconFont;
    }

private:
    ImFont *iconFont = nullptr;
};
}

#endif // CONTEXT_H
