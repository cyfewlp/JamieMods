// Internal helper script - Personal Use Only.
// -----------------------------------------------------------------------------
// This is a quick-and-dirty utility maintained for specific local tasks.
// It is NOT a production-grade tool and will not receive regular updates.
// If it breaks, you get to keep both pieces.
// -----------------------------------------------------------------------------

#include "../common/json/simdjson.h"

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <print>
#include <ranges>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

using namespace simdjson;

namespace
{
constexpr auto id_length                   = 16U + 1U;
constexpr auto designSystems_length        = std::string_view{"designSystems"}.length() + 1;
constexpr auto token_set_length            = std::string_view{"tokenSets"}.length() + 1;
constexpr auto displayGroups_length        = std::string_view{"displayGroups"}.length() + 1;
constexpr auto tokens_length               = std::string_view{"tokens"}.length() + 1;
constexpr auto values_length               = std::string_view{"values"}.length() + 1;
constexpr auto designSystems_prefix_length = designSystems_length + id_length;
constexpr auto token_set_prefix_length     = designSystems_prefix_length + token_set_length;
constexpr auto displayGroups_prefix_length = token_set_prefix_length + id_length + displayGroups_length;
constexpr auto tokens_prefix_length        = token_set_prefix_length + id_length + tokens_length;
constexpr auto values_prefix_length        = tokens_prefix_length + id_length + values_length;

constexpr auto DisplayGroup_Deprecated_Prefix = std::string_view{"[Deprecated]"};
constexpr auto Motion_Token_Prefix            = std::string_view{"md.sys.motion"};

auto token_name_to_camelCase(std::string_view tokenName, std::string &out)
{
    bool toUpper = true;
    for (size_t index = 0; index < tokenName.length(); index++)
    {
        if (tokenName[index] == '-' || tokenName[index] == '.')
        {
            toUpper = true;
        }
        else if (toUpper)
        {
            out.push_back(static_cast<char>(std::toupper(tokenName[index])));
            toUpper = false;
        }
        else
        {
            out.push_back(tokenName[index]);
        }
    }
}

auto normalize_token_value(std::string_view tokenName) -> std::string
{
    std::string memberDef = "static constexpr auto ";
    token_name_to_camelCase(tokenName, memberDef);
    return memberDef;
}

auto normalize_color_value(std::string_view colorValue) -> std::string
{
    auto lastDotPos = colorValue.rfind('.');

    if (lastDotPos != std::string_view::npos)
    {
        auto        colorName = colorValue.substr(lastDotPos + 1);
        std::string colorRole = "ColorRole::";
        bool        toUpper   = false;
        for (size_t index = 0; index < colorName.length(); index++)
        {
            if (colorName[index] == '-')
            {
                toUpper = true;
            }
            else if (toUpper)
            {
                colorRole.push_back(static_cast<char>(std::toupper(colorName[index])));
                toUpper = false;
            }
            else
            {
                colorRole.push_back(colorName[index]);
            }
        }
        return colorRole;
    }
    return std::string(colorValue);
}

auto normalize_shape_value(std::string_view shapeValue) -> std::string
{
    auto lastDotPos = shapeValue.rfind('.');

    if (lastDotPos != std::string_view::npos)
    {
        auto        shapeName = shapeValue.substr(lastDotPos + 1);
        std::string shape     = "ShapeCorner::";
        bool        toUpper   = true; // first char is upper case.
        for (size_t index = 0; index < shapeName.length(); index++)
        {
            if (shapeName[index] == '-')
            {
                toUpper = true;
            }
            else if (toUpper)
            {
                shape.push_back(static_cast<char>(std::toupper(shapeName[index])));
                toUpper = false;
            }
            else
            {
                shape.push_back(shapeName[index]);
            }
        }
        return shape;
    }
    return std::string(shapeValue);
}

enum class token_type
{
    color,
    length,
    typography,
    unsupported
};

struct TokenValue
{
    std::string name;
    std::string value;
};

using TokenValueCache = std::unordered_map<std::string, TokenValue>;

struct TokenMetadata
{
    size_t     index;
    size_t     displayGroupIndex;
    token_type type;
};

struct Token
{
    std::string name;
    std::string tokenName;
    std::string displayName;
    std::string type;
    TokenValue  value;

    void normalize()
    {
        if (type == "COLOR")
        {
            value.value = normalize_color_value(value.value);
        }
        else if (type == "SHAPE")
        {
            value.value = normalize_shape_value(value.value);
        }
        tokenName = normalize_token_value(tokenName);
    }

    [[nodiscard]] auto toString() const -> std::string
    {
        return std::format(
            R"(    //! {}
   {} = {};
)",
            displayName,
            tokenName,
            value.value
        );
    }
};

struct TokenSet
{
    std::string        name;
    std::string        tokenSetNameSuffix;
    std::string        tokenSetName;
    std::vector<Token> tokens;

    void normalize()
    {
        std::string def = "struct ";
        if (tokenSetName.starts_with("md.comp"))
        {
            tokenSetName.erase(0, std::string_view{"md.comp."}.length());
        }
        tokenSetName[0] = std::toupper(tokenSetName[0]);
        token_name_to_camelCase(tokenSetName, def);
        tokenSetName = def;
    }
};

using TokenId = std::string;

auto parse_length_value(simdjson_result<ondemand::value> jValue, TokenValue &value)
{
    bool parseSuccess = false;

    int64_t          length = 0;
    std::string_view tUnit;
    if (auto errorCode = jValue["length"]["value"].get(length); SUCCESS == jValue["length"]["unit"].get(tUnit))
    {
        if (errorCode != SUCCESS && errorCode != EMPTY && tUnit != "DIPS")
        {
            std::println("[Warning] Unsupported length unit: {} ", tUnit.data());
        }
        else
        {
            value.value  = std::format("dp<{}>()", length);
            parseSuccess = true;
        }
    }
    return parseSuccess;
}

auto parse_values(simdjson_result<ondemand::value> jValues) -> TokenValueCache
{
    auto jValueArray = jValues.get_array();
    if (jValueArray.error() != SUCCESS)
    {
        std::cout << "[Error] Failed to parse values.  " << jValues.error() << '\n';
    }
    // std::vector<TokenValue> values;
    TokenValueCache valuesCache;
    size_t          size = 0;
    if (SUCCESS == jValueArray.count_elements().get(size))
    {
        valuesCache.reserve(size);
    }
    // may contains deprecated tokens value
    for (auto value : jValueArray)
    {
        TokenValue tokenValue;
        if (SUCCESS == value["name"].get(tokenValue.name) && SUCCESS == value["tokenName"].get(tokenValue.value))
        {
            valuesCache.emplace(tokenValue.name, std::move(tokenValue));
        }
    }
    return valuesCache;
}

auto operator<<(std::ostream &os, const Token &token) -> std::ostream &
{
    if (token.value.value.starts_with(Motion_Token_Prefix))
    {
        std::println("[Warning] Motion token is not supported yet: {} ", token.tokenName);
        return os;
    }
    return os << token.toString();
}

auto operator<<(std::ostream &os, const TokenSet &tokenSet) -> std::ostream &
{
    auto &os1 = os << tokenSet.tokenSetName << "\n{\n";
    std::ranges::copy(tokenSet.tokens, std::ostream_iterator<Token>(os1, "\n"));
    os1 << "};\n";
    return os1;
}

auto parse_tokenSets(simdjson_result<ondemand::value> jTokenSets) -> std::vector<TokenSet>
{
    if (jTokenSets.error() != SUCCESS)
    {
        std::cout << "[Error] Failed to parse tokenSets!" << '\n';
    }
    std::vector<TokenSet> tokenSets;
    for (auto jTokenSet : jTokenSets.get_array())
    {
        TokenSet tokenSet;
        if (SUCCESS == jTokenSet["name"].get(tokenSet.name) && SUCCESS == jTokenSet["tokenSetName"].get(tokenSet.tokenSetName) &&
            SUCCESS == jTokenSet["tokenSetNameSuffix"].get(tokenSet.tokenSetNameSuffix))
        {
            tokenSet.name.erase(0, token_set_prefix_length);
            tokenSets.push_back(std::move(tokenSet));
        }
    }
    return tokenSets;
}

auto parse_tokens(simdjson_result<ondemand::value> jTokens, std::vector<TokenSet> &tokenSets, bool containDeprecatedToken)
{
    if (jTokens.error() != SUCCESS)
    {
        std::cout << "[Error] Failed to parse tokens!" << '\n';
        return;
    }
    if (tokenSets.empty())
    {
        return;
    }
    using TokenIndex = size_t;
    std::unordered_map<std::string_view, TokenIndex> tokenSetCache;
    for (int index = 0; const auto &tokenSet : tokenSets)
    {
        tokenSetCache[tokenSet.name] = index;
        index++;
    }

    for (auto jToken : jTokens.get_array())
    {
        Token token;
        if (jToken["name"].get(token.name) == SUCCESS)
        {
            if (jToken["deprecationMessage"].has_value())
            {
                std::println("[Warning] Token deprecated: {} ", token.name);
                if (!containDeprecatedToken)
                {
                    continue;
                }
            }
            auto tokenSetId = token.name.substr(token_set_prefix_length, 16U);
            auto tokenId    = token.name.substr(tokens_prefix_length, 16U);
            if (tokenSetCache.contains(tokenSetId))
            {
                auto tokenSetIndex = tokenSetCache[tokenSetId];
                if (SUCCESS == jToken["tokenNameSuffix"].get(token.tokenName) && SUCCESS == jToken["displayName"].get(token.displayName) &&
                    SUCCESS == jToken["tokenValueType"].get(token.type))
                {
                    tokenSets[tokenSetIndex].tokens.push_back(std::move(token));
                }
                else
                {
                    std::cout << "[Error] Failed to parse token: " << token.name.c_str() << '\n';
                }
            }
        }
        else
        {
            std::cout << "[Error] Failed to parse token: Not have a name key!" << '\n';
        }
    }
}

auto parse_contextualReferenceTrees(simdjson_result<ondemand::value> jRefTrees, std::vector<TokenSet> &tokenSets, TokenValueCache &valueCache)
{
    for (auto &tokenSet : tokenSets)
    {
        for (auto &token : tokenSet.tokens)
        {
            if (auto jObj = jRefTrees[token.name].get_object(); jObj.error() == SUCCESS)
            {
                for (auto ref : jObj["contextualReferenceTree"].get_array())
                {
                    const std::string refTokenName;
                    const token_type  tokenType = token_type::unsupported;

                    TokenValue tokenValue;
                    bool       parseSuccess = false;
                    if (token.type == "LENGTH")
                    {
                        parseSuccess = parse_length_value(ref["resolvedValue"], tokenValue);
                    }
                    else
                    {
                        std::string tValueName;
                        if (SUCCESS == ref["referenceTree"]["value"]["name"].get(tValueName))
                        {
                            tokenValue.value = valueCache[tValueName].value;
                            parseSuccess     = true;
                        }
                    }
                    if (parseSuccess)
                    {
                        token.value = std::move(tokenValue);
                    }
                }
            }
        }
    }
}

} // namespace

auto main(int argc, char **argv) -> int
{
    if (argc < 2)
    {
        std::cout << "Usage: m3_design_system_data_parser.exe spec.json" << '\n';
        return -1;
    }

    bool                  containDeprecatedToken = false;
    std::filesystem::path filePath;
    std::filesystem::path outputFilePath;

    for (size_t i = 1; std::cmp_less(i, argc); i++)
    {
        const std::string_view arg = argv[i];
        if (arg == "--help" || arg == "-h")
        {
            std::cout << "Usage: m3_design_system_data_parser.exe spec.json" << '\n';
            return 0;
        }
        if (arg == "--deprecated" || arg == "-d")
        {
            std::cout << "Deprecated tokens will also be parsed, but with a warning." << '\n';
            containDeprecatedToken = true;
        }
        else
        {
            // try to parse file path.
            const std::filesystem::path path(arg);
            if (std::filesystem::exists(path))
            {
                filePath = path;
            }
            else
            {
                std::cout << "File not found: " << arg << '\n';
                return -1;
            }
        }
    }

    if (filePath.has_parent_path())
    {
        outputFilePath = filePath.parent_path();
    }
    outputFilePath.append("m3_normalized_spec_data.txt");

    ondemand::parser parser;
    auto             json_result = padded_string::load(filePath.generic_string());
    if (json_result.error() != SUCCESS)
    {
        std::cerr << "Error loading JSON: " << json_result.error() << '\n';
        return 1;
    }
    auto doc = parser.iterate(json_result);

    auto tokenSets = parse_tokenSets(doc["system"]["tokenSets"]);
    parse_tokens(doc["system"]["tokens"], tokenSets, containDeprecatedToken);
    TokenValueCache valueCache = parse_values(doc["system"]["values"]);
    parse_contextualReferenceTrees(doc["system"]["contextualReferenceTrees"], tokenSets, valueCache);
    valueCache.clear();

    for (auto &tokenSet : tokenSets)
    {
        tokenSet.normalize();

        // Some tokens have state suffixes like "pressed", "hovered", "focused", "disabled", which we want to ignore when sorting tokens, so that
        // tokens with the same prefix will be grouped together.
        // For example, "md.comp.button.filled-container.pressed" and "md.comp.button.filled-container.hovered" will be grouped together as they only
        // differ in the state suffix.
        auto skipAnyStateTokenPart = [](std::string_view tokenNameSv) {
            static const std::unordered_set<std::string_view> stateNames = {"pressed", "hovered", "focused", "disabled"};

            const size_t pos1             = 0;
            const bool   isFoundStatePart = false;

            std::string result;
            result.reserve(tokenNameSv.size());

            size_t start = 0;
            while (start < tokenNameSv.size())
            {
                const size_t dotPos = tokenNameSv.find('.', start);
                const size_t end    = (dotPos == std::string_view::npos) ? tokenNameSv.size() : dotPos;

                const std::string_view part = tokenNameSv.substr(start, end - start);

                if (!stateNames.contains(part))
                {
                    if (!result.empty() && result.back() != '.')
                    {
                        result += '.';
                    }
                    result.append(part);
                }

                start = (dotPos == std::string_view::npos) ? tokenNameSv.size() : dotPos + 1;
            }

            return std::move(result);
        };

        // sort tokens by reverse lexicographical order of their names (compare from the end)
        // using rbegin()/rend() makes tokens with similar suffixes group together; this is intentional
        std::ranges::sort(tokenSet.tokens, [&](const Token &a, const Token &b) -> bool {
            std::string aTokenNameSort = skipAnyStateTokenPart(a.tokenName);
            std::string bTokenNameSort = skipAnyStateTokenPart(b.tokenName);

            return std::ranges::lexicographical_compare(std::ranges::reverse_view(aTokenNameSort), std::ranges::reverse_view(bTokenNameSort));
        });

        for (auto &token : tokenSet.tokens)
        {
            token.normalize();
        }
    }

    std::ofstream outputFile;
    outputFile.open(outputFilePath, std::ios::out | std::ios::trunc);

    std::ranges::copy(tokenSets, std::ostream_iterator<TokenSet>(outputFile, "\n"));

    // auto displayGroups = parse_display_groups(doc["system"]["displayGroups"]);
    // parse_tokens(doc["system"]["tokens"], doc, displayGroups);
    // std::ranges::copy(displayGroups, std::ostream_iterator<DisplayGroup>(outputFile, "\n"));
}
