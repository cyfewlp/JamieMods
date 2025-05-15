//
// Created by jamie on 2025/5/15.
//

#ifndef ABOUTPOPUP_H
#define ABOUTPOPUP_H

#include "Popup.h"
#include "common/config.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Popup
{
class AboutPopup final : public ModalPopup
{
public:
    explicit AboutPopup(const std::string_view &nameKey) : ModalPopup(nameKey) {}

protected:
    void DoDraw(SosUiData &uiData, bool &confirmed) override;
};
}
}

#endif // ABOUTPOPUP_H
