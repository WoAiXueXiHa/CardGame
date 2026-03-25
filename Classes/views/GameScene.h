#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include "controllers/GameController.h"

class CocosGameView;
class CocosPlayFieldView;
class CocosStackView;

// 游戏主场景：创建MVC对象并完成装配。
class GameScene : public cocos2d::Scene
{
public:
    static GameScene* create();

    bool init() override;

private:
    // 初始化MVC：创建视图、绑定控制器、加载关卡。
    bool _initMvc();

private:
    GameController _gameController;
    CocosGameView* _gameView;
    CocosPlayFieldView* _playFieldView;
    CocosStackView* _stackView;
};

#endif // __GAME_SCENE_H__
