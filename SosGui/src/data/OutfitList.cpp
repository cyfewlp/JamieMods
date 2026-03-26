#include "data/OutfitList.h"

#include "common/config.h"
#include "data/id.h"

#include <string>

namespace LIBC_NAMESPACE_DECL
{
[[nodiscard]]
auto OutfitList::SetFavoriteOutfit(const OutfitId id, bool favorite) -> std::expected<void, unassociated_outfit_error>
{
    const auto outfitIt = m_outfitById.find(id);
    if (outfitIt == m_outfitById.end())
    {
        return std::unexpected(unassociated_outfit_error{});
    }
    m_outfitById.modify(outfitIt, [&](auto &outfit) {
        outfit.SetFavorite(favorite);
    });
    return {};
}

[[nodiscard]]
auto OutfitList::SetFavoriteOutfit(const std::string &outfitName, bool favorite) -> std::expected<void, unassociated_outfit_error>
{
    if (auto outfitIt = m_outfitByName.find(outfitName); outfitIt != m_outfitByName.end())
    {
        m_outfitByName.modify(outfitIt, [&](auto &outfit) {
            outfit.SetFavorite(favorite);
        });
        return {};
    }
    return std::unexpected(unassociated_outfit_error{});
}

auto OutfitList::GetOutfitByNameRank(const size_t rank) const -> boost::optional<const SosUiOutfit &>
{
    boost::optional<const SosUiOutfit &> opt;
    if (const auto foundId = m_outfitByName.nth(rank); foundId != m_outfitByName.end())
    {
        opt = *foundId;
    }
    return opt;
}

auto OutfitList::GetOutfitById(const OutfitId id) const -> boost::optional<const SosUiOutfit &>
{
    boost::optional<const SosUiOutfit &> opt;

    if (const auto foundId = m_outfitById.find(id); foundId != m_outfitById.end())
    {
        opt = *foundId;
    }
    return opt;
}

void OutfitList::RenameOutfit(const OutfitId id, const std::string &&newName)
{
    if (auto where = m_outfitById.find(id); where != m_outfitById.end())
    {
        m_outfitById.modify(where, [&](auto &outfit) {
            outfit.SetName(newName);
        });
    }
}

void OutfitList::AddArmor(const OutfitId id, const Armor *armor)
{
    if (auto where = m_outfitById.find(id); where != m_outfitById.end())
    {
        m_outfitById.modify(where, [&](auto &outfit) {
            outfit.AddArmor(armor);
        });
    }
}

auto OutfitList::GetNameRank(const OutfitId &id) const -> size_t
{
    if (!HasOutfit(id))
    {
        throw unassociated_outfit_error();
    }
    auto itById   = m_outfitById.find(id);
    auto itByName = project<by_name>(m_container, itById);
    return m_outfitByName.rank(itByName);
}

auto OutfitList::TryFindIdByName(const std::string &outfitName) const -> boost::optional<OutfitId>
{
    const auto itByName = m_outfitByName.find(outfitName);
    if (itByName != m_outfitByName.end())
    {
        return itByName->GetId();
    }
    return boost::none;
}

auto OutfitList::findIdByName(const std::string &outfitName) const -> OutfitId
{
    auto itByName = m_outfitByName.find(outfitName);
    if (itByName == m_outfitByName.end())
    {
        return INVALID_OUTFIT_ID;
    }
    return itByName->GetId();
}

} // namespace LIBC_NAMESPACE_DECL
