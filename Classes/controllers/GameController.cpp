#include "GameController.h"
#include "views/GameView.h"
#include "views/PlayFieldView.h"
#include "views/StackView.h"
#include "cocos2d.h"

GameController::GameController()
    : _undoManager(100),
      _gameView(nullptr),
      _playFieldView(nullptr),
      _stackView(nullptr)
{
}

GameController::~GameController()
{
    clear();
}

bool GameController::init(GameView* gameView,
                          PlayFieldView* playFieldView,
                          StackView* stackView)
{
    if (!gameView || !playFieldView || !stackView) {
        return false;
    }

    _gameView = gameView;
    _playFieldView = playFieldView;
    _stackView = stackView;

    if (!_playFieldController.init(&_gameModel, &_undoManager, _playFieldView)) {
        return false;
    }
    if (!_stackController.init(&_gameModel, &_undoManager, _stackView)) {
        return false;
    }

    _playFieldView->bind(&_gameModel, &_playFieldController);
    _stackView->bind(&_gameModel, &_stackController);

    _bindViewEvents();
    return true;
}

bool GameController::startGame(const std::string& levelFilePath)
{
    _levelFilePath = levelFilePath;
    LevelConfig levelConfig;
    if (!_levelConfigLoader.loadFromFile(levelFilePath, levelConfig)) {
        return false;
    }
    if (!_gameModelGenerator.generate(levelConfig, _gameModel)) {
        return false;
    }

    _undoManager.clear();
    _playFieldController.refreshFlippedState();
    _refreshGameOverState();

    if (_playFieldView) {
        _playFieldView->refresh();
    }
    if (_stackView) {
        _stackView->updateStackDisplay(_gameModel.getStackCardCount());
    }
    return true;
}

bool GameController::restartGame()
{
    if (_levelFilePath.empty()) {
        return false;
    }
    if (_gameView) {
        _gameView->setGameOverState(false);
    }
    if (_stackView) {
        _stackView->setGameOverState(false);
    }
    return startGame(_levelFilePath);
}

void GameController::quitGame()
{
    cocos2d::Director::getInstance()->end();
}

bool GameController::handleUndoClick()
{
    const UndoRecord* topRecord = _undoManager.peekRecord();
    if (!topRecord) {
        return false;
    }

    if (topRecord->operationType == 0) {
        return _playFieldController.executeUndo();
    }

    if (topRecord->operationType == 1) {
        const UndoRecord record = _undoManager.popRecord();
        return _stackController.undoDrawCard(record);
    }

    return false;
}

void GameController::clear()
{
    _playFieldController.clear();
    _stackController.clear();
    _undoManager.clear();
    _gameModel.clear();

    _gameView = nullptr;
    _playFieldView = nullptr;
    _stackView = nullptr;
}

void GameController::_bindViewEvents()
{
    if (!_gameView) {
        return;
    }

    _gameView->setOnPlayFieldCardClick([this](int32_t cardId) {
        if (_gameModel.isGameOver()) {
            return;
        }
        if (_playFieldController.handleCardClick(cardId)) {
            _refreshGameOverState();
        }
    });

    _gameView->setOnDrawCardClick([this]() {
        if (_gameModel.isGameOver()) {
            return;
        }
        if (_stackController.drawCard()) {
            _refreshGameOverState();
        }
    });

    _gameView->setOnUndoClick([this]() {
        if (handleUndoClick()) {
            _refreshGameOverState();
        }
    });

    _gameView->setOnRestartClick([this]() {
        restartGame();
    });

    _gameView->setOnQuitClick([this]() {
        quitGame();
    });
}

void GameController::_refreshGameOverState()
{
    bool hasUnmatched = false;
    for (const auto& card : _gameModel.getPlayFieldCards()) {
        if (!card.isMatched) {
            hasUnmatched = true;
            break;
        }
    }

    bool gameOver = false;
    if (!hasUnmatched) {
        gameOver = true;
    } else {
        const int32_t trayFace = _gameModel.getTrayTopCard().cardFace;
        bool hasMove = false;
        for (const auto& card : _gameModel.getPlayFieldCards()) {
            if (card.isMatched || !card.isFlipped) {
                continue;
            }
            const int32_t diff = card.cardFace - trayFace;
            if (diff == 1 || diff == -1) {
                hasMove = true;
                break;
            }
        }

        const bool canDraw = _stackController.hasRemainingCards();
        gameOver = !hasMove && !canDraw;
    }

    _gameModel.setGameOver(gameOver);
    if (_gameView) {
        _gameView->setGameOverState(gameOver);
    }
    if (_stackView) {
        _stackView->setGameOverState(gameOver);
    }
}
