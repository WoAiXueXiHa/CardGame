# CardGame - C++纸牌匹配消除游戏

## 📖 项目概述

本项目是一款基于 Cocos2d-x (v3.17) 引擎开发的单机纸牌匹配消除游戏。游戏核心玩法要求玩家通过点数相邻（差值为1，无花色限制）的规则，将主牌区的卡牌与底牌区的顶部牌进行匹配消除。当主牌区无可用匹配时，玩家可从手牌堆抽取新牌。项目实现了完整的连续撤销（Undo）机制。

本项目在工程设计上严格遵守面向对象设计原则，采用高度解耦的 **MVC + 局部服务管理器** 架构，具备极强的可维护性与功能扩展性。

------

## 🏗️ 核心架构设计 (Architecture)

为了避免传统游戏开发中“上帝类”（God Class）的产生，本项目将逻辑、数据与渲染严格分离：

* **Models (运行时动态数据层):** * 以 `GameModel` 和 `CardData` 为核心，纯粹的 Plain Old Data (POD) 风格容器。
  * **亮点：** 隔绝了一切 Cocos2d-x 渲染依赖，支持轻量级的全量 JSON 序列化与反序列化，为后续扩展“游戏存档/读档”功能奠定了底层基础。
* **Views (视图表现层):** * 包含 `CocosGameView`、`CocosPlayFieldView` 等。
  * **亮点：** 绝对的被动渲染器（Passive View）。View 层不持有任何业务逻辑，仅持有 Model 的 `const` 指针进行状态读取。所有的用户触控输入均通过 `std::function` 回调总线向外抛出，实现了与控制逻辑的完全解耦。
* **Controllers (逻辑控制层):** * `GameController` 担任中枢调度，管理生命周期与跨模块通信。
  * `PlayFieldController` 和 `StackController` 处理具体的局部业务规则（如遮挡关系计算、合法性校验等）。
* **Managers & Services (独立服务与局部管理):** * `UndoManager`：作为 Controller 的局部组合成员（严禁使用全局单例），负责维护历史操作栈。
  * `GameModelFromLevelGenerator`：纯无状态服务，负责将静态的 `LevelConfig` JSON 数据清洗并转化为运行时的 `GameModel`。

------

## 🔄 核心亮点剖析：精细化回退 (Undo) 机制

游戏状态的追溯与回滚是本项目的核心技术难点。本项目摒弃了低效的“全局快照备份”，采用基于增量记录（Delta Record）的轻量级回退方案。

### 1. 核心定义（本质）

回退机制的本质是**基于命令模式（Command Pattern）思想的“反向操作补偿”**。

通过一个轻量级的结构体 `UndoRecord`，我们在每一次原子操作（如：消除、抽牌）发生前，精准捕获该操作的“差分快照”：

* **动作类型 (`operationType`)**：标识这是一次主牌区匹配，还是一次手牌堆抽牌。
* **操作实体 (`sourceCard`, `targetCard`)**：完整拷贝操作前相关卡牌的数据状态。
* **空间坐标 (`sourceX/Y`, `targetX/Y`)**：记录卡牌在 UI 层面的起点与终点，用于反向动画的精准定位。

### 2. 在实际工程中怎么用，有哪些坑点

**执行流：**

* **录制：** 当玩家点击一张合法的 `♦️3`（源牌），在它替换底牌区的 `♣4`（目标牌）之前，Controller 会立即生成一份 `UndoRecord` 压入 `UndoManager` 的固定容量栈中，随后再修改 Model 并触发 View 的 MoveTo 动画。
* **回退：** 玩家点击 Undo，`GameController` 弹出栈顶记录，通过 `operationType` 路由给对应的子 Controller。子 Controller 强制覆写 Model 还原为旧状态，并指示 View 播放起点到终点的反向 MoveTo 动画。

**⚠️ 工程级坑点与解决方案：**

1. **对象生命周期与悬空指针（Dangling Pointer）：** * *坑点：* 若 `UndoRecord` 保存的是卡牌对象的 C++ 指针，一旦卡牌在匹配后被销毁或内存复用，回退时解引用就会导致 Crash。
   * *解法：* 记录采用**值语义拷贝**（`CardData` 结构体）并强依赖唯一业务主键 `cardId` 进行寻址（`getPlayFieldCard(cardId)`）。即使原实体被移除，回退时也能基于 `cardId` 重新构建并塞回 Model，体现了严谨的内存安全思维。
2. **数据状态与 UI 表现的异步撕裂：** * *坑点：* Model 数据的还原是 $O(1)$ 瞬间完成的，但 View 层的 MoveTo 动画需要 0.2 秒。如果在卡牌退回原位的这 0.2 秒内，玩家再次点击这张处于“飞行状态”的牌，就会造成判定错乱。
   * *演进方案：* 当前 UI 层通过简易的状态标志位进行控制。在更复杂的大型工程中，此处可引入全局的 `UIEventLock`（输入锁）或状态机（FSM），在动画播放期间挂起底层 Touch 事件的派发。

### 3. 价值意义

这一机制的设计具有极高的工程价值。它实现了真正的**高内聚、低耦合**：

* **空间复杂度极优：** 相比于每一步保存整个桌面的状态（包含数十张牌的坐标、翻面状态），只保存操作相关的 2 张牌的数据，内存开销降低了一个数量级。
* **职责边界清晰：** 业务逻辑（怎么打牌）和 历史回溯逻辑（怎么撤销）互不干涉。未来即使加入再多花里胡哨的特效或新规则，Undo 机制底层代码也无需改动一行，保证了底盘的绝对稳定。

------

## 🛠️ 数据流转与通信时序 (Data Flow)

以一次典型的“点击卡牌消除”为例，系统的数据流转如下：

1. **用户输入**：玩家点击屏幕上的 `♦️3`。
2. **视图抛出**：`CocosPlayFieldView` 触控测试命中，调用已绑定的 `_onCardClick(cardId)` 回调。
3. **总线中转**：`GameView` 将事件透传至 `GameController`。
4. **逻辑处理**：`PlayFieldController` 接收指令，校验业务规则（判断是否被遮挡、点数是否与底牌差1）。
5. **数据变更**：校验通过。录制 `UndoRecord`，随后更新 `GameModel` 中的卡牌归属与坐标。
6. **视图更新**：Controller 调用 `PlayFieldView::playMatchAnimation`，指令 UI 模块驱动卡牌执行位移动画，最终刷新其余卡牌的翻面（覆盖）状态。

------

## 🧩 扩展性指南 (Extension Guide)

得益于 MVC 架构，扩展新功能变得非常机械和安全。

* **新增特殊牌型（如：炸弹牌、万能牌）：**

  无需修改底层的 Model 与 View 结构。只需在 `configs` 增加对应的 JSON 定义，并在 `PlayFieldController::_isValidMatch()` 函数中新增一行逻辑校验分支即可。

* **新增回退类型（如：撤销一次洗牌技能）：**

  扩展 `UndoRecord` 的 `operationType` 枚举，在 `GameController::handleUndoClick` 中新增 `switch/case` 路由，并实现对应的还原算法即可。

