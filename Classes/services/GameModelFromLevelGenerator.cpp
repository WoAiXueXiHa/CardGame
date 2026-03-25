#include "GameModelFromLevelGenerator.h"

GameModelFromLevelGenerator::GameModelFromLevelGenerator()
{
}

GameModelFromLevelGenerator::~GameModelFromLevelGenerator()
{
}

bool GameModelFromLevelGenerator::generate(const LevelConfig& levelConfig, GameModel& outGameModel)
{
    outGameModel.clear();

    int32_t nextCardId = 0;

    const std::vector<CardConfig>& playFieldCards = levelConfig.getPlayFieldCards();
    for (const CardConfig& cardConfig : playFieldCards) {
        int32_t cardId = _generateCardId(nextCardId);
        CardData cardData = _convertCardConfigToCardData(cardConfig, cardId, true);
        outGameModel.addPlayFieldCard(cardData);
    }

    const std::vector<CardConfig>& stackCards = levelConfig.getStackCards();
    for (const CardConfig& cardConfig : stackCards) {
        int32_t cardId = _generateCardId(nextCardId);
        CardData cardData = _convertCardConfigToCardData(cardConfig, cardId, true);
        if (cardData.posX == 0.0f && cardData.posY == 0.0f) {
            cardData.posX = 860.0f;
            cardData.posY = 240.0f;
        }
        outGameModel.addStackCard(cardData);
    }

    const CardConfig& initialTrayConfig = levelConfig.getInitialTrayCard();
    int32_t trayCardId = _generateCardId(nextCardId);
    CardData trayCardData = _convertCardConfigToCardData(initialTrayConfig, trayCardId, true);
    if (trayCardData.posX == 0.0f && trayCardData.posY == 0.0f) {
        trayCardData.posX = 540.0f;
        trayCardData.posY = 220.0f;
    }
    outGameModel.setTrayTopCard(trayCardData);

    return true;
}

int32_t GameModelFromLevelGenerator::_generateCardId(int32_t& nextCardId) const
{
    return nextCardId++;
}

CardData GameModelFromLevelGenerator::_convertCardConfigToCardData(const CardConfig& cardConfig,
                                                                   int32_t cardId,
                                                                   bool isFlipped) const
{
    CardData cardData;
    cardData.cardId = cardId;
    cardData.cardFace = cardConfig.cardFace;
    cardData.cardSuit = cardConfig.cardSuit;
    cardData.posX = cardConfig.posX;
    cardData.posY = cardConfig.posY;
    cardData.isFlipped = isFlipped;
    cardData.isMatched = false;
    return cardData;
}
