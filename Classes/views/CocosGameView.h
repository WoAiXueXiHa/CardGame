#ifndef __COCOS_GAME_VIEW_H__
#define __COCOS_GAME_VIEW_H__

#include "views/GameView.h"
#include "cocos2d.h"

// Cocos层：实现Undo按钮、GameOver面板（New Game/Quit）并透传事件。
class CocosGameView : public cocos2d::Layer, public GameView
{
public:
    static CocosGameView* create();

    bool init() override;
    void setGameOverState(bool gameOver) override;

private:
    // 构建Undo按钮（形状/文本/触摸区域）。
    void _initUndoButton();

    // 构建GameOver弹出面板（标题/New Game/Quit）。
    void _initGameOverPanel();

private:
    cocos2d::DrawNode* _undoButton;
    cocos2d::Node* _gameOverPanel;
    bool _isGameOver;
};

#endif // __COCOS_GAME_VIEW_H__
