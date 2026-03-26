//
// Created by jamie on 2025/5/15.
//

#include "Popup.h"

namespace SosGui::Popup
{
class AboutPopup final : public ModalPopup
{
public:
    explicit AboutPopup(const std::string_view &nameKey) : ModalPopup(nameKey) {}

protected:
    void DoDraw(SosUiData &uiData, bool &confirmed) override;
};
} // namespace SosGui::Popup
