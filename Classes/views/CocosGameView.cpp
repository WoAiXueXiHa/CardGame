#include "views/CocosGameView.h"

#include <new>
USING_NS_CC;

CocosGameView* CocosGameView::create()
{
    CocosGameView* ret = new (std::nothrow) CocosGameView();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool CocosGameView::init()
{
    if (!Layer::init()) {
        return false;
    }

    _undoButton = nullptr;
    _gameOverPanel = nullptr;
    _isGameOver = false;

    _initUndoButton();
    _initGameOverPanel();
    return true;
}

void CocosGameView::setGameOverState(bool gameOver)
{
    _isGameOver = gameOver;
    if (_gameOverPanel) {
        _gameOverPanel->setVisible(gameOver);
    }
}

void CocosGameView::_initUndoButton()
{
    _undoButton = DrawNode::create();
    Vec2 verts[4] = {
        Vec2(-100.0f, -60.0f),
        Vec2(100.0f, -60.0f),
        Vec2(100.0f, 60.0f),
        Vec2(-100.0f, 60.0f)
    };
    _undoButton->drawSolidPoly(verts, 4, Color4F(0.8f, 0.2f, 0.25f, 1.0f));
    _undoButton->drawPoly(verts, 4, true, Color4F::WHITE);
    _undoButton->setPosition(Vec2(170.0f, 120.0f));
    this->addChild(_undoButton);

    auto text = Label::createWithSystemFont("Undo", "Arial", 34);
    text->setPosition(Vec2(170.0f, 120.0f));
    this->addChild(text);

    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        CC_UNUSED_PARAM(event);
        if (_isGameOver) {
            return false;
        }
        Rect rect(70.0f, 60.0f, 200.0f, 120.0f);
        if (!rect.containsPoint(touch->getLocation())) {
            return false;
        }
        emitUndoClick();
        return true;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void CocosGameView::_initGameOverPanel()
{
    _gameOverPanel = Node::create();
    _gameOverPanel->setPosition(Vec2(540.0f, 1040.0f));
    _gameOverPanel->setVisible(false);
    this->addChild(_gameOverPanel, 30);

    // 半透明背景
    auto bg = DrawNode::create();
    Vec2 bgVerts[4] = {
        Vec2(-280.0f, -220.0f),
        Vec2(280.0f, -220.0f),
        Vec2(280.0f, 220.0f),
        Vec2(-280.0f, 220.0f)
    };
    bg->drawSolidPoly(bgVerts, 4, Color4F(0.08f, 0.08f, 0.1f, 0.92f));
    bg->drawPoly(bgVerts, 4, true, Color4F(0.9f, 0.9f, 1.0f, 0.8f));
    _gameOverPanel->addChild(bg);

    // 标题
    auto title = Label::createWithSystemFont("Game Over", "Arial", 54);
    title->setPosition(Vec2(0.0f, 120.0f));
    title->setColor(Color3B(255, 220, 80));
    _gameOverPanel->addChild(title);

    // 重新开始按钮
    auto restartBtn = DrawNode::create();
    Vec2 rVerts[4] = {
        Vec2(-130.0f, -40.0f),
        Vec2(130.0f, -40.0f),
        Vec2(130.0f, 40.0f),
        Vec2(-130.0f, 40.0f)
    };
    restartBtn->drawSolidPoly(rVerts, 4, Color4F(0.15f, 0.55f, 0.25f, 1.0f));
    restartBtn->drawPoly(rVerts, 4, true, Color4F::WHITE);
    restartBtn->setPosition(Vec2(-150.0f, -80.0f));
    _gameOverPanel->addChild(restartBtn);

    auto restartText = Label::createWithSystemFont("New Game", "Arial", 30);
    restartText->setPosition(Vec2(-150.0f, -80.0f));
    _gameOverPanel->addChild(restartText);

    // 退出按钮
    auto quitBtn = DrawNode::create();
    Vec2 qVerts[4] = {
        Vec2(-130.0f, -40.0f),
        Vec2(130.0f, -40.0f),
        Vec2(130.0f, 40.0f),
        Vec2(-130.0f, 40.0f)
    };
    quitBtn->drawSolidPoly(qVerts, 4, Color4F(0.65f, 0.15f, 0.15f, 1.0f));
    quitBtn->drawPoly(qVerts, 4, true, Color4F::WHITE);
    quitBtn->setPosition(Vec2(150.0f, -80.0f));
    _gameOverPanel->addChild(quitBtn);

    auto quitText = Label::createWithSystemFont("Quit", "Arial", 30);
    quitText->setPosition(Vec2(150.0f, -80.0f));
    _gameOverPanel->addChild(quitText);

    // 触摸监听
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        CC_UNUSED_PARAM(event);
        if (!_isGameOver || !_gameOverPanel->isVisible()) {
            return false;
        }

        Vec2 localPoint = _gameOverPanel->convertToNodeSpace(touch->getLocation());

        Rect restartRect(-280.0f, -120.0f, 260.0f, 80.0f);
        if (restartRect.containsPoint(localPoint)) {
            emitRestartClick();
            return true;
        }

        Rect quitRect(20.0f, -120.0f, 260.0f, 80.0f);
        if (quitRect.containsPoint(localPoint)) {
            emitQuitClick();
            return true;
        }

        return false;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, _gameOverPanel);
}
