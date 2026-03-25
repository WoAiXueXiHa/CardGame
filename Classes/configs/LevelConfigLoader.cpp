#include "LevelConfigLoader.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <vector>

namespace
{

bool extractArrayContent(const std::string& text,
                         const std::string& key,
                         std::string& outArrayContent)
{
    const std::string token = "\"" + key + "\"";
    size_t keyPos = text.find(token);
    if (keyPos == std::string::npos) {
        return false;
    }

    size_t leftBracketPos = text.find('[', keyPos);
    if (leftBracketPos == std::string::npos) {
        return false;
    }

    int depth = 0;
    for (size_t i = leftBracketPos; i < text.size(); ++i) {
        if (text[i] == '[') {
            ++depth;
        } else if (text[i] == ']') {
            --depth;
            if (depth == 0) {
                outArrayContent = text.substr(leftBracketPos + 1, i - leftBracketPos - 1);
                return true;
            }
        }
    }

    return false;
}


std::vector<CardConfig> parseCardList(const std::string& arrayContent)
{
    std::vector<CardConfig> cards;

    const std::regex cardPattern(
        R"(\{\s*"CardFace"\s*:\s*(-?\d+)\s*,\s*"CardSuit"\s*:\s*(-?\d+)\s*,\s*"Position"\s*:\s*\{\s*"x"\s*:\s*(-?\d+(?:\.\d+)?)\s*,\s*"y"\s*:\s*(-?\d+(?:\.\d+)?)\s*\}\s*\})");

    std::sregex_iterator begin(arrayContent.begin(), arrayContent.end(), cardPattern);
    std::sregex_iterator end;

    for (std::sregex_iterator it = begin; it != end; ++it) {
        const std::smatch& m = *it;
        CardConfig cfg;
        cfg.cardFace = std::stoi(m[1].str());
        cfg.cardSuit = std::stoi(m[2].str());
        cfg.posX = std::stof(m[3].str());
        cfg.posY = std::stof(m[4].str());
        cards.push_back(cfg);
    }

    return cards;
}
} // namespace

LevelConfigLoader::LevelConfigLoader()
{
}

LevelConfigLoader::~LevelConfigLoader()
{
}

bool LevelConfigLoader::loadFromFile(const std::string& filePath, LevelConfig& outConfig)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return loadFromString(buffer.str(), outConfig);
}

bool LevelConfigLoader::loadFromString(const std::string& jsonString, LevelConfig& outConfig)
{
    if (jsonString.empty()) {
        return false;
    }

    outConfig.clear();

    if (!_parsePlayFieldCards(jsonString, outConfig)) {
        return false;
    }

    if (!_parseStackCards(jsonString, outConfig)) {
        return false;
    }

    return true;
}

bool LevelConfigLoader::_parsePlayFieldCards(const std::string& jsonString, LevelConfig& outConfig)
{
    std::string playfieldContent;
    if (!extractArrayContent(jsonString, "Playfield", playfieldContent)) {
        return false;
    }

    const std::vector<CardConfig> cards = parseCardList(playfieldContent);
    if (cards.empty()) {
        return false;
    }

    for (const CardConfig& cfg : cards) {
        outConfig.addPlayFieldCard(cfg);
    }

    return true;
}

bool LevelConfigLoader::_parseStackCards(const std::string& jsonString, LevelConfig& outConfig)
{
    std::string stackContent;
    if (!extractArrayContent(jsonString, "Stack", stackContent)) {
        return false;
    }

    const std::vector<CardConfig> cards = parseCardList(stackContent);
    if (cards.empty()) {
        return false;
    }

    outConfig.setInitialTrayCard(cards.front());
    for (size_t i = 1; i < cards.size(); ++i) {
        outConfig.addStackCard(cards[i]);
    }

    return true;
}
