#pragma once

namespace LIBC_NAMESPACE_DECL
{
    struct SosFunctionNames
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

    class PapyrusEvent
    {
        SKSE::RegistrationSet<const char *>      requireUiDataEvent{"OnRequireUiData"};
        SKSE::RegistrationSet<const RE::Actor *> requireAddActor{"OnRequireAddActor"};
        SKSE::RegistrationSet<bool>              requireSetEnabled{"OnRequireSetEnabled"};
        SKSE::RegistrationSet<const RE::Actor *> requireRemoveActor{"OnRequireRemoveActor"};

    public:
        static auto Bind(RE::BSScript::IVirtualMachine *vm) -> bool;

        static auto GetInstance() -> PapyrusEvent &
        {
            static PapyrusEvent g_instance;
            return g_instance;
        }

        constexpr auto CallNoArgs(const char *methodName)
        {
            requireUiDataEvent.QueueEvent(methodName);
        }

        constexpr auto CallAddActor(const RE::Actor *actor)
        {
            requireAddActor.QueueEvent(actor);
        }

        constexpr auto CallSetEnabled(bool enabled)
        {
            requireSetEnabled.QueueEvent(enabled);
        }

        constexpr auto CallRemoveActor(const RE::Actor *actor)
        {
            requireRemoveActor.QueueEvent(actor);
        }

    private:
        static void RegisterForRequireUiData(RE::StaticFunctionTag *, RE::TESForm *thisForm)
        {
            GetInstance().requireUiDataEvent.Register(thisForm);
        }

        static void RegisterForRequireAddActor(RE::StaticFunctionTag *, RE::TESForm *thisForm)
        {
            GetInstance().requireAddActor.Register(thisForm);
        }

        static void RegisterForRequireSetEnabled(RE::StaticFunctionTag *, RE::TESForm *thisForm)
        {
            GetInstance().requireSetEnabled.Register(thisForm);
        }

        static void RegisterForRequireRemoveActor(RE::StaticFunctionTag *, RE::TESForm *thisForm)
        {
            GetInstance().requireRemoveActor.Register(thisForm);
        }
    };
}