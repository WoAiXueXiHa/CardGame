# CardGame - C++纸牌匹配消除游戏

## 📖 项目概述

本项目是一款基于 Cocos2d-x (v3.17) 引擎开发的单机纸牌匹配消除游戏。游戏核心玩法要求玩家通过点数相邻（差值为1，无花色限制）的规则，将主牌区的卡牌与底牌区的顶部牌进行匹配消除。当主牌区无可用匹配时，玩家可从手牌堆抽取新牌。项目实现了完整的连续撤销（Undo）机制。

本项目在工程设计上严格遵守面向对象设计原则，采用高度解耦的 **MVC + 局部服务管理器** 架构，具备极强的可维护性与功能扩展性。

---

## 🏗️ 核心架构设计 (Architecture)

本项目将逻辑、数据与渲染严格分离，采用分层设计模式。整体架构遵循 MVC 模式，并在此基础上引入局部服务管理器，避免全局单例的弊端。

### 1. Models 层 - 运行时动态数据

**核心类：** `GameModel`、`CardData`、`PlayFieldCard`、`StackCard`

**职责：**
- 以纯 POD（Plain Old Data）风格容器存储游戏状态
- 维护主牌区、底牌区、手牌堆的卡牌集合
- 记录卡牌的点数、ID、位置、翻面状态等属性
- 提供数据查询接口（不涉及业务逻辑）

**设计亮点：**
- **零渲染依赖：** 完全隔绝 Cocos2d-x 引擎依赖，Model 层可独立单元测试，便于逻辑验证
- **JSON 序列化能力：** 支持全量序列化与反序列化，为存档/读档功能预留接口
- **值语义设计：** 采用值拷贝而非指针引用，避免悬空指针问题，为回退机制提供安全基础

**数据结构示例：**
```cpp
struct CardData {
    int cardId;           // 唯一标识
    int point;            // 点数 (1-13)
    int suit;             // 花色
    bool isFaceUp;        // 翻面状态
};

struct GameModel {
    std::vector<PlayFieldCard> playField;    // 主牌区
    std::vector<StackCard> stack;            // 底牌区
    std::vector<CardData> handPile;          // 手牌堆
    int score;                               // 游戏分数
};
```

---

### 2. Views 层 - 视图表现层

**核心类：** `CocosGameView`、`CocosPlayFieldView`、`CocosStackView`、`CocosHandPileView`

**职责：**
- 将 Model 数据渲染为可视化的 UI 元素（精灵、标签、按钮）
- 处理用户触控输入并通过回调向外抛出
- 执行卡牌动画（位移、翻转、消除特效）
- 维护 UI 层的临时状态（如动画播放中标志）

**设计亮点：**
- **被动渲染器（Passive View）：** View 层仅持有 Model 的 `const` 指针，不修改任何业务状态，确保数据流向单向
- **事件总线模式：** 所有用户交互（点击、长按）通过 `std::function` 回调绑定，实现与 Controller 的完全解耦
- **动画驱动分离：** 动画播放与数据更新异步进行，通过状态标志位防止动画期间的重复操作

**关键接口：**
```cpp
class CocosPlayFieldView {
    // 绑定点击回调
    void setOnCardClickCallback(std::function<void(int cardId)> callback);
    
    // 播放卡牌消除动画
    void playMatchAnimation(int sourceCardId, int targetCardId, 
                           const Vec2& fromPos, const Vec2& toPos);
    
    // 刷新卡牌翻面状态（遮挡关系）
    void updateCardCoverState(int cardId, bool isCovered);
};
```

---

### 3. Controllers 层 - 逻辑控制中枢

**核心类：** `GameController`、`PlayFieldController`、`StackController`

**职责分工：**

| 控制器 | 职责 | 关键方法 |
|--------|------|---------|
| `GameController` | 全局生命周期管理、模块间协调、Undo 栈管理 | `init()`、`handleUndoClick()`、`update()` |
| `PlayFieldController` | 主牌区业务规则、匹配校验、遮挡关系计算 | `isValidMatch()`、`executeMatch()`、`updateCoverState()` |
| `StackController` | 底牌区管理、抽牌逻辑、底牌顶部维护 | `drawCard()`、`getTopCard()`、`isEmpty()` |

**设计思路：**
- **分层职责：** 全局逻辑由 `GameController` 统筹，具体业务由子 Controller 处理，避免单点过载
- **局部服务注入：** 每个 Controller 持有所需的 Manager（如 `UndoManager`），而非全局单例，提升可测试性和模块独立性
- **事件驱动流：** Controller 监听 View 的回调事件，触发 Model 更新，再指示 View 刷新

**典型调用链：**
```
用户点击 → View 回调 → GameController → PlayFieldController 
→ 校验规则 → 录制 Undo → 更新 Model → View 播放动画
```

---

### 4. Managers & Services 层 - 独立服务

**核心类：** `UndoManager`、`GameModelFromLevelGenerator`

#### 4.1 UndoManager - 历史操作管理

**职责：**
- 维护固定容量的操作栈（默认 50 步）
- 提供 `push()` 和 `pop()` 接口
- 支持栈溢出时的自动淘汰策略（FIFO）

**设计特点：**
- **值语义存储：** 每条记录完整拷贝卡牌数据，避免指针悬空
- **轻量级记录：** 仅记录操作相关的 2 张牌信息，空间复杂度 O(1)
- **局部组合：** 作为 `GameController` 的成员变量，严禁全局单例

#### 4.2 GameModelFromLevelGenerator - 关卡数据转换

**职责：**
- 将静态的 `LevelConfig` JSON 转化为运行时 `GameModel`
- 执行数据清洗与合法性校验
- 初始化卡牌的位置、翻面状态等

**设计特点：**
- **无状态服务：** 纯函数式设计，多次调用结果一致
- **配置驱动：** 支持 JSON 配置文件定义关卡布局，便于策划快速迭代

---

## 🔄 回退机制详解 (Undo Mechanism)

游戏状态的追溯与回滚是本项目的核心技术难点。本项目摒弃了低效的"全局快照备份"，采用基于增量记录（Delta Record）的轻量级回退方案。

### 1. 核心设计理念

回退机制的本质是**基于命令模式（Command Pattern）思想的"反向操作补偿"**。

与其保存整个游戏状态的快照，我们采用更精细的策略：在每一次原子操作（如：消除、抽牌）发生**前**，精准捕获该操作的"差分快照"，记录在轻量级的 `UndoRecord` 结构体中。

**UndoRecord 的核心字段：**
```cpp
struct UndoRecord {
    OperationType operationType;    // 操作类型：匹配消除 / 抽牌
    CardData sourceCard;            // 源卡牌的完整数据拷贝
    CardData targetCard;            // 目标卡牌的完整数据拷贝
    Vec2 sourcePos;                 // 源卡牌的起始位置
    Vec2 targetPos;                 // 目标卡牌的位置
    int sourceIndex;                // 源卡牌在容器中的索引
    int targetIndex;                // 目标卡牌在容器中的索引
};
```

**为什么采用这种设计：**
- **空间高效：** 每条记录仅包含 2 张牌的数据，而非整个游戏状态（可能包含数十张牌）
- **时间高效：** 回退时无需遍历历史，直接 O(1) 弹出栈顶并还原
- **逻辑清晰：** 每条记录对应一次用户操作，语义明确

---

### 2. 执行流程与工程实现

#### 2.1 录制阶段（Recording Phase）

当玩家点击一张合法的卡牌时：

```
1. View 层触发 onCardClick 回调
   ↓
2. GameController 接收事件
   ↓
3. PlayFieldController 校验匹配规则
   - 检查卡牌是否被遮挡
   - 检查点数是否相邻（差值为1）
   - 检查底牌区是否有可用牌
   ↓
4. 校验通过 → 生成 UndoRecord
   - 拷贝源卡牌数据
   - 拷贝目标卡牌数据
   - 记录当前位置与索引
   ↓
5. UndoManager::push(record)
   - 将记录压入栈
   - 若栈满则自动淘汰最旧的记录
   ↓
6. 更新 GameModel
   - 修改卡牌的归属（从主牌区移到底牌区）
   - 更新卡牌的位置坐标
   ↓
7. 指示 View 播放动画
   - 卡牌从源位置飞向目标位置
   - 其他卡牌的遮挡状态刷新
```

#### 2.2 回退阶段（Undo Phase）

当玩家点击 Undo 按钮时：

```
1. View 层触发 onUndoClick 回调
   ↓
2. GameController::handleUndoClick()
   ↓
3. UndoManager::pop() 获取栈顶记录
   ↓
4. 根据 operationType 路由到对应的 Controller
   ├─ MATCH_OPERATION → PlayFieldController::undoMatch()
   └─ DRAW_OPERATION → StackController::undoDraw()
   ↓
5. 子 Controller 执行还原
   - 强制覆写 Model 中的卡牌数据
   - 恢复卡牌的位置与索引
   ↓
6. 指示 View 播放反向动画
   - 卡牌从目标位置飞回源位置
   - 其他卡牌的遮挡状态刷新
```

---

### 3. 工程级坑点与解决方案

#### 坑点 1：对象生命周期与悬空指针（Dangling Pointer）

**问题描述：**
若 `UndoRecord` 保存的是卡牌对象的 C++ 指针，一旦卡牌在匹配后被销毁或内存复用，回退时解引用就会导致 Crash。

**解决方案：**
- 记录采用**值语义拷贝**（`CardData` 结构体），而非指针
- 强依赖唯一业务主键 `cardId` 进行寻址（`getPlayFieldCard(cardId)`）
- 即使原实体被移除，回退时也能基于 `cardId` 重新构建并塞回 Model

**代码示例：**
```cpp
// ❌ 错误做法：保存指针
struct BadUndoRecord {
    PlayFieldCard* sourceCard;  // 危险！可能悬空
};

// ✅ 正确做法：保存值拷贝
struct GoodUndoRecord {
    CardData sourceCard;        // 安全的值拷贝
    int cardId;                 // 业务主键
};
```

#### 坑点 2：数据状态与 UI 表现的异步撕裂

**问题描述：**
Model 数据的还原是 O(1) 瞬间完成的，但 View 层的 MoveTo 动画需要 0.2 秒。如果在卡牌退回原位的这 0.2 秒内，玩家再次点击这张处于"飞行状态"的牌，就会造成判定错乱。

**解决方案：**
- 当前 UI 层通过简易的状态标志位进行控制
- 在动画播放期间，禁用底层 Touch 事件的派发
- 在更复杂的大型工程中，可引入全局的 `UIEventLock`（输入锁）或状态机（FSM）

**代码示例：**
```cpp
class CocosPlayFieldView {
private:
    bool _isAnimating = false;  // 动画播放标志
    
public:
    void playMatchAnimation(...) {
        _isAnimating = true;
        // 播放动画...
        scheduleOnce([this](float) {
            _isAnimating = false;
        }, 0.2f, "animationEnd");
    }
    
    void onTouchBegan(...) {
        if (_isAnimating) return;  // 动画期间忽略点击
        // 处理点击...
    }
};
```

#### 坑点 3：栈溢出与历史丢失

**问题描述：**
固定容量的栈（如 50 步）满了之后，新的操作会覆盖最旧的记录，导致无法回退到很久之前的状态。

**解决方案：**
- 采用 FIFO（先进先出）策略，自动淘汰最旧的记录
- 可配置栈容量，根据内存预算调整
- 未来可考虑引入"检查点"机制，定期保存完整快照

**代码示例：**
```cpp
class UndoManager {
private:
    static const int MAX_RECORDS = 50;
    std::deque<UndoRecord> _records;
    
public:
    void push(const UndoRecord& record) {
        if (_records.size() >= MAX_RECORDS) {
            _records.pop_front();  // 淘汰最旧的
        }
        _records.push_back(record);
    }
};
```

---

### 4. 回退机制的价值意义

**空间复杂度极优：**
- 全局快照方案：每步保存整个 GameModel（数十张牌的数据）→ O(n) 空间
- 增量记录方案：每步仅保存 2 张牌的数据 → O(1) 空间
- 内存开销降低了一个数量级

**职责边界清晰：**
- 业务逻辑（怎么打牌）和历史回溯逻辑（怎么撤销）互不干涉
- 未来即使加入再多花里胡哨的特效或新规则，Undo 机制底层代码也无需改动一行
- 保证了底盘的绝对稳定性

**易于扩展：**
- 新增操作类型时，仅需扩展 `OperationType` 枚举和对应的还原逻辑
- 无需修改 UndoManager 的核心实现

---

## 🛠️ 数据流转与通信时序 (Data Flow)

以一次典型的"点击卡牌消除"为例，系统的数据流转如下：

```
┌─────────────────────────────────────────────────────────────┐
│ 1. 用户输入：玩家点击屏幕上的 ♦️3                            │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 2. 视图抛出：CocosPlayFieldView 触控测试命中                 │
│    调用已绑定的 _onCardClick(cardId) 回调                    │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 3. 总线中转：GameView 将事件透传至 GameController            │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 4. 逻辑处理：PlayFieldController 接收指令                    │
│    校验业务规则：                                            │
│    - 是否被遮挡？                                            │
│    - 点数是否与底牌差1？                                     │
│    - 底牌区是否有可用牌？                                    │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 5. 数据变更：校验通过                                        │
│    - 录制 UndoRecord 并压入栈                                │
│    - 更新 GameModel 中的卡牌归属与坐标                       │
│    - 更新分数                                                │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 6. 视图更新：Controller 调用 PlayFieldView::playMatchAnimation│
│    - 指令 UI 模块驱动卡牌执行位移动画                        │
│    - 刷新其余卡牌的翻面（遮挡）状态                          │
│    - 更新 UI 显示（分数、剩余牌数等）                        │
└─────────────────────────────────────────────────────────────┘
```

---

## 🧩 扩展性指南 (Extension Guide)

得益于 MVC 架构与清晰的职责划分，扩展新功能变得非常机械和安全。

### 场景 1：新增特殊牌型（如：炸弹牌、万能牌）

**步骤：**
1. 在 `CardData` 中新增 `cardType` 字段（普通牌 / 炸弹牌 / 万能牌）
2. 在 `configs` JSON 中定义新牌型的配置
3. 在 `PlayFieldController::isValidMatch()` 中新增逻辑分支

**代码示例：**
```cpp
bool PlayFieldController::isValidMatch(const CardData& source, const CardData& target) {
    // 原有逻辑
    if (std::abs(source.point - target.point) == 1) {
        return true;
    }
    
    // 新增：炸弹牌可匹配任何牌
    if (source.cardType == CardType::BOMB || target.cardType == CardType::BOMB) {
        return true;
    }
    
    // 新增：万能牌可匹配任何牌
    if (source.cardType == CardType::WILDCARD || target.cardType == CardType::WILDCARD) {
        return true;
    }
    
    return false;
}
```

### 场景 2：新增回退类型（如：撤销一次洗牌技能）

**步骤：**
1. 扩展 `UndoRecord` 的 `operationType` 枚举
2. 在 `GameController::handleUndoClick()` 中新增 `switch/case` 路由
3. 实现对应的还原算法

**代码示例：**
```cpp
enum class OperationType {
    MATCH_OPERATION,
    DRAW_OPERATION,
    SHUFFLE_OPERATION  // 新增
};

void GameController::handleUndoClick() {
    auto record = _undoManager.pop();
    
    switch (record.operationType) {
        case OperationType::MATCH_OPERATION:
            _playFieldController.undoMatch(record);
            break;
        case OperationType::DRAW_OPERATION:
            _stackController.undoDraw(record);
            break;
        case OperationType::SHUFFLE_OPERATION:
            undoShuffle(record);  // 新增处理
            break;
    }
}
```

### 场景 3：新增游戏模式（如：计时模式、关卡模式）

**步骤：**
1. 在 `GameModel` 中新增 `gameMode` 字段
2. 在 `GameController` 中根据模式调整逻辑（如计时器、关卡进度）
3. 在 View 层显示对应的 UI 元素

---

## 📁 项目文件结构

```
CardGame/
├── Classes/
│   ├── Models/
│   │   ├── GameModel.h/cpp          # 游戏状态数据容器
│   │   ├── CardData.h/cpp           # 卡牌数据结构
│   │   └── LevelConfig.h/cpp        # 关卡配置
│   │
│   ├── Views/
│   │   ├── CocosGameView.h/cpp      # 主游戏视图
│   │   ├── CocosPlayFieldView.h/cpp # 主牌区视图
│   │   ├── CocosStackView.h/cpp     # 底牌区视图
│   │   └── CocosHandPileView.h/cpp  # 手牌堆视图
│   │
│   ├── Controllers/
│   │   ├── GameController.h/cpp     # 全局控制器
│   │   ├── PlayFieldController.h/cpp# 主牌区控制器
│   │   └── StackController.h/cpp    # 底牌区控制器
│   │
│   ├── Managers/
│   │   ├── UndoManager.h/cpp        # 回退管理器
│   │   └── GameModelFromLevelGenerator.h/cpp  # 关卡转换服务
│   │
│   └── Scenes/
│       └── GameScene.h/cpp          # 游戏场景入口
│
├── Resources/
│   ├── configs/
│   │   └── levels.json              # 关卡配置文件
│   └── images/
│       └── cards/                   # 卡牌图片资源
│
└── README.md                        # 本文件
```

---

## 🚀 快速开始

### 编译与运行

```bash
# 使用 Cocos2d-x 官方工具链
cocos run -s . -p win32

# 或使用 CMake
mkdir build && cd build
cmake ..
make
./CardGame
```

### 游戏规则

1. **匹配消除：** 点击主牌区的卡牌，若其点数与底牌区顶部牌相邻（差值为1），则消除
2. **抽牌：** 当主牌区无可用匹配时，点击手牌堆抽取新牌
3. **撤销：** 点击 Undo 按钮回退上一步操作
4. **游戏结束：** 当手牌堆为空且无可用匹配时，游戏结束

---

## 📝 设计总结

本项目通过以下设计原则实现了高质量的代码架构：

| 原则 | 实现方式 | 收益 |
|------|--------|------|
| 单一职责 | 分层架构（M/V/C）+ 局部服务 | 代码易维护、易测试 |
| 开闭原则 | 配置驱动 + 枚举扩展 | 新功能无需修改核心代码 |
| 依赖倒置 | 回调总线 + 接口抽象 | 模块间解耦、可替换 |
| 组合优于继承 | Manager 作为成员而非单例 | 避免全局状态污染 |
| 值语义设计 | UndoRecord 采用值拷贝 | 内存安全、无悬空指针 |

这些设计决策确保了项目的长期可维护性和可扩展性。


# debug版本支持
在`../DemoRun/cmake-build/bin/cpp-empty-test/`路径下有测试demo版本
