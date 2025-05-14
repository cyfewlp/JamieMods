//
// Created by jamie on 2025/5/14.
//

#ifndef OUTFITSELECTPOPUP_H
#define OUTFITSELECTPOPUP_H

#include "common/config.h"
#include "gui/Popup.h"
#include "util/ImGuiUtil.h"

#include <vector>

namespace LIBC_NAMESPACE_DECL
{
class OutfitList;

class OutfitSelectPopup : Popup::BasicPopup
{
    struct OutfitDebounceInput final : ImGuiUtil::DebounceInput
    {
        std::vector<const SosUiOutfit *> viewData{};

        void Clear() override;
        void OnInput() override;
        void UpdateView(const OutfitList &outfitList);
    } debounceInput{};

public:
    void UpdateView(const OutfitList &outfitList)
    {
        debounceInput.UpdateView(outfitList);
    }

    bool Draw(const char *nameKey, const OutfitList &outfitList, __out OutfitId &selectId);
};
}

#endif // OUTFITSELECTPOPUP_H
