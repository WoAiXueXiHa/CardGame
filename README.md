# Demo — 卡牌消除游戏

基于 **Cocos2d-x** 构建的单人卡牌消除游戏 Demo，采用严格的 **MVC 分层架构**，核心亮点是完整的多步**卡牌回退（Undo）机制**。

---

## 目录

1. [游戏玩法](#游戏玩法)
2. [项目结构](#项目结构)
3. [架构概览](#架构概览)
4. [卡牌回退机制（重点）](#卡牌回退机制重点)
5. [关卡配置](#关卡配置)
6. [游戏结束判定](#游戏结束判定)

---

## 游戏玩法

- 桌面上有一组**主牌区**卡牌和一张当前**顶部牌**（Tray Top）。
- 点击主牌区中点数与顶部牌**相差 1**（如 5↔4、5↔6）的牌即可匹配消除，该牌成为新的顶部牌。
- 若场上无合法匹配，可点击**手牌堆（Stack）**抽一张牌替换顶部牌。
- 任何时候均可点击 **Undo** 按钮逐步回退操作（最多 100 步）。
- 主牌区卡牌全部消除即**胜利**；无合法匹配且手牌堆已空即**失败**。

---

## 项目结构

```
Demo/
├── level_1.json                        # 关卡布局配置文件
├── main.cpp                            # 程序入口
└── Classes/
    ├── AppDelegate.cpp/h               # Cocos2d-x 应用生命周期
    ├── configs/
    │   ├── LevelConfig.h/cpp           # 关卡静态配置数据结构
    │   └── LevelConfigLoader.h/cpp     # JSON 关卡文件解析器
    ├── models/
    │   └── GameModel.h/cpp             # 游戏运行时数据模型（M 层）
    ├── services/
    │   └── GameModelFromLevelGenerator.h/cpp  # 关卡配置→运行时模型转换服务
    ├── managers/
    │   └── UndoManager.h/cpp           # 撤销栈管理器（核心）
    ├── controllers/
    │   ├── GameController.h/cpp        # 主控制器，事件分发与 Undo 路由
    │   ├── PlayFieldController.h/cpp   # 主牌区业务控制器
    │   └── StackController.h/cpp       # 手牌堆业务控制器
    └── views/
        ├── GameView.h/cpp              # 主视图事件总线（抽象接口）
        ├── PlayFieldView.h/cpp         # 主牌区视图接口
        ├── StackView.h/cpp             # 手牌堆视图接口
        ├── GameScene.h/cpp             # Cocos 场景，负责 MVC 装配
        ├── CocosGameView.h/cpp         # GameView 的 Cocos 实现
        ├── CocosPlayFieldView.h/cpp    # PlayFieldView 的 Cocos 实现
        └── CocosStackView.h/cpp        # StackView 的 Cocos 实现
```

---

## 架构概览

项目严格遵循 **MVC 模式**，各层职责清晰，依赖方向单向向下（Controller → Model，Controller → View 接口）。

```
┌─────────────────────────────────────────────────────────┐
│                        View 层                          │
│  GameView (事件总线)  PlayFieldView  StackView           │
│  CocosGameView        CocosPlayFieldView  CocosStackView │
└──────────────────────────┬──────────────────────────────┘
                           │ 事件回调 (std::function)
┌──────────────────────────▼──────────────────────────────┐
│                      Controller 层                      │
│   GameController  ←── UndoManager (撤销栈)              │
│       ├── PlayFieldController                           │
│       └── StackController                              │
└──────────────────────────┬──────────────────────────────┘
                           │ 读写
┌──────────────────────────▼──────────────────────────────┐
│                       Model 层                          │
│   GameModel                                             │
│   ├── playFieldCards[]  (主牌区卡牌集合)                 │
│   ├── trayTopCard       (当前顶部牌)                     │
│   └── stackCards[]      (手牌堆，尾部为栈顶)             │
└─────────────────────────────────────────────────────────┘
```

**数据流向（以点牌为例）：**

```
玩家触摸
  → CocosGameView::emitPlayFieldCardClick(cardId)
  → GameController::_onPlayFieldCardClick(cardId)
  → PlayFieldController::handleCardClick(cardId)
      ├── 校验合法性
      ├── UndoManager::pushRecord()      ← 压栈快照
      ├── GameModel 更新状态
      └── PlayFieldView::playMatchAnimation()  ← 驱动动画
  → GameController::_refreshGameOverState()
```

---

## 卡牌回退机制（重点）

### 设计目标

支持**最多 100 步**的连续撤销，两类操作（主牌区匹配、手牌堆抽牌）均可回退，且回退时伴随反向动画。整个机制分为四个层次：**数据快照结构**、**撤销栈管理**、**操作录制**、**回退执行**。

---

### 1. 数据快照结构 — `UndoRecord`

**文件：** `Classes/managers/UndoManager.h`

每次合法操作执行前，将操作所涉及的所有关键数据以**值拷贝**方式打包成一个 `UndoRecord`：

```cpp
enum class UndoOperationType : int32_t
{
    kPlayFieldMatch = 0,  // 主牌区点牌匹配消除
    kStackDraw      = 1,  // 手牌堆抽牌
};

struct UndoRecord
{
    UndoOperationType operationType; // 操作类型，决定回退路由
    CardData sourceCard;  // 来源牌完整数据快照（点击牌 / 抽出的手牌）
    CardData targetCard;  // 目标牌完整数据快照（旧顶部牌）
    float sourceX, sourceY; // 来源牌操作前坐标（用于反向动画定位）
    float targetX, targetY; // 目标牌坐标（用于反向动画定位）
};
```

**关键设计决策：**
- 使用**值语义**（`CardData` 是 POD 结构体），避免操作执行后出现悬空指针。
- 同时保存**来源牌和目标牌**的快照，因为两者在操作后均会发生变化。
- 单独保存**坐标字段**（而非依赖 `CardData.posX/Y`），因操作执行后坐标会被修改，快照中的坐标专用于反向动画路径还原。

---

### 2. 撤销栈管理 — `UndoManager`

**文件：** `Classes/managers/UndoManager.h/cpp`

`UndoManager` 是纯数据管理器，内部维护一个 `std::vector<UndoRecord>`，尾部为栈顶：

```
栈底 [最旧操作] ──────────────────── [最新操作] 栈顶
      record_0   record_1  ...  record_N-1
```

| 方法 | 说明 |
|---|---|
| `pushRecord(record)` | 压入新记录；若已满（默认 100），从**栈底**淘汰最旧记录（FIFO） |
| `popRecord()` | 弹出并返回栈顶记录 |
| `peekRecord()` | 只读查看栈顶，不弹出（用于路由判断） |
| `canUndo()` | 判断栈是否非空 |
| `clear()` | 新一局开始时清空全部历史 |

**FIFO 淘汰策略（栈满时）：**

```cpp
void UndoManager::pushRecord(const UndoRecord& record)
{
    _undoStack.push_back(record);
    if (_undoStack.size() > static_cast<size_t>(_maxStackSize)) {
        _undoStack.erase(_undoStack.begin()); // 丢弃最旧记录
    }
}
```

`UndoManager` 作为 `GameController` 的**组合成员**（非单例），生命周期与一局游戏绑定，每次 `startGame()` 时调用 `clear()` 重置。

---

### 3. 操作录制 — 压栈时机

两类操作各自在**修改 Model 之前**录制 `UndoRecord` 并压栈。

#### 3a. 主牌区匹配操作的录制

**文件：** `Classes/controllers/PlayFieldController.cpp`

```
玩家点击主牌区卡牌
  → handleCardClick(cardId)
      ├── 校验：卡牌存在 / 未消除 / 牌面朝上 / 与顶部牌点数相差 1
      └── _replaceWithPlayFieldCard(playFieldCard)
              ├── [1] _recordUndo(...)        ← 先快照，再改数据
              │       operationType = kPlayFieldMatch
              │       sourceCard   = 被点击的主牌区卡牌（完整快照）
              │       targetCard   = 当前顶部牌（完整快照）
              │       sourceX/Y    = 主牌区卡牌原始坐标
              │       targetX/Y    = 顶部牌坐标
              ├── [2] playFieldCard->isMatched = true
              ├── [3] GameModel::setTrayTopCard(新顶部牌)
              ├── [4] refreshFlippedState()   ← 更新遮挡状态
              └── [5] PlayFieldView::playMatchAnimation()
```

#### 3b. 手牌堆抽牌操作的录制

**文件：** `Classes/controllers/StackController.cpp`

```
玩家点击抽牌
  → StackController::drawCard()
      ├── [1] 记录 oldTrayCard（当前顶部牌快照）
      ├── [2] GameModel::popStackCard() 取出新牌
      ├── [3] _recordUndo(newStackCard, oldTrayCard)
      │       operationType = kStackDraw
      │       sourceCard   = 从手牌堆抽出的新牌
      │       targetCard   = 被替换掉的旧顶部牌
      │       sourceX/Y    = 新牌在手牌堆中的坐标
      │       targetX/Y    = 顶部牌坐标
      ├── [4] GameModel::setTrayTopCard(newTrayCard)
      └── [5] StackView::playDrawAnimation()
```

---

### 4. 回退执行 — Undo 路由与还原

**文件：** `Classes/controllers/GameController.cpp`

玩家点击 Undo 时，`GameController::handleUndoClick()` 先 **peek（不弹出）** 栈顶记录，根据 `operationType` 路由到不同子控制器：

```cpp
bool GameController::handleUndoClick()
{
    const UndoRecord* topRecord = _undoManager.peekRecord();
    if (!topRecord) return false;

    if (topRecord->operationType == UndoOperationType::kPlayFieldMatch) {
        return _playFieldController.executeUndo();    // 由子控制器内部弹出
    }
    if (topRecord->operationType == UndoOperationType::kStackDraw) {
        const UndoRecord record = _undoManager.popRecord(); // 此处弹出
        return _stackController.undoDrawCard(record);
    }
    return false;
}
```

> **设计说明：** `kPlayFieldMatch` 路由时由 `PlayFieldController::executeUndo()` 内部负责弹栈，职责内聚；`kStackDraw` 路由时由 `GameController` 先弹出记录再传入子控制器，避免子控制器持有对 `UndoManager` 的直接弹出依赖。

#### 4a. 主牌区匹配的回退

**文件：** `Classes/controllers/PlayFieldController.cpp` — `executeUndo()`

```
PlayFieldController::executeUndo()
  ├── [1] UndoManager::popRecord()               ← 弹出栈顶记录
  ├── [2] 在主牌区中查找 sourceCard.cardId
  │       ├── 找到 → 重置 isMatched=false，恢复原始坐标
  │       └── 未找到 → 重新 addPlayFieldCard(restored)  ← 防御性还原
  ├── [3] GameModel::setTrayTopCard(record.targetCard)  ← 还原旧顶部牌
  ├── [4] refreshFlippedState()                 ← 重新计算遮挡关系
  └── [5] PlayFieldView::playUndoAnimation(
              sourceCardId, sourceX, sourceY,    ← 卡牌飞回原始位置
              targetCardId, targetX, targetY)
```

**还原逻辑要点：**
- 主牌区卡牌的 `isMatched` 被重置为 `false`，使其重新参与游戏逻辑。
- 坐标恢复使用 `record.sourceX/Y`（操作前快照），而非 Model 当前值。
- 还原后立即调用 `refreshFlippedState()`，重新计算所有卡牌的遮挡关系，确保 Flip 状态与实际布局一致。

#### 4b. 手牌堆抽牌的回退

**文件：** `Classes/controllers/StackController.cpp` — `undoDrawCard()`

```
StackController::undoDrawCard(record)
  ├── [1] GameModel::setTrayTopCard(record.targetCard)    ← 还原旧顶部牌
  ├── [2] 将 record.sourceCard 恢复坐标后 addStackCard()  ← 归还手牌堆
  └── [3] StackView::playUndoDrawAnimation(
              cardFace, cardSuit,
              targetX, targetY,    ← 动画起点：顶部牌位置
              sourceX, sourceY)    ← 动画终点：手牌堆位置
      + updateStackDisplay()       ← 刷新剩余牌数 UI
```

**还原逻辑要点：**
- 抽出的牌通过 `addStackCard()` 压回手牌堆尾部（即栈顶），下次抽牌仍会先取到它。
- 旧顶部牌直接用快照 `record.targetCard` 覆盖，无需额外坐标修正。

---

### 5. 完整 Undo 时序图

```
玩家点击 Undo 按钮
        │
        ▼
GameController::handleUndoClick()
        │
        ├─ peekRecord() → kPlayFieldMatch?
        │       │
        │       ▼
        │  PlayFieldController::executeUndo()
        │       ├── popRecord()
        │       ├── GameModel: 恢复主牌区卡牌状态 + 恢复顶部牌
        │       ├── refreshFlippedState()
        │       └── PlayFieldView: 播放卡牌飞回动画
        │
        └─ peekRecord() → kStackDraw?
                │
                ▼
           popRecord()（GameController 层弹出）
           StackController::undoDrawCard(record)
                ├── GameModel: 恢复顶部牌 + 归还手牌堆
                └── StackView: 播放卡牌飞回动画 + 刷新数量
        │
        ▼
GameController::_refreshGameOverState()  ← 重新判断胜负
```

---

### 6. 遮挡判定与 Undo 的联动

主牌区卡牌存在**层叠遮挡**关系——被上方卡牌覆盖的牌 `isFlipped=false`，不可点击。每次回退还原主牌区后，必须重新计算遮挡关系，否则被还原的牌可能错误地保持不可点击状态。

遮挡判定规则（`PlayFieldController::refreshFlippedState()`）：

```
对主牌区中每张未消除的牌 card：
    遍历所有其他未消除的牌 other：
        if (other.posY < card.posY)                    // other 在 card 上方
        && (|other.posX - card.posX| <= 120)           // X 方向重叠
        && (card.posY  - other.posY  <= 260)           // Y 方向足够近
        then card.isFlipped = false  （被遮挡，不可点击）

    若无任何牌遮挡 → card.isFlipped = true  （可点击）
```

Undo 执行后调用此方法，保证 Model 状态与视觉布局完全同步。

---

## 关卡配置

关卡以 JSON 文件描述（如 `level_1.json`），由 `LevelConfigLoader` 解析为 `LevelConfig`，再由 `GameModelFromLevelGenerator` 转换为运行时 `GameModel`，为每张牌分配全局唯一的 `cardId`。

```json
{
    "Playfield": [
        { "CardFace": 12, "CardSuit": 0, "Position": {"x": 250, "y": 1000} },
        { "CardFace": 2,  "CardSuit": 0, "Position": {"x": 300, "y": 800}  }
    ],
    "Stack": [
        { "CardFace": 2, "CardSuit": 0, "Position": {"x": 0, "y": 0} }
    ]
}
```

| 字段 | 说明 |
|---|---|
| `Playfield` | 主牌区初始布局，坐标决定卡牌层叠遮挡关系 |
| `Stack` | 手牌堆，`Position` 为 `(0,0)` 时由代码赋予默认坐标 `(860, 240)` |
| `tray`（隐含）| `GameModelFromLevelGenerator` 将 Stack 第一张牌设为初始顶部牌 |

`CardFace` 取值：`0=A, 1=2, 2=3, ..., 12=K`
`CardSuit` 取值：`0=梅花, 1=方块, 2=红心, 3=黑桃`

---

## 游戏结束判定

每次操作（匹配、抽牌、Undo）执行后，`GameController::_refreshGameOverState()` 自动触发：

```
① 主牌区所有牌均已消除（isMatched=true）
        → 胜利（gameOver=true）

② 主牌区存在未消除牌，但：
        - 无任何翻开的牌与顶部牌点数相差 1（无合法匹配）
        AND
        - 手牌堆已空（无法抽牌）
        → 失败（gameOver=true）

③ 以上均不满足 → 游戏继续
```

结束状态通过 `GameView::setGameOverState(bool)` 通知 View 层显示结束面板，并屏蔽后续输入。Undo 操作同样会触发此判定——若回退后局面重新可解，结束面板会自动消失。
 

# debug测试程序
在`../cocosCode/DemoRun/cmake-build/bin/cpp-empty-test/Debug/cpp-empty-test.exe`这个路径下有配置好的debug测试版本demo
