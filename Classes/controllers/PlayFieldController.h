#ifndef __PLAY_FIELD_CONTROLLER_H__
#define __PLAY_FIELD_CONTROLLER_H__

#include <cstdint>
#include "models/GameModel.h"
#include "managers/UndoManager.h"

class PlayFieldView;

// 主牌区逻辑：点牌匹配、替换顶部牌、回退。
class PlayFieldController
{
public:
    PlayFieldController();
    ~PlayFieldController();

    // @param gameModel 运行时数据模型。
    // @param undoManager 撤销栈管理器。
    // @param view 主牌区视图。
    bool init(GameModel* gameModel, UndoManager* undoManager, PlayFieldView* view);

    // @param cardId 被点击的主牌区卡牌ID。
    bool handleCardClick(int32_t cardId);

    // 执行一次主牌区操作回退。
    bool executeUndo();

    // 按当前遮挡关系刷新主牌区卡牌翻开状态。
    void refreshFlippedState();

    bool canUndo() const;
    void clear();

private:
    // @param cardFace1 待匹配牌点数。
    // @param cardFace2 顶部牌点数。
    bool _isValidMatch(int32_t cardFace1, int32_t cardFace2) const;

    // @param sourceCard 来源牌（主牌区点击牌）。
    // @param targetCard 目标牌（旧顶部牌）。
    // @param sourceX/sourceY 来源牌原始位置。
    // @param targetX/targetY 目标牌位置。
    void _recordUndo(const CardData& sourceCard,
                     const CardData& targetCard,
                     float sourceX,
                     float sourceY,
                     float targetX,
                     float targetY);

    // @param playFieldCard 待替换到顶部牌的主牌区卡牌。
    bool _replaceWithPlayFieldCard(CardData* playFieldCard);

private:
    GameModel* _gameModel;
    UndoManager* _undoManager;
    PlayFieldView* _view;
};

#endif // __PLAY_FIELD_CONTROLLER_H__
