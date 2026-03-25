#include "LevelConfig.h"

LevelConfig::LevelConfig()
    : _levelId(-1)
{
}

LevelConfig::~LevelConfig()
{
    clear();
}

void LevelConfig::setLevelId(int32_t levelId)
{
    _levelId = levelId;
}

int32_t LevelConfig::getLevelId() const
{
    return _levelId;
}

void LevelConfig::addPlayFieldCard(const CardConfig& cardConfig)
{
    _playFieldCards.push_back(cardConfig);
}

const std::vector<CardConfig>& LevelConfig::getPlayFieldCards() const
{
    return _playFieldCards;
}

int32_t LevelConfig::getPlayFieldCardCount() const
{
    return static_cast<int32_t>(_playFieldCards.size());
}

void LevelConfig::addStackCard(const CardConfig& cardConfig)
{
    _stackCards.push_back(cardConfig);
}

const std::vector<CardConfig>& LevelConfig::getStackCards() const
{
    return _stackCards;
}

int32_t LevelConfig::getStackCardCount() const
{
    return static_cast<int32_t>(_stackCards.size());
}

void LevelConfig::setInitialTrayCard(const CardConfig& cardConfig)
{
    _initialTrayCard = cardConfig;
}

const CardConfig& LevelConfig::getInitialTrayCard() const
{
    return _initialTrayCard;
}

void LevelConfig::clear()
{
    _levelId = -1;
    _playFieldCards.clear();
    _stackCards.clear();
    _initialTrayCard = CardConfig();
}
