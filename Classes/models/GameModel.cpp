#include "GameModel.h"
#include <algorithm>
#include <regex>
#include <sstream>

namespace
{
std::string cardToJson(const CardData& card)
{
    std::ostringstream oss;
    oss << "{"
        << "\"cardId\":" << card.cardId << ","
        << "\"cardFace\":" << card.cardFace << ","
        << "\"cardSuit\":" << card.cardSuit << ","
        << "\"posX\":" << card.posX << ","
        << "\"posY\":" << card.posY << ","
        << "\"isFlipped\":" << (card.isFlipped ? 1 : 0) << ","
        << "\"isMatched\":" << (card.isMatched ? 1 : 0)
        << "}";
    return oss.str();
}

bool parseCardFromMatch(const std::smatch& m, CardData& out)
{
    out.cardId = std::stoi(m[1].str());
    out.cardFace = std::stoi(m[2].str());
    out.cardSuit = std::stoi(m[3].str());
    out.posX = std::stof(m[4].str());
    out.posY = std::stof(m[5].str());
    out.isFlipped = std::stoi(m[6].str()) != 0;
    out.isMatched = std::stoi(m[7].str()) != 0;
    return true;
}

bool extractSliceByTokenOrder(const std::string& jsonString,
                              const std::string& fromToken,
                              const std::string& toToken,
                              std::string& outSlice)
{
    const size_t fromPos = jsonString.find(fromToken);
    if (fromPos == std::string::npos) {
        return false;
    }

    if (toToken.empty()) {
        outSlice = jsonString.substr(fromPos);
        return true;
    }

    const size_t toPos = jsonString.find(toToken);
    if (toPos == std::string::npos || toPos <= fromPos) {
        return false;
    }

    outSlice = jsonString.substr(fromPos, toPos - fromPos);
    return true;
}

void parseCardsFromSlice(const std::string& slice,
                         const std::regex& cardPattern,
                         std::vector<CardData>& outCards)
{
    for (std::sregex_iterator it(slice.begin(), slice.end(), cardPattern), end; it != end; ++it) {
        CardData card;
        parseCardFromMatch(*it, card);
        outCards.push_back(card);
    }
}
}

GameModel::GameModel()
    : _isGameOver(false)
{
}

GameModel::~GameModel()
{
    clear();
}

void GameModel::addPlayFieldCard(const CardData& card)
{
    _playFieldCards.push_back(card);
}

const std::vector<CardData>& GameModel::getPlayFieldCards() const
{
    return _playFieldCards;
}

std::vector<CardData>& GameModel::getPlayFieldCardsMutable()
{
    return _playFieldCards;
}

CardData* GameModel::getPlayFieldCard(int32_t cardId)
{
    auto it = std::find_if(_playFieldCards.begin(), _playFieldCards.end(),
        [cardId](const CardData& card) { return card.cardId == cardId; });

    if (it != _playFieldCards.end()) {
        return &(*it);
    }
    return nullptr;
}

bool GameModel::removePlayFieldCard(int32_t cardId)
{
    auto it = std::find_if(_playFieldCards.begin(), _playFieldCards.end(),
        [cardId](const CardData& card) { return card.cardId == cardId; });

    if (it != _playFieldCards.end()) {
        _playFieldCards.erase(it);
        return true;
    }
    return false;
}

void GameModel::setTrayTopCard(const CardData& card)
{
    _trayTopCard = card;
}

const CardData& GameModel::getTrayTopCard() const
{
    return _trayTopCard;
}

CardData* GameModel::getTrayTopCardMutable()
{
    return &_trayTopCard;
}

void GameModel::addStackCard(const CardData& card)
{
    _stackCards.push_back(card);
}

const std::vector<CardData>& GameModel::getStackCards() const
{
    return _stackCards;
}

CardData GameModel::popStackCard()
{
    if (_stackCards.empty()) {
        return CardData();
    }

    CardData card = _stackCards.back();
    _stackCards.pop_back();
    return card;
}

int32_t GameModel::getStackCardCount() const
{
    return static_cast<int32_t>(_stackCards.size());
}

std::string GameModel::toJsonString() const
{
    std::ostringstream oss;
    oss << "{\"isGameOver\":" << (_isGameOver ? 1 : 0) << ",";

    oss << "\"trayTop\":" << cardToJson(_trayTopCard) << ",";

    oss << "\"playField\":[";
    for (size_t i = 0; i < _playFieldCards.size(); ++i) {
        if (i > 0) {
            oss << ",";
        }
        oss << cardToJson(_playFieldCards[i]);
    }
    oss << "],";

    oss << "\"stack\":[";
    for (size_t i = 0; i < _stackCards.size(); ++i) {
        if (i > 0) {
            oss << ",";
        }
        oss << cardToJson(_stackCards[i]);
    }
    oss << "]}";

    return oss.str();
}

bool GameModel::loadFromJsonString(const std::string& jsonString)
{
    if (jsonString.empty()) {
        return false;
    }

    const std::regex gameOverPattern(R"("isGameOver"\s*:\s*(\d+))");
    const std::regex trayPattern(
        R"("trayTop"\s*:\s*\{\s*"cardId"\s*:\s*(-?\d+)\s*,\s*"cardFace"\s*:\s*(-?\d+)\s*,\s*"cardSuit"\s*:\s*(-?\d+)\s*,\s*"posX"\s*:\s*(-?\d+(?:\.\d+)?)\s*,\s*"posY"\s*:\s*(-?\d+(?:\.\d+)?)\s*,\s*"isFlipped"\s*:\s*(\d+)\s*,\s*"isMatched"\s*:\s*(\d+)\s*\})");
    const std::regex cardPattern(
        R"(\{\s*"cardId"\s*:\s*(-?\d+)\s*,\s*"cardFace"\s*:\s*(-?\d+)\s*,\s*"cardSuit"\s*:\s*(-?\d+)\s*,\s*"posX"\s*:\s*(-?\d+(?:\.\d+)?)\s*,\s*"posY"\s*:\s*(-?\d+(?:\.\d+)?)\s*,\s*"isFlipped"\s*:\s*(\d+)\s*,\s*"isMatched"\s*:\s*(\d+)\s*\})");

    std::smatch m;
    if (!std::regex_search(jsonString, m, gameOverPattern)) {
        return false;
    }
    const bool loadedGameOver = std::stoi(m[1].str()) != 0;

    if (!std::regex_search(jsonString, m, trayPattern)) {
        return false;
    }
    CardData loadedTray;
    parseCardFromMatch(m, loadedTray);

    std::string playFieldSlice;
    std::string stackSlice;
    if (!extractSliceByTokenOrder(jsonString, "\"playField\"", "\"stack\"", playFieldSlice)) {
        return false;
    }
    if (!extractSliceByTokenOrder(jsonString, "\"stack\"", "", stackSlice)) {
        return false;
    }

    clear();
    _isGameOver = loadedGameOver;
    _trayTopCard = loadedTray;

    parseCardsFromSlice(playFieldSlice, cardPattern, _playFieldCards);
    parseCardsFromSlice(stackSlice, cardPattern, _stackCards);
    return true;
}

void GameModel::clear()
{
    _playFieldCards.clear();
    _stackCards.clear();
    _trayTopCard = CardData();
    _isGameOver = false;
}

bool GameModel::isGameOver() const
{
    return _isGameOver;
}

void GameModel::setGameOver(bool over)
{
    _isGameOver = over;
}
