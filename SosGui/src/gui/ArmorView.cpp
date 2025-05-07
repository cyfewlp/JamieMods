//
// Created by jamie on 2025/5/7.
//

#include "gui/ArmorView.h"
#include "imgui.h"

#include <ranges>

namespace LIBC_NAMESPACE_DECL
{
bool ArmorView::ArmorFilter::PassFilter(const Armor *armor) const
{
    if (mustPlayable && IsArmorNonPlayable(armor))
    {
        return false;
    }
    if (!filter.IsActive())
    {
        return true;
    }
    if (DebounceInput::PassFilter(armor->GetName()))
    {
        return true;
    }
    return false;
}

bool ArmorView::ArmorFilter::Draw()
{
    bool needUpdate = ImGuiUtil::CheckBox("$SkyOutSys_OEdit_AddFromList_Filter_Playable", &mustPlayable);
    ImGui::SameLine();
    needUpdate |= DebounceInput::Draw("##ArmorFilter",
                                      Translation::Translate("$SkyOutSys_OEdit_AddFromList_Filter_Name").c_str());
    return needUpdate;
}

bool ArmorView::ModFilterer::PassFilter(const Armor *armor) const
{
    bool result = true;
    if (passModList.empty()) return result;
    for (const auto &[modName, checked] : passModList)
    {
        if (checked)
        {
            result = false;
            if (util::GetFormModFileName(armor) == modName)
            {
                return true;
            }
        }
    }
    return result;
}

void ArmorView::init()
{
    auto       *dataHandler = RE::TESDataHandler::GetSingleton();
    const auto &armorArray  = dataHandler->GetFormArray<RE::TESObjectARMO>();

    availableArmorCount = 0;
    for (const auto &armor : armorArray)
    {
        if (IsArmorCanDisplay(armor))
        {
            armorContainer.Insert(armor);
            availableArmorCount++;
        }
    }
}

void ArmorView::clear()
{
    armorContainer.Clear();
    viewData.clear();
    modRefCounter.clear();
    slotCounter.fill(0);
    slotFiltererSelected = 0;
    checkAllSlot         = true;
    multiSelection.Clear();
    armorFilter.clear();
    modFilterer.Clear();
    modRefCounter.clear();
}

void ArmorView::clearViewData()
{
    viewData.clear();
    multiSelection.Clear();
}

void ArmorView::remove_armors_has_slot(Slot selectedSlots, Slot toRemoveSlot)
{
    multiSelection.Clear();
    for (auto itBegin = viewData.begin(); itBegin != viewData.end();)
    {
        if (const auto *armor = *itBegin;
            armor->HasPartOf(toRemoveSlot) && util::IsArmorHasNoneSlotOf(armor, selectedSlots))
        {
            itBegin = viewData.erase(itBegin);
        }
        else
        {
            ++itBegin;
        }
    }
}

void ArmorView::add_armors_in_outfit(SosUiData &uiData, const SosUiOutfit *editingOutfit)
{
    for (const auto &armor : editingOutfit->GetUniqueArmors())
    {
        if (auto result = add_armor(armor); !result.has_value())
        {
            uiData.PushErrorMessage(
                std::format("Can't restore armor {} from outfit {}", armor->GetName(), editingOutfit->GetName()));
        }
    }
}

void ArmorView::remove_armors_in_outfit(const SosUiOutfit *editingOutfit)
{
    // may multi armor use the same slot
    for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
    {
        if (const auto *armor = editingOutfit->GetArmorAt(slotPos); armor != nullptr)
        {
            if (auto result = find_armor(armor); result.has_value())
            {
                viewData.erase(viewData.begin() + result.value());
                slotCounter[slotPos] -= 1;
            }
        }
    }
}

bool ArmorView::filter(const Armor *armor) const
{
    if (!modFilterer.PassFilter(armor))
    {
        return false;
    }
    // filter by slot: has any selected slot
    auto slot = static_cast<Slot>(slotFiltererSelected.to_ulong());
    if (!checkAllSlot && slot != Slot::kNone && util::IsArmorHasNoneSlotOf(armor, slot))
    {
        return false;
    }
    // filter by armor name
    if (!armorFilter.PassFilter(armor))
    {
        return false;
    }
    return true;
}

auto ArmorView::add_armor(Armor *armor) -> std::expected<void, error>
{
    if (filter(armor))
    {
        const auto armorRank = armorContainer.GetRank(armor->formID);
        size_t     startPos  = 0;
        size_t     endPos    = viewData.size();
        while (endPos - startPos > 0)
        {
            const size_t middle = startPos + (endPos - startPos) / 2;
            const auto   rank   = armorContainer.GetRank(viewData.at(middle)->formID);
            if (rank >= armorContainer.Size())
            {
                return std::unexpected{error::unassociated_armor};
            }
            if (armorRank < rank)
            {
                endPos = middle;
            }
            else if (armorRank > rank)
            {
                startPos = middle + 1;
            }
            else
            {
                return std::unexpected{error::armor_already_exists};
            }
        }
        viewData.insert(viewData.begin() + startPos, armor);
    }
    return {};
}

bool ArmorView::remove_armor(const Armor *armor)
{
    if (const auto result = find_armor(armor); result.has_value())
    {
        viewData.erase(viewData.begin() + result.value());
        for (uint8_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
        {
            if (armor->HasPartOf(static_cast<Slot>(1 << slotPos)))
            {
                slotCounter[slotPos] -= 1;
            }
        }
        return true;
    }
    return false;
}

void ArmorView::reset_counter()
{
    modRefCounter.clear();
    for (const auto &armor : viewData)
    {
        std::string_view modName = util::GetFormModFileName(armor);
        modRefCounter.emplace(modName, 0);
        modRefCounter[modName] += 1;

        const auto slotMaskValue = static_cast<uint32_t>(armor->GetSlotMask());
        for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
        {
            uint32_t slotValue = 1 << slotPos;
            if (slotValue > slotMaskValue) break;
            if ((slotMaskValue & slotValue) != 0)
            {
                slotCounter[slotPos]++;
            }
        }
    }
    for (const auto &modName : modRefCounter | std::views::keys)
    {
        modFilterer.passModList.emplace_back(modName, false);
    }
}

void ArmorView::update_view_data(ArmorGenerator *generator, const SosUiOutfit *editingOutfit)
{
    clearViewData();
    if (generator != nullptr)
    {
        try
        {
            generator->for_each([&](Armor *armor) {
                if (const auto result = add_armor(armor); !result.has_value())
                {
                    throw std::runtime_error(ToErrorMessage(result.error()));
                }
            });
        }
        catch (const std::runtime_error &e)
        {
            // TODO: add error message to notifier?
            log_error("WARNING: Invalid armor generator: {}", e.what());
        }
        remove_armors_in_outfit(editingOutfit);
    }
}

void ArmorView::reset_view(ArmorGenerator *generator, const SosUiOutfit *editingOutfit)
{
    modFilterer.Clear();
    slotFiltererSelected     = 0;
    armorFilter.mustPlayable = false;
    armorFilter.filter.Clear();

    update_view_data(generator, editingOutfit);
    reset_counter();
}

auto ArmorView::find_armor(const Armor *armor) const -> std::expected<size_t, error>
{
    const auto armorRank = armorContainer.GetRank(armor->formID);
    size_t     startPos  = 0;
    size_t     endPos    = viewData.size();
    while (endPos - startPos > 0)
    {
        const size_t middle = (endPos + startPos) / 2;
        const auto   rank   = armorContainer.GetRank(viewData.at(middle)->formID);
        if (rank >= armorContainer.Size())
        {
            return std::unexpected{error::unassociated_armor};
        }
        if (armorRank < rank)
        {
            endPos = middle;
        }
        else if (armorRank > rank)
        {
            startPos = middle + 1;
        }
        else
        {
            return middle;
        }
    }
    return std::unexpected{error::armor_not_exist};
}

bool ArmorView::no_select_any_slot() const
{
    return !checkAllSlot && slotFiltererSelected == 0;
}
}
