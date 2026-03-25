#ifndef __STACK_VIEW_H__
#define __STACK_VIEW_H__

#include <cstdint>
#include <functional>

class GameModel;
class StackController;

// 手牌堆视图接口。
class StackView
{
public:
    StackView();
    virtual ~StackView();

    // @param gameModel 只读模型。
    // @param controller 手续逻辑控制器。
    virtual void bind(const GameModel* gameModel, const StackController* controller) = 0;

    // @param callback 抽牌点击回调。
    virtual void setOnDrawClick(const std::function<void()>& callback) = 0;

    // @param cardFace/cardSuit 动画牌点数和花色。
    // @param fromX/fromY 动画起点（stack位置）。
    // @param toX/toY 动画终点（顶部牌位置）。
    virtual void playDrawAnimation(int32_t cardFace,
                                   int32_t cardSuit,
                                   float fromX,
                                   float fromY,
                                   float toX,
                                   float toY) = 0;

    // @param cardFace/cardSuit 回退牌点数和花色。
    // @param fromX/fromY 回退起点（顶部牌位置）。
    // @param toX/toY 回退终点（stack位置）。
    virtual void playUndoDrawAnimation(int32_t cardFace,
                                       int32_t cardSuit,
                                       float fromX,
                                       float fromY,
                                       float toX,
                                       float toY) = 0;

    // @param remainingCount 手牌堆剩余数量。
    virtual void updateStackDisplay(int32_t remainingCount) = 0;

    // @param gameOver 是否处于结束状态（用于禁用输入）。
    virtual void setGameOverState(bool gameOver) { (void)gameOver; }
};

#endif // __STACK_VIEW_H__
