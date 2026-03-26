//
// Created by jamie on 2025/5/15.
//

#include "Popup.h"

namespace SosGui::Popup
{
class SettingsPopup : public ModalPopup
{
public:
    explicit SettingsPopup(const std::string_view nameKey) : ModalPopup(nameKey) {}

protected:
    void DoDraw(SosUiData &uiData, bool &confirmed) override;

    static void ThemeCombo();
};
} // namespace SosGui::Popup
