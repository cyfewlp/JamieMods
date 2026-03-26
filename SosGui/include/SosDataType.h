//
// Created by jamie on 2025/4/3.
//

#ifndef SOSDATATYPE_H
#define SOSDATATYPE_H

#pragma once

#include "Translation.h"
#include "imgui.h"

#include <cstdint>

namespace LIBC_NAMESPACE_DECL
{
struct SosFunction
{
    static constexpr auto GetOutfitNameMaxLength                    = "GetOutfitNameMaxLength";
    static constexpr auto GetCarriedArmor                           = "GetCarriedArmor";
    static constexpr auto GetWornItems                              = "GetWornItems";
    static constexpr auto RefreshArmorFor                           = "RefreshArmorFor";
    static constexpr auto RefreshArmorForAllConfiguredActors        = "RefreshArmorForAllConfiguredActors";
    static constexpr auto ActorNearPC                               = "ActorNearPC";
    static constexpr auto PrepArmorSearch                           = "PrepArmorSearch";
    static constexpr auto GetArmorSearchResultForms                 = "GetArmorSearchResultForms";
    static constexpr auto GetArmorSearchResultNames                 = "GetArmorSearchResultNames";
    static constexpr auto ClearArmorSearch                          = "ClearArmorSearch";
    static constexpr auto PrepOutfitBodySlotListing                 = "PrepOutfitBodySlotListing";
    static constexpr auto GetOutfitBodySlotListingArmorForms        = "GetOutfitBodySlotListingArmorForms";
    static constexpr auto GetOutfitBodySlotListingArmorNames        = "GetOutfitBodySlotListingArmorNames";
    static constexpr auto GetOutfitBodySlotListingSlotIndices       = "GetOutfitBodySlotListingSlotIndices";
    static constexpr auto ClearOutfitBodySlotListing                = "ClearOutfitBodySlotListing";
    static constexpr auto NaturalSort_ASCII                         = "NaturalSort_ASCII";
    static constexpr auto NaturalSortPairArmor_ASCII                = "NaturalSortPairArmor_ASCII";
    static constexpr auto HexToInt32                                = "HexToInt32";
    static constexpr auto ToHex                                     = "ToHex";
    static constexpr auto AddArmorToOutfit                          = "AddArmorToOutfit";
    static constexpr auto ArmorConflictsWithOutfit                  = "ArmorConflictsWithOutfit";
    static constexpr auto CreateOutfit                              = "CreateOutfit";
    static constexpr auto DeleteOutfit                              = "DeleteOutfit";
    static constexpr auto GetOutfitContents                         = "GetOutfitContents";
    static constexpr auto GetOutfitFavoriteStatus                   = "GetOutfitFavoriteStatus";
    static constexpr auto SetOutfitFavoriteStatus                   = "SetOutfitFavoriteStatus";
    static constexpr auto BodySlotPolicyNamesForOutfit              = "BodySlotPolicyNamesForOutfit";
    static constexpr auto SetBodySlotPoliciesForOutfit              = "SetBodySlotPoliciesForOutfit";
    static constexpr auto SetAllBodySlotPoliciesForOutfit           = "SetAllBodySlotPoliciesForOutfit";
    static constexpr auto SetBodySlotPolicyToDefaultForOutfit       = "SetBodySlotPolicyToDefaultForOutfit";
    static constexpr auto GetAvailablePolicyNames                   = "GetAvailablePolicyNames";
    static constexpr auto GetAvailablePolicyCodes                   = "GetAvailablePolicyCodes";
    static constexpr auto GetSelectedOutfit                         = "GetSelectedOutfit";
    static constexpr auto IsEnabled                                 = "IsEnabled";
    static constexpr auto ListOutfits                               = "ListOutfits";
    static constexpr auto RemoveArmorFromOutfit                     = "RemoveArmorFromOutfit";
    static constexpr auto RemoveConflictingArmorsFrom               = "RemoveConflictingArmorsFrom";
    static constexpr auto RenameOutfit                              = "RenameOutfit";
    static constexpr auto OutfitExists                              = "OutfitExists";
    static constexpr auto OverwriteOutfit                           = "OverwriteOutfit";
    static constexpr auto SetEnabled                                = "SetEnabled";
    static constexpr auto SetSelectedOutfit                         = "SetSelectedOutfit";
    static constexpr auto AddActor                                  = "AddActor";
    static constexpr auto RemoveActor                               = "RemoveActor";
    static constexpr auto ListActors                                = "ListActors";
    static constexpr auto SetLocationBasedAutoSwitchEnabled         = "SetLocationBasedAutoSwitchEnabled";
    static constexpr auto GetLocationBasedAutoSwitchEnabled         = "GetLocationBasedAutoSwitchEnabled";
    static constexpr auto GetAutoSwitchStateArray                   = "GetAutoSwitchStateArray";
    static constexpr auto IdentifyStateType                         = "IdentifyStateType";
    static constexpr auto SetOutfitUsingState                       = "SetOutfitUsingState";
    static constexpr auto SetOutfitUsingStateForAllConfiguredActors = "SetOutfitUsingStateForAllConfiguredActors";
    static constexpr auto SetStateOutfit                            = "SetStateOutfit";
    static constexpr auto UnsetStateOutfit                          = "UnsetStateOutfit";
    static constexpr auto GetStateOutfit                            = "GetStateOutfit";
    static constexpr auto ExportSettings                            = "ExportSettings";
    static constexpr auto ImportSettings                            = "ImportSettings";
    static constexpr auto NotifyCombatStateChanged                  = "NotifyCombatStateChanged";
};

static constexpr std::array ARMOR_SLOT_NAMES = {
    "None",
    "Head",
    "Hair",
    "Body",
    "Hands",
    "Forearms",
    "Amulet",
    "Ring",
    "Feet",
    "Calves",
    "Shield",
    "Tail",
    "LongHair",
    "Circlet",
    "Ears",
    "ModMouth",
    "ModNeck",
    "ModChestPrimary",
    "ModBack",
    "ModMisc1",
    "ModPelvisPrimary",
    "DecapitateHead",
    "Decapitate",
    "ModPelvisSecondary",
    "ModLegRight",
    "ModLegLeft",
    "ModFaceJewelry",
    "ModChestSecondary",
    "ModShoulder",
    "ModArmLeft",
    "ModArmRight",
    "ModMisc2",
    "FX01",
};

static constexpr std::array SLOT_POLICY_NAMES = {
    "XXXX", // not available advanced: true
    "XXXE", // not available advanced: true
    "XXXO", //     available advanced: false  // name: Require equipped
    "XXOX", // not available advanced: true
    "XXOE", // not available advanced: true
    "XXOO", //     available advanced: false  // name: Always use outfit
    "XEXX", // not available advanced: true
    "XEXE", // not available advanced: true
    "XEXO", //     available advanced: false  // name: Passthrough
    "XEOX", // not available advanced: true
    "XEOE", // not available advanced: true
    "XEOO", // not available advanced: true
};

static constexpr std::array AVAILABLE_SLOT_POLICY = {
    "XXXO", //     available advanced: false  // name: Require equipped
    "XXOO", //     available advanced: false  // name: Always use outfit
    "XEXO", //     available advanced: false  // name: Passthrough
};

enum class SlotPolicy : uint8_t
{
    Inherit,
    RequireEquipped,
    AlwaysUseOutfit,
    Passthrough
};

inline auto SlotPolicyToUiKey(SlotPolicy policy) -> const char *
{
    switch (policy)
    {
        case SlotPolicy::Inherit:
            return "$SkyOutSys_Desc_PolicyName_INHERIT";
        case SlotPolicy::RequireEquipped:
            return "$SkyOutSys_Desc_EasyPolicyName_XXXO";
        case SlotPolicy::AlwaysUseOutfit:
            return "$SkyOutSys_Desc_EasyPolicyName_XXOO";
        case SlotPolicy::Passthrough:
            return "$SkyOutSys_Desc_EasyPolicyName_XEXO";
        default:
            return "$SkyOutSys_Desc_EasyPolicyName_UNKNOWN";
    }
}

inline auto SlotPolicyToUiString(SlotPolicy policy) -> std::string
{
    auto key = SlotPolicyToUiKey(policy);
    return Translation::Translate(key);
}

inline auto SlotPolicyToTooltipKey(SlotPolicy policy) -> const char *
{
    switch (policy)
    {
        case SlotPolicy::Inherit:
            return "";
        case SlotPolicy::RequireEquipped:
            return "$SkyOutSys_Desc_PolicyName_XXXO";
        case SlotPolicy::AlwaysUseOutfit:
            return "$SkyOutSys_Desc_PolicyName_XXOO";
        case SlotPolicy::Passthrough:
            return "$SkyOutSys_Desc_PolicyName_XEXO";
        default:
            return "$SkyOutSys_Desc_PolicyName_UNKNOWN";
    }
}

inline auto SlotPolicyToTooltipString(SlotPolicy policy) -> std::string
{
    auto key = SlotPolicyToTooltipKey(policy);
    return Translation::Translate(key);
}

// used for SkyrimOutfitSystem
inline auto SlotPolicyToCode(const SlotPolicy policy) -> std::string
{
    switch (policy)
    {
        case SlotPolicy::Inherit:
            return "";
        case SlotPolicy::RequireEquipped:
            return "XXXO";
        case SlotPolicy::AlwaysUseOutfit:
            return "XXOO";
        case SlotPolicy::Passthrough:
            return "XEXO";
        default:
            return "UNKNOWN";
    }
}

constexpr std::string_view SOS_SPELL_EDITOR_ID   = "SkyrimOutfitSystemQuickslotSpell";
constexpr std::string_view SOS_NATIVE_CLASS_NAME = "SkyrimOutfitSystemNativeFuncs";
constexpr uint8_t          MAX_ERROR_COUNT       = 64;
constexpr uint8_t          MAX_ERROR_SHOW_COUNT  = 5;
} // namespace LIBC_NAMESPACE_DECL

#endif // SOSDATATYPE_H
