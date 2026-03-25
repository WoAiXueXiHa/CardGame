#ifndef __GAME_CONTROLLER_H__
#define __GAME_CONTROLLER_H__

#include <string>
#include "configs/LevelConfigLoader.h"
#include "controllers/PlayFieldController.h"
#include "controllers/StackController.h"
#include "managers/UndoManager.h"
#include "models/GameModel.h"
#include "services/GameModelFromLevelGenerator.h"

class GameView;
class PlayFieldView;
class StackView;

// 主控制器：串联加载、输入分发、抽牌、匹配、回退。
class GameController
{
public:
    GameController();
    ~GameController();

    // gameView 主UI事件总线。
    // playFieldView 主牌区视图。
    // stackView 手牌堆视图。
    bool init(GameView* gameView, PlayFieldView* playFieldView, StackView* stackView);

    // 关卡JSON路径（如 level_1.json）
    bool startGame(const std::string& levelFilePath);

    // 重新随机发牌并开始新一局（使用相同关卡文件）。
    bool restartGame();

    // 退出程序。
    void quitGame();

    // 处理一次撤销按钮点击。
    bool handleUndoClick();

    // 清空运行状态与绑定。
    void clear();

private:
    // 绑定视图事件到各控制器。
    void _bindViewEvents();

    // 根据当前模型刷新游戏结束状态。
    void _refreshGameOverState();

private:
    GameModel _gameModel;
    UndoManager _undoManager;
    PlayFieldController _playFieldController;
    StackController _stackController;
    LevelConfigLoader _levelConfigLoader;
    GameModelFromLevelGenerator _gameModelGenerator;
    GameView* _gameView;
    PlayFieldView* _playFieldView;
    StackView* _stackView;
    std::string _levelFilePath;
};

#endif // __GAME_CONTROLLER_H__
