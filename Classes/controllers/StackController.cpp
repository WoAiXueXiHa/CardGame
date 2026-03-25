#include "StackController.h"
#include "views/StackView.h"

StackController::StackController()
    : _gameModel(nullptr), _undoManager(nullptr), _view(nullptr)
{
}

StackController::~StackController()
{
    clear();
}

bool StackController::init(GameModel* gameModel, UndoManager* undoManager, StackView* view)
{
    if (!gameModel || !undoManager || !view) {
        return false;
    }

    _gameModel = gameModel;
    _undoManager = undoManager;
    _view = view;

    return true;
}

bool StackController::drawCard()
{
    if (!_gameModel || !_undoManager || !_view) {
        return false;
    }

    if (!hasRemainingCards()) {
        return false;
    }

    CardData oldTrayCard = _gameModel->getTrayTopCard();

    CardData newStackCard = _gameModel->popStackCard();
    if (newStackCard.cardId == -1) {
        return false;
    }

    if (newStackCard.posX == 0.0f && newStackCard.posY == 0.0f) {
        newStackCard.posX = 860.0f;
        newStackCard.posY = 240.0f;
    }

    _recordUndo(newStackCard, oldTrayCard);

    CardData newTrayCard = newStackCard;
    newTrayCard.posX = oldTrayCard.posX;
    newTrayCard.posY = oldTrayCard.posY;
    _gameModel->setTrayTopCard(newTrayCard);

    _view->playDrawAnimation(newTrayCard.cardFace,
                             newTrayCard.cardSuit,
                             newStackCard.posX,
                             newStackCard.posY,
                             oldTrayCard.posX,
                             oldTrayCard.posY);
    _view->updateStackDisplay(_gameModel->getStackCardCount());

    return true;
}

bool StackController::undoDrawCard(const UndoRecord& record)
{
    if (!_gameModel || !_view) {
        return false;
    }

    _gameModel->setTrayTopCard(record.targetCard);

    CardData restoredStackCard = record.sourceCard;
    restoredStackCard.posX = record.sourceX;
    restoredStackCard.posY = record.sourceY;
    _gameModel->addStackCard(restoredStackCard);

    _view->playUndoDrawAnimation(record.sourceCard.cardFace,
                                 record.sourceCard.cardSuit,
                                 record.targetX,
                                 record.targetY,
                                 record.sourceX,
                                 record.sourceY);
    _view->updateStackDisplay(_gameModel->getStackCardCount());

    return true;
}

bool StackController::hasRemainingCards() const
{
    if (!_gameModel) {
        return false;
    }
    return _gameModel->getStackCardCount() > 0;
}

int32_t StackController::getRemainingCardCount() const
{
    if (!_gameModel) {
        return 0;
    }
    return _gameModel->getStackCardCount();
}

void StackController::clear()
{
    _gameModel = nullptr;
    _undoManager = nullptr;
    _view = nullptr;
}

void StackController::_recordUndo(const CardData& newTrayCard, const CardData& oldTrayCard)
{
    if (!_undoManager) {
        return;
    }

    UndoRecord record;
    record.operationType = 1;
    record.sourceCard = newTrayCard;
    record.targetCard = oldTrayCard;
    record.sourceX = newTrayCard.posX;
    record.sourceY = newTrayCard.posY;
    record.targetX = oldTrayCard.posX;
    record.targetY = oldTrayCard.posY;

    _undoManager->pushRecord(record);
}
