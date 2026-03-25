#ifndef __COCOS_STACK_VIEW_H__
#define __COCOS_STACK_VIEW_H__

#include "views/StackView.h"
#include "cocos2d.h"

// Cocos手牌堆实现：顶部牌、stack预览、Draw按钮与动画。
class CocosStackView : public cocos2d::Node, public StackView
{
public:
    static CocosStackView* create();

    bool init() override;

    // @param gameModel 只读模型。
    // @param controller stack控制器。
    void bind(const GameModel* gameModel, const StackController* controller) override;

    // @param callback Draw点击回调。
    void setOnDrawClick(const std::function<void()>& callback) override;

    // @param cardFace/cardSuit 动画牌点数与花色。
    // @param fromX/fromY 抽牌动画起点（stack位置）。
    // @param toX/toY 抽牌动画终点（顶部牌位置）。
    void playDrawAnimation(int32_t cardFace,
                           int32_t cardSuit,
                           float fromX,
                           float fromY,
                           float toX,
                           float toY) override;

    // @param cardFace/cardSuit 回退牌点数与花色。
    // @param fromX/fromY 回退动画起点（顶部牌位置）。
    // @param toX/toY 回退动画终点（stack位置）。
    void playUndoDrawAnimation(int32_t cardFace,
                               int32_t cardSuit,
                               float fromX,
                               float fromY,
                               float toX,
                               float toY) override;

    // @param remainingCount stack剩余张数。
    void updateStackDisplay(int32_t remainingCount) override;

    // @param gameOver 是否处于结束状态（禁用Draw点击）。
    void setGameOverState(bool gameOver) override;

private:
    void _initButton();
    void _refreshStackCardsPreview();

    // @param cardFace/cardSuit 渲染牌面点数和花色。
    cocos2d::Node* _createCardNode(int32_t cardFace, int32_t cardSuit) const;

private:
    const GameModel* _gameModel;
    const StackController* _controller;
    std::function<void()> _onDrawClick;
    cocos2d::Label* _countLabel;
    cocos2d::DrawNode* _drawButton;
    cocos2d::Node* _trayCardNode;
    cocos2d::Node* _stackCardsLayer;
    bool _isGameOver;
};

#endif // __COCOS_STACK_VIEW_H__
