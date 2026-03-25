#include "PlayFieldController.h"
#include "views/PlayFieldView.h"
#include <cmath>

namespace
{
constexpr float kCoverOverlapX = 120.0f;
constexpr float kCoverOverlapY = 260.0f;
}

PlayFieldController::PlayFieldController()
    : _gameModel(nullptr), _undoManager(nullptr), _view(nullptr)
{
}

PlayFieldController::~PlayFieldController()
{
    clear();
}

bool PlayFieldController::init(GameModel* gameModel, UndoManager* undoManager, PlayFieldView* view)
{
    if (!gameModel || !undoManager || !view) {
        return false;
    }

    _gameModel = gameModel;
    _undoManager = undoManager;
    _view = view;
    refreshFlippedState();
    return true;
}

bool PlayFieldController::handleCardClick(int32_t cardId)
{
    if (!_gameModel || !_undoManager || !_view) {
        return false;
    }

    CardData* playFieldCard = _gameModel->getPlayFieldCard(cardId);
    if (!playFieldCard) {
        return false;
    }

    if (playFieldCard->isMatched || !playFieldCard->isFlipped) {
        return false;
    }

    const CardData& trayCard = _gameModel->getTrayTopCard();
    if (!_isValidMatch(playFieldCard->cardFace, trayCard.cardFace)) {
        return false;
    }

    return _replaceWithPlayFieldCard(playFieldCard);
}

bool PlayFieldController::executeUndo()
{
    if (!_gameModel || !_undoManager || !_view || !_undoManager->canUndo()) {
        return false;
    }

    UndoRecord record = _undoManager->popRecord();

    CardData* playFieldCard = _gameModel->getPlayFieldCard(record.sourceCard.cardId);
    if (playFieldCard) {
        playFieldCard->isMatched = false;
        playFieldCard->posX = record.sourceX;
        playFieldCard->posY = record.sourceY;
    } else {
        CardData restored = record.sourceCard;
        restored.posX = record.sourceX;
        restored.posY = record.sourceY;
        restored.isMatched = false;
        _gameModel->addPlayFieldCard(restored);
    }

    CardData restoredTray = record.targetCard;
    restoredTray.posX = record.targetX;
    restoredTray.posY = record.targetY;
    _gameModel->setTrayTopCard(restoredTray);

    refreshFlippedState();

    _view->playUndoAnimation(record.sourceCard.cardId,
                             record.sourceX,
                             record.sourceY,
                             record.targetCard.cardId,
                             record.targetX,
                             record.targetY);
    return true;
}

void PlayFieldController::refreshFlippedState()
{
    if (!_gameModel) {
        return;
    }

    auto& cards = _gameModel->getPlayFieldCardsMutable();
    for (auto& card : cards) {
        if (card.isMatched) {
            card.isFlipped = false;
            continue;
        }

        bool covered = false;
        for (const auto& other : cards) {
            if (other.cardId == card.cardId || other.isMatched) {
                continue;
            }

            const bool above = other.posY < card.posY;
            const bool overlapX = std::fabs(other.posX - card.posX) <= kCoverOverlapX;
            const bool nearY = (card.posY - other.posY) <= kCoverOverlapY;
            if (above && overlapX && nearY) {
                covered = true;
                break;
            }
        }

        card.isFlipped = !covered;
    }
}

bool PlayFieldController::canUndo() const
{
    return _undoManager ? _undoManager->canUndo() : false;
}

void PlayFieldController::clear()
{
    _gameModel = nullptr;
    _undoManager = nullptr;
    _view = nullptr;
}

bool PlayFieldController::_isValidMatch(int32_t cardFace1, int32_t cardFace2) const
{
    const int32_t diff = cardFace1 - cardFace2;
    return diff == 1 || diff == -1;
}

void PlayFieldController::_recordUndo(const CardData& sourceCard,
                                      const CardData& targetCard,
                                      float sourceX,
                                      float sourceY,
                                      float targetX,
                                      float targetY)
{
    UndoRecord record;
    record.operationType = 0;
    record.sourceCard = sourceCard;
    record.targetCard = targetCard;
    record.sourceX = sourceX;
    record.sourceY = sourceY;
    record.targetX = targetX;
    record.targetY = targetY;
    _undoManager->pushRecord(record);
}

bool PlayFieldController::_replaceWithPlayFieldCard(CardData* playFieldCard)
{
    if (!playFieldCard || !_gameModel || !_view) {
        return false;
    }

    CardData trayCard = _gameModel->getTrayTopCard();

    _recordUndo(*playFieldCard,
                trayCard,
                playFieldCard->posX,
                playFieldCard->posY,
                trayCard.posX,
                trayCard.posY);

    playFieldCard->isMatched = true;

    CardData newTray = *playFieldCard;
    newTray.posX = trayCard.posX;
    newTray.posY = trayCard.posY;
    newTray.isMatched = false;
    _gameModel->setTrayTopCard(newTray);

    refreshFlippedState();

    _view->playMatchAnimation(playFieldCard->cardId, trayCard.posX, trayCard.posY);
    return true;
}
