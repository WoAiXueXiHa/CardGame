#include "UndoManager.h"

UndoManager::UndoManager(int32_t maxStackSize)
    : _maxStackSize(maxStackSize)
{
}

UndoManager::~UndoManager()
{
    clear();
}

void UndoManager::pushRecord(const UndoRecord& record)
{
    _undoStack.push_back(record);
    if (_undoStack.size() > static_cast<size_t>(_maxStackSize)) {
        _undoStack.erase(_undoStack.begin());
    }
}

UndoRecord UndoManager::popRecord()
{
    if (_undoStack.empty()) {
        return UndoRecord();
    }

    UndoRecord record = _undoStack.back();
    _undoStack.pop_back();
    return record;
}

const UndoRecord* UndoManager::peekRecord() const
{
    if (_undoStack.empty()) {
        return nullptr;
    }
    return &_undoStack.back();
}

int32_t UndoManager::getStackSize() const
{
    return static_cast<int32_t>(_undoStack.size());
}

bool UndoManager::canUndo() const
{
    return !_undoStack.empty();
}

void UndoManager::clear()
{
    _undoStack.clear();
}

int32_t UndoManager::getMaxStackSize() const
{
    return _maxStackSize;
}

void UndoManager::setMaxStackSize(int32_t maxSize)
{
    _maxStackSize = maxSize;
    if (_undoStack.size() > static_cast<size_t>(_maxStackSize)) {
        const size_t removeCount = _undoStack.size() - static_cast<size_t>(_maxStackSize);
        _undoStack.erase(_undoStack.begin(), _undoStack.begin() + removeCount);
    }
}
