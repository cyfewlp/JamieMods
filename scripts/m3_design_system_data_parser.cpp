#include "../common/json/simdjson.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

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
    std::string             name;
    std::string             tokenName;
    std::string             displayName;
    std::string             type;
    std::vector<TokenValue> values;

    auto toString() const -> std::string
    {
        return std::format(
            R"(    {}
    tokenName: {}
    {}
    {}
)",
            displayName,
            tokenName,
            type,
            values.empty() ? "" : values[0].value
        );
    }
};

struct TokenSet
{
    std::string        name;
    std::string        tokenSetName;
    std::vector<Token> tokens;
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
            printf("[Warning] Unsupported length unit: %s \n", tUnit.data());
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
        std::cout << "[Error] Failed to parse values.  " << jValues.error() << std::endl;
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

std::ostream &operator<<(std::ostream &os, const Token &token)
{
    return os << token.toString();
}

std::ostream &operator<<(std::ostream &os, const TokenSet &tokenSet)
{
    auto &os1 = os << tokenSet.name << ": " << tokenSet.tokenSetName << '\n';
    std::ranges::copy(tokenSet.tokens, std::ostream_iterator<Token>(os1, "\n"));
    return os1;
}

auto parse_tokenSets(simdjson_result<ondemand::value> jTokenSets) -> std::vector<TokenSet>
{
    if (jTokenSets.error() != SUCCESS)
    {
        std::cout << "[Error] Failed to parse tokenSets!" << std::endl;
    }
    std::vector<TokenSet> tokenSets;
    for (auto jTokenSet : jTokenSets.get_array())
    {
        TokenSet tokenSet;
        if (SUCCESS == jTokenSet["name"].get(tokenSet.name) &&
            SUCCESS == jTokenSet["tokenSetName"].get(tokenSet.tokenSetName))
        {
            tokenSet.name.erase(0, token_set_prefix_length);
            tokenSets.push_back(std::move(tokenSet));
        }
    }
    return tokenSets;
}

auto parse_tokens(simdjson_result<ondemand::value> jTokens, std::vector<TokenSet> &tokenSets)
{
    if (jTokens.error() != SUCCESS)
    {
        std::cout << "[Error] Failed to parse tokens!" << std::endl;
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
            auto tokenSetId = token.name.substr(token_set_prefix_length, 16U);
            auto tokenId    = token.name.substr(tokens_prefix_length, 16U);
            if (tokenSetCache.contains(tokenSetId))
            {
                auto tokenSetIndex = tokenSetCache[tokenSetId];
                if (SUCCESS == jToken["tokenName"].get(token.tokenName) &&
                    SUCCESS == jToken["displayName"].get(token.displayName) &&
                    SUCCESS == jToken["tokenValueType"].get(token.type))
                {
                    tokenSets[tokenSetIndex].tokens.push_back(std::move(token));
                }
                else
                {
                    std::cout << "[Error] Failed to parse token: " << token.name.c_str() << std::endl;
                }
            }
        }
        else
        {
            std::cout << "[Error] Failed to parse token: Not have a name key!" << std::endl;
        }
    }
}

auto parse_contextualReferenceTrees(
    simdjson_result<ondemand::value> jRefTrees, std::vector<TokenSet> &tokenSets, TokenValueCache &valueCache
)
{
    for (auto &tokenSet : tokenSets)
    {
        for (auto &token : tokenSet.tokens)
        {
            if (auto jObj = jRefTrees[token.name].get_object(); jObj.error() == SUCCESS)
            {
                for (auto ref : jObj["contextualReferenceTree"].get_array())
                {
                    std::string refTokenName;
                    token_type  tokenType = token_type::unsupported;

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
                        token.values.push_back(std::move(tokenValue));
                    }
                }
            }
        }
    }
}

} // namespace

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: m3_design_system_data_parser.exe spec.json" << std::endl;
        return -1;
    }

    std::filesystem::path filePath(argv[1]);
    std::filesystem::path outputFilePath;

    if (filePath.has_parent_path())
    {
        outputFilePath = filePath.parent_path();
    }
    outputFilePath.append("m3_normalized_spec_data.txt");

    ondemand::parser parser;
    auto             json_result = padded_string::load(filePath.generic_string());
    if (json_result.error() != SUCCESS)
    {
        std::cerr << "Error loading JSON: " << json_result.error() << std::endl;
        return 1;
    }
    auto doc = parser.iterate(json_result);

    auto tokenSets = parse_tokenSets(doc["system"]["tokenSets"]);
    parse_tokens(doc["system"]["tokens"], tokenSets);
    TokenValueCache valueCache = parse_values(doc["system"]["values"]);
    parse_contextualReferenceTrees(doc["system"]["contextualReferenceTrees"], tokenSets, valueCache);
    valueCache.clear();

    std::ofstream outputFile;
    outputFile.open(outputFilePath, std::ios::out | std::ios::trunc);

    std::ranges::copy(tokenSets, std::ostream_iterator<TokenSet>(outputFile, "\n"));

    // auto displayGroups = parse_display_groups(doc["system"]["displayGroups"]);
    // parse_tokens(doc["system"]["tokens"], doc, displayGroups);
    // std::ranges::copy(displayGroups, std::ostream_iterator<DisplayGroup>(outputFile, "\n"));
}
