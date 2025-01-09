#include "Transmogrify.h"
#include "Ext.h"

RE::InventoryChanges::IItemChangeVisitor::~IItemChangeVisitor() {};

class EquippedArmorVisitor : public RE::InventoryChanges::IItemChangeVisitor
{
public:
    virtual RE::BSContainer::ForEachResult Visit(RE::InventoryEntryData* data) override
    {
        LOG(debug, "item name: [{}]", data->GetDisplayName());
        auto form = data->object;
        if (form && form->formType == RE::FormType::Armor) {
            equipped.emplace(skyrim_cast<RE::TESObjectARMO*>(form));
        }
        return RE::BSContainer::ForEachResult::kContinue; // Return true to "continue visiting".
    }

    std::unordered_set<const RE::TESObjectARMO*> equipped;
};

static void
printPlayerInventory(RE::Actor* player)
{
    auto* race = player->GetRace();
    if (!player || !race) {
        LOG(debug, "can't acquire player.");
        return;
    }

    RE::InventoryChanges* inventoryChanges = player->GetInventoryChanges();

    if (!inventoryChanges || !inventoryChanges->entryList) {
        LOG(debug, "The inventoryChanges is nullity");
        return;
    }
    EquippedArmorVisitor visitor;
    inventoryChanges->VisitWornItems(visitor);
    for (const auto* armor : visitor.equipped) {
        LOG(debug, "armor form id: {}, editor-id: {}, print add-ons:", armor->GetFormID(), armor->GetFormEditorID());
        for (auto& currentAddon : armor->armorAddons) {
            if (currentAddon) {
                LOG(debug,
                    "  add-on: form id [{}], editor-id: {}, local form id: {}, raw form id: {}",
                    currentAddon->GetFormID(),
                    currentAddon->GetFormEditorID(),
                    currentAddon->GetLocalFormID(),
                    currentAddon->GetRawFormID());
            }
        }
    }

    for (auto armor : race->decapitateArmors) {
        if (armor) {
            LOG(debug, "  slot: form id [{}], form type: {}", armor->GetFormID(), (int)armor->GetFormType());
        }
    }
}

void
Transmogrify::AccessPlayerInventory()
{
    RE::Actor* player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        LOG(debug, "can't acquire player.");
        return;
    }
    printPlayerInventory(player);
}

void
Transmogrify::GetAllArmors(MapToSet* modToArmors)
{
    auto* handler = RE::TESDataHandler::GetSingleton();
    auto& armorList = handler->GetFormArray(RE::FormType::Armor);

    for (auto& form : armorList) {
        if (!form || form->GetFormType() != RE::FormType::Armor) {
            continue;
        }
        auto armor = skyrim_cast<RE::TESObjectARMO*>(form);
        if (!armor || armor->templateArmor) {
            continue;
        }
        std::string armorName, modName;
        auto file = armor->GetFile();
        modName = file ? std::string(file->GetFilename()) : std::string("Unknown");
        armorName = armor->GetFullName();

        if (!armorName.empty()) {
            LOG(debug, "mod: {}, armor: {}", modName, armorName);
            auto [element, inserted] = modToArmors->try_emplace(modName, std::unordered_set<std::string>());
            element->second.insert(armorName);
        }
    }
}

void
Transmogrify::CombineEventSinks::Install()
{
    // auto eventSource = RE::ScriptEventSourceHolder::GetSingleton();
    // eventSource->AddEventSink(CombineEventSinks::GetSingleton());
}

RE::BSEventNotifyControl
Transmogrify::CombineEventSinks::ProcessEvent(
  const RE::TESActorLocationChangeEvent* a_event,
  [[maybe_unused]] RE::BSTEventSource<RE::TESActorLocationChangeEvent>* a_eventSource)
{
    /*auto actor = a_event->actor.get()->As<RE::Actor>();
    if (actor && actor->Is3DLoaded()) {
        actor->Update3DModel();
    }*/

    return RE::BSEventNotifyControl::kContinue;
}