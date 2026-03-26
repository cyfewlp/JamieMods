//
// Created by jamie on 2025/5/12.
//

#include "popup/Popup.h"

#include <list>
#include <memory>

namespace SosGui
{
struct Context
{
    std::list<std::unique_ptr<Popup::ModalPopup>> popupList;

    void SetIconFont(ImFont *font) { iconFont = font; }

    auto GetIconFont() const -> ImFont * { return iconFont; }

private:
    ImFont *iconFont = nullptr;
};
} // namespace SosGui
