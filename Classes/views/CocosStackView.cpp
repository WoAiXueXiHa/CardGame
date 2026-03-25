#include "views/CocosStackView.h"

#include <new>
#include "controllers/StackController.h"
#include "models/GameModel.h"

namespace
{
const char* kFaceTextureNames[] = {
    "big_red_A.png", "big_red_2.png", "big_red_3.png", "big_red_4.png", "big_red_5.png", "big_red_6.png",
    "big_red_7.png", "big_red_8.png", "big_red_9.png", "big_red_10.png", "big_red_J.png", "big_red_Q.png", "big_red_K.png"
};

const char* kSuitTextureNames[] = {
    "club.png", "diamond.png", "heart.png", "spade.png"
};

std::string faceToTexturePath(int32_t face)
{
    if (face < 0 || face >= 13) {
        return "";
    }
    return std::string("card/") + kFaceTextureNames[face];
}

std::string suitToTexturePath(int32_t suit)
{
    if (suit < 0 || suit >= 4) {
        return "";
    }
    return std::string("suits/") + kSuitTextureNames[suit];
}
}

USING_NS_CC;

CocosStackView* CocosStackView::create()
{
    CocosStackView* ret = new (std::nothrow) CocosStackView();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool CocosStackView::init()
{
    if (!Node::init()) {
        return false;
    }

    _gameModel = nullptr;
    _controller = nullptr;
    _countLabel = nullptr;
    _drawButton = nullptr;
    _trayCardNode = nullptr;
    _stackCardsLayer = nullptr;
    _isGameOver = false;

    _initButton();
    updateStackDisplay(0);
    return true;
}

void CocosStackView::bind(const GameModel* gameModel, const StackController* controller)
{
    _gameModel = gameModel;
    _controller = controller;
    if (_gameModel) {
        updateStackDisplay(_gameModel->getStackCardCount());
    } else {
        updateStackDisplay(0);
    }
}

void CocosStackView::setOnDrawClick(const std::function<void()>& callback)
{
    _onDrawClick = callback;
}

void CocosStackView::playDrawAnimation(int32_t cardFace,
                                       int32_t cardSuit,
                                       float fromX,
                                       float fromY,
                                       float toX,
                                       float toY)
{
    auto animCard = _createCardNode(cardFace, cardSuit);
    if (!animCard) {
        updateStackDisplay(_gameModel ? _gameModel->getStackCardCount() : 0);
        return;
    }

    this->addChild(animCard, 10);
    animCard->setPosition(Vec2(fromX - this->getPositionX(), fromY - this->getPositionY()));
    animCard->runAction(Sequence::create(
        MoveTo::create(0.2f, Vec2(toX - this->getPositionX(), toY - this->getPositionY())),
        RemoveSelf::create(),
        nullptr));

    updateStackDisplay(_gameModel ? _gameModel->getStackCardCount() : 0);
}

void CocosStackView::playUndoDrawAnimation(int32_t cardFace,
                                           int32_t cardSuit,
                                           float fromX,
                                           float fromY,
                                           float toX,
                                           float toY)
{
    auto animCard = _createCardNode(cardFace, cardSuit);
    if (!animCard) {
        updateStackDisplay(_gameModel ? _gameModel->getStackCardCount() : 0);
        return;
    }

    this->addChild(animCard, 10);
    animCard->setPosition(Vec2(fromX - this->getPositionX(), fromY - this->getPositionY()));
    animCard->runAction(Sequence::create(
        MoveTo::create(0.2f, Vec2(toX - this->getPositionX(), toY - this->getPositionY())),
        RemoveSelf::create(),
        nullptr));

    updateStackDisplay(_gameModel ? _gameModel->getStackCardCount() : 0);
}

void CocosStackView::updateStackDisplay(int32_t remainingCount)
{
    if (_countLabel) {
        _countLabel->setString("Stack: " + std::to_string(remainingCount));
    }

    if (_trayCardNode) {
        _trayCardNode->removeFromParent();
        _trayCardNode = nullptr;
    }

    if (_gameModel) {
        const CardData& tray = _gameModel->getTrayTopCard();
        if (tray.cardFace >= 0) {
            _trayCardNode = _createCardNode(tray.cardFace, tray.cardSuit);
            if (_trayCardNode) {
                _trayCardNode->setPosition(Vec2(0.0f, 0.0f));
                this->addChild(_trayCardNode, 5);
            }
        }
    }

    _refreshStackCardsPreview();
}

void CocosStackView::_initButton()
{
    auto trayBg = DrawNode::create();
    Vec2 trayVerts[4] = {
        Vec2(-98.0f, -144.0f),
        Vec2(98.0f, -144.0f),
        Vec2(98.0f, 144.0f),
        Vec2(-98.0f, 144.0f)
    };
    trayBg->drawSolidPoly(trayVerts, 4, Color4F(0.92f, 0.92f, 0.92f, 0.35f));
    trayBg->drawPoly(trayVerts, 4, true, Color4F::WHITE);
    trayBg->setPosition(Vec2(0.0f, 0.0f));
    this->addChild(trayBg);

    _drawButton = DrawNode::create();
    Vec2 verts[4] = {
        Vec2(-83.0f, -50.0f),
        Vec2(83.0f, -50.0f),
        Vec2(83.0f, 50.0f),
        Vec2(-83.0f, 50.0f)
    };
    _drawButton->drawSolidPoly(verts, 4, Color4F(0.2f, 0.35f, 0.7f, 1.0f));
    _drawButton->drawPoly(verts, 4, true, Color4F::WHITE);
    _drawButton->setPosition(Vec2(233.0f, -233.0f));
    this->addChild(_drawButton);

    auto buttonText = Label::createWithSystemFont("Draw", "Arial", 28);
    buttonText->setPosition(Vec2(233.0f, -233.0f));
    this->addChild(buttonText);

    _countLabel = Label::createWithSystemFont("Stack: 0", "Arial", 30);
    _countLabel->setPosition(Vec2(233.0f, -343.0f));
    this->addChild(_countLabel);

    _stackCardsLayer = Node::create();
    this->addChild(_stackCardsLayer, 2);

    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        CC_UNUSED_PARAM(event);
        if (_isGameOver) {
            return false;
        }
        Rect drawRect(150.0f, -283.0f, 166.0f, 100.0f);
        Rect stackRect(-360.0f, -144.0f, 350.0f, 288.0f);
        Vec2 localPoint = this->convertToNodeSpace(touch->getLocation());
        if (!drawRect.containsPoint(localPoint) && !stackRect.containsPoint(localPoint)) {
            return false;
        }
        if (_onDrawClick) {
            _onDrawClick();
        }
        return true;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void CocosStackView::setGameOverState(bool gameOver)
{
    _isGameOver = gameOver;
}

void CocosStackView::_refreshStackCardsPreview()
{
    if (!_stackCardsLayer) {
        return;
    }

    _stackCardsLayer->removeAllChildren();

    if (!_gameModel) {
        return;
    }

    const auto& cards = _gameModel->getStackCards();
    if (cards.empty()) {
        return;
    }

    const int32_t maxPreview = 3;
    const int32_t total = static_cast<int32_t>(cards.size());
    const int32_t start = total > maxPreview ? total - maxPreview : 0;

    for (int32_t i = start; i < total; ++i) {
        const int32_t localIndex = i - start;
        auto cardNode = _createCardNode(cards[i].cardFace, cards[i].cardSuit);
        if (!cardNode) {
            continue;
        }

        cardNode->setScale(0.92f);
        cardNode->setPosition(Vec2(-275.0f + localIndex * 58.0f, 0.0f));
        _stackCardsLayer->addChild(cardNode, localIndex);
    }
}

Node* CocosStackView::_createCardNode(int32_t cardFace, int32_t cardSuit) const
{
    auto node = Node::create();

    auto cardBase = Sprite::create("card_general.png");
    if (cardBase) {
        cardBase->setScale(0.64f);
        node->addChild(cardBase);
    } else {
        auto fallback = DrawNode::create();
        Vec2 verts[4] = {
            Vec2(-74.0f, -108.0f),
            Vec2(74.0f, -108.0f),
            Vec2(74.0f, 108.0f),
            Vec2(-74.0f, 108.0f)
        };
        fallback->drawSolidPoly(verts, 4, Color4F(0.95f, 0.95f, 0.95f, 1.0f));
        fallback->drawPoly(verts, 4, true, Color4F::BLACK);
        node->addChild(fallback);
    }

    auto faceSprite = Sprite::create(faceToTexturePath(cardFace));
    if (faceSprite) {
        faceSprite->setScale(0.64f);
        faceSprite->setPosition(Vec2(0.0f, 10.0f));
        node->addChild(faceSprite, 2);
    }

    auto suitSprite = Sprite::create(suitToTexturePath(cardSuit));
    if (suitSprite) {
        suitSprite->setScale(0.58f);
        suitSprite->setPosition(Vec2(-34.0f, 58.0f));
        node->addChild(suitSprite, 2);
    }

    return node;
}
