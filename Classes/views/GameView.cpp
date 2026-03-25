#include "GameView.h"

GameView::GameView()
{
}

GameView::~GameView()
{
}

void GameView::setOnPlayFieldCardClick(const std::function<void(int32_t)>& callback)
{
    _onPlayFieldCardClick = callback;
}

void GameView::setOnDrawCardClick(const std::function<void()>& callback)
{
    _onDrawCardClick = callback;
}

void GameView::setOnUndoClick(const std::function<void()>& callback)
{
    _onUndoClick = callback;
}

void GameView::setOnRestartClick(const std::function<void()>& callback)
{
    _onRestartClick = callback;
}

void GameView::setOnQuitClick(const std::function<void()>& callback)
{
    _onQuitClick = callback;
}

void GameView::emitPlayFieldCardClick(int32_t cardId) const
{
    if (_onPlayFieldCardClick) {
        _onPlayFieldCardClick(cardId);
    }
}

void GameView::emitDrawCardClick() const
{
    if (_onDrawCardClick) {
        _onDrawCardClick();
    }
}

void GameView::emitUndoClick() const
{
    if (_onUndoClick) {
        _onUndoClick();
    }
}

void GameView::emitRestartClick() const
{
    if (_onRestartClick) {
        _onRestartClick();
    }
}

void GameView::emitQuitClick() const
{
    if (_onQuitClick) {
        _onQuitClick();
    }
}

void GameView::setGameOverState(bool gameOver)
{
    (void)gameOver;
}
