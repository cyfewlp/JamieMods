//
// Created by jamie on 2025/4/3.
//

#ifndef SOSDATATYPE_H
#define SOSDATATYPE_H

#pragma once

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

    enum StateType : int8_t
    {
        Combat  = 12,
        World   = 0,
        Town    = 1,
        Dungeon = 2,
        City    = 9,

        WorldSnowy   = 3,
        TownSnowy    = 4,
        DungeonSnowy = 5,
        CitySnowy    = 10,

        WorldRainy      = 6,
        TownRainy       = 7,
        DungeonRainy    = 8,
        CityRainy       = 11,
        StateType_Count = 13,
        StateType_All   = -1
    };

    enum OutfitAddPolicy : int8_t
    {
        OutfitAddPolicy_AddFromCarried = 0,
        OutfitAddPolicy_AddFromWorn,
        OutfitAddPolicy_AddByID,
        OutfitAddPolicy_AddAny,
        OutfitAddPolicy_Count,
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

    constexpr std::string_view SOS_SPELL_EDITOR_ID = "SkyrimOutfitSystemQuickslotSpell";
    constexpr std::string_view SOS_NATIVE_CLASS_NAME = "SkyrimOutfitSystemNativeFuncs";

    constexpr auto HintFontSize = [] {
        return ImGui::GetFontSize() * 1.2F;
    };
    constexpr auto HeaderFontSize = [] {
        return ImGui::GetFontSize() * 1.3F;
    };
}

#endif // SOSDATATYPE_H
