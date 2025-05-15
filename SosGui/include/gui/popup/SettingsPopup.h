//
// Created by jamie on 2025/5/15.
//

#ifndef SETTINGSPOPUP_H
#define SETTINGSPOPUP_H

#include "Popup.h"
#include "common/config.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Popup
{
class SettingsPopup : public ModalPopup
{
public:
    explicit SettingsPopup(const std::string_view nameKey) : ModalPopup(nameKey) {}

protected:
    void DoDraw(SosUiData &uiData, bool &confirmed) override;

    static void ThemeCombo(SosUiData &uiData);
};
}
}

#endif // SETTINGSPOPUP_H
