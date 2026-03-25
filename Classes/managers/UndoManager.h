#ifndef __UNDO_MANAGER_H__
#define __UNDO_MANAGER_H__

#include <vector>
#include <cstdint>
#include "models/GameModel.h"

// 一条撤销记录：完整保存一次操作前后的关键数据。
struct UndoRecord
{
    int32_t operationType; // 0=主牌区匹配, 1=手牌堆抽牌
    CardData sourceCard;   // 操作来源牌
    CardData targetCard;   // 被替换目标牌（旧顶部牌）
    float sourceX;         // 来源牌原始X
    float sourceY;         // 来源牌原始Y
    float targetX;         // 目标位置X
    float targetY;         // 目标位置Y

    UndoRecord()
        : operationType(-1),
          sourceX(0.0f),
          sourceY(0.0f),
          targetX(0.0f),
          targetY(0.0f)
    {
    }
};

// 撤销栈管理器。
class UndoManager
{
public:
    explicit UndoManager(int32_t maxStackSize = 100);
    ~UndoManager();

    // @param record 要压栈的撤销记录。
    void pushRecord(const UndoRecord& record);

    // 弹出并返回栈顶记录；空栈时返回默认记录。
    UndoRecord popRecord();

    // 查看栈顶记录（不弹出）。
    const UndoRecord* peekRecord() const;

    int32_t getStackSize() const;
    bool canUndo() const;
    void clear();

    int32_t getMaxStackSize() const;

    // @param maxSize 撤销栈最大容量。
    void setMaxStackSize(int32_t maxSize);

private:
    std::vector<UndoRecord> _undoStack;
    int32_t _maxStackSize;
};

#endif // __UNDO_MANAGER_H__
