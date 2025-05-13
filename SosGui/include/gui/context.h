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
constexpr auto *NF_MD_REFRESH      = "\xf3\xb0\x91\x90"; // nf-md-refresh \udb81\udc50
constexpr auto *NF_OCT_HEART       = "\xe2\x99\xa5";     // nf-oct-heart \u2665
constexpr auto *NF_OCT_HEART_FILL  = "\xef\x93\xa1";     //\uf4e1
constexpr auto *NF_MD_HEART_PLUS   = "\xf3\xb1\x90\xae"; //\udb85\udc2e
constexpr auto *NF_MD_HEART_REMOVE = "\xf3\xb1\x90\xb0"; //\udb85\udc30
constexpr auto *NF_OCT_GEAR        = "\xef\x90\xa3";     // nf-oct-heart \uf423
constexpr auto *NF_FA_EDIT         = "\xef\x81\x84";     //\uf044
constexpr auto *NF_FA_SHIRT        = "\xee\xb8\x9c";     //\uee1c

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
