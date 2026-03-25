#include "views/GameScene.h"

#include <new>
#include "views/CocosGameView.h"
#include "views/CocosPlayFieldView.h"
#include "views/CocosStackView.h"

USING_NS_CC;

GameScene* GameScene::create()
{
    GameScene* ret = new (std::nothrow) GameScene();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool GameScene::init()
{
    if (!Scene::init()) {
        return false;
    }

    _gameView = nullptr;
    _playFieldView = nullptr;
    _stackView = nullptr;
    return _initMvc();
}

bool GameScene::_initMvc()
{
    _gameView = CocosGameView::create();
    _playFieldView = CocosPlayFieldView::create();
    _stackView = CocosStackView::create();
    if (!_gameView || !_playFieldView || !_stackView) {
        return false;
    }

    this->addChild(_gameView, 1);
    this->addChild(_playFieldView, 2);
    this->addChild(_stackView, 2);

    _playFieldView->setPosition(Vec2(-110.0f, 580.0f));
    _playFieldView->setScale(1.20f);
    _stackView->setPosition(Vec2(630.0f, 400.0f));
    _stackView->setScale(1.20f);

    if (!_gameController.init(_gameView, _playFieldView, _stackView)) {
        return false;
    }

    // 加载关卡
    if (!_gameController.startGame("level_1.json")) {
        return false;
    }

    _playFieldView->setOnCardClick([this](int32_t cardId) {
        _gameView->emitPlayFieldCardClick(cardId);
    });

    _stackView->setOnDrawClick([this]() {
        _gameView->emitDrawCardClick();
    });

    return true;
}
