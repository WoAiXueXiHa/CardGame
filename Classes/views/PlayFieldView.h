#ifndef __PLAY_FIELD_VIEW_H__
#define __PLAY_FIELD_VIEW_H__

#include <cstdint>
#include <functional>

class GameModel;
class PlayFieldController;

// 主牌区视图接口。
class PlayFieldView
{
public:
    PlayFieldView();
    virtual ~PlayFieldView();

    // @param gameModel 只读模型。
    // @param controller 逻辑控制器。
    virtual void bind(const GameModel* gameModel, const PlayFieldController* controller) = 0;

    // 刷新主牌区可见卡牌。
    virtual void refresh() = 0;

    // @param callback 点牌回调，参数为cardId。
    virtual void setOnCardClick(const std::function<void(int32_t)>& callback) = 0;

    // @param cardId 被匹配的主牌区卡牌ID。
    // @param targetX/targetY 动画终点（顶部牌坐标）。
    virtual void playMatchAnimation(int32_t cardId, float targetX, float targetY) = 0;

    // @param sourceCardId 回退卡牌ID。
    // @param sourceX/sourceY 回退终点（原始位置）。
    // @param targetCardId 被替换目标牌ID（用于语义标记）。
    // @param targetX/targetY 回退起点（顶部牌位置）。
    virtual void playUndoAnimation(int32_t sourceCardId,
                                   float sourceX,
                                   float sourceY,
                                   int32_t targetCardId,
                                   float targetX,
                                   float targetY) = 0;
};

#endif // __PLAY_FIELD_VIEW_H__
