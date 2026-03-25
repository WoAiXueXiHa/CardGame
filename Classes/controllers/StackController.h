#ifndef __STACK_CONTROLLER_H__
#define __STACK_CONTROLLER_H__

#include <cstdint>
#include "managers/UndoManager.h"
#include "models/GameModel.h"

class StackView;

// 手牌堆逻辑：抽牌、替换顶部牌、抽牌回退。
class StackController
{
public:
    StackController();
    ~StackController();

    // @param gameModel 运行时数据模型。
    // @param undoManager 撤销栈管理器。
    // @param view 手牌堆视图。
    bool init(GameModel* gameModel, UndoManager* undoManager, StackView* view);

    // 执行一次抽牌（stack -> trayTop）。
    bool drawCard();

    // @param record 一条抽牌操作撤销记录。
    bool undoDrawCard(const UndoRecord& record);

    bool hasRemainingCards() const;
    int32_t getRemainingCardCount() const;
    void clear();

private:
    // @param newTrayCard 抽到并成为新顶部牌的卡。
    // @param oldTrayCard 被替换掉的旧顶部牌。
    void _recordUndo(const CardData& newTrayCard, const CardData& oldTrayCard);

private:
    GameModel* _gameModel;
    UndoManager* _undoManager;
    StackView* _view;
};

#endif // __STACK_CONTROLLER_H__
