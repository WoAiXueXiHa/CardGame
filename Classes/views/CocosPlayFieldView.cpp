#include "views/CocosPlayFieldView.h"

#include <new>
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

CocosPlayFieldView* CocosPlayFieldView::create()
{
    CocosPlayFieldView* ret = new (std::nothrow) CocosPlayFieldView();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool CocosPlayFieldView::init()
{
    if (!Node::init()) {
        return false;
    }
    _gameModel = nullptr;
    _controller = nullptr;
    _initTouchListener();
    return true;
}

void CocosPlayFieldView::bind(const GameModel* gameModel, const PlayFieldController* controller)
{
    _gameModel = gameModel;
    _controller = controller;
    refresh();
}

void CocosPlayFieldView::refresh()
{
    this->removeAllChildren();
    _cardNodes.clear();
    _nodeToCardId.clear();

    if (!_gameModel) {
        return;
    }

    const std::vector<CardData>& cards = _gameModel->getPlayFieldCards();
    for (const CardData& card : cards) {
        if (card.isMatched) {
            continue;
        }
        Node* cardNode = _createCardNode(card);
        this->addChild(cardNode);
        _cardNodes[card.cardId] = cardNode;
        _nodeToCardId[cardNode] = card.cardId;
    }
}

void CocosPlayFieldView::setOnCardClick(const std::function<void(int32_t)>& callback)
{
    _onCardClick = callback;
}

void CocosPlayFieldView::playMatchAnimation(int32_t cardId, float targetX, float targetY)
{
    auto it = _cardNodes.find(cardId);
    if (it == _cardNodes.end()) {
        refresh();
        return;
    }

    it->second->runAction(Sequence::create(
        MoveTo::create(0.2f, Vec2(targetX, targetY)),
        CallFunc::create([this]() { refresh(); }),
        nullptr));
}

void CocosPlayFieldView::playUndoAnimation(int32_t sourceCardId,
                                           float sourceX,
                                           float sourceY,
                                           int32_t targetCardId,
                                           float targetX,
                                           float targetY)
{
    CC_UNUSED_PARAM(targetCardId);

    refresh();
    auto it = _cardNodes.find(sourceCardId);
    if (it == _cardNodes.end()) {
        return;
    }
    it->second->setPosition(Vec2(targetX, targetY));
    it->second->runAction(MoveTo::create(0.2f, Vec2(sourceX, sourceY)));
}

void CocosPlayFieldView::_initTouchListener()
{
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        CC_UNUSED_PARAM(event);
        int32_t cardId = _hitTestCardId(touch->getLocation());
        if (cardId < 0) {
            return false;
        }
        if (_onCardClick) {
            _onCardClick(cardId);
        }
        return true;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

int32_t CocosPlayFieldView::_hitTestCardId(const Vec2& worldPoint) const
{
    const auto& children = this->getChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        Node* cardNode = *it;
        auto idIt = _nodeToCardId.find(cardNode);
        if (idIt == _nodeToCardId.end()) {
            continue;
        }

        Rect localRect(-74.0f, -108.0f, 148.0f, 216.0f);
        Vec2 localPoint = cardNode->convertToNodeSpace(worldPoint);
        if (localRect.containsPoint(localPoint)) {
            return idIt->second;
        }
    }
    return -1;
}

Node* CocosPlayFieldView::_createCardNode(const CardData& card) const
{
    auto container = Node::create();
    container->setPosition(Vec2(card.posX, card.posY));

    auto cardBase = Sprite::create("card_general.png");
    if (cardBase) {
        cardBase->setScale(0.64f);
        container->addChild(cardBase);
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
        container->addChild(fallback);
    }

    if (!card.isFlipped) {
        auto cover = DrawNode::create();
        Vec2 coverVerts[4] = {
            Vec2(-62.0f, -96.0f),
            Vec2(62.0f, -96.0f),
            Vec2(62.0f, 96.0f),
            Vec2(-62.0f, 96.0f)
        };
        cover->drawSolidPoly(coverVerts, 4, Color4F(0.18f, 0.18f, 0.2f, 0.88f));
        cover->drawPoly(coverVerts, 4, true, Color4F(0.8f, 0.8f, 0.85f, 0.9f));
        container->addChild(cover, 2);
        return container;
    }

    auto faceSprite = Sprite::create(faceToTexturePath(card.cardFace));
    if (faceSprite) {
        faceSprite->setScale(0.64f);
        faceSprite->setPosition(Vec2(0.0f, 10.0f));
        container->addChild(faceSprite, 2);
    }

    auto suitSprite = Sprite::create(suitToTexturePath(card.cardSuit));
    if (suitSprite) {
        suitSprite->setScale(0.58f);
        suitSprite->setPosition(Vec2(-34.0f, 58.0f));
        container->addChild(suitSprite, 2);
    }

    return container;
}
