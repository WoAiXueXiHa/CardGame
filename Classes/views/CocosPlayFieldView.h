#ifndef __COCOS_PLAY_FIELD_VIEW_H__
#define __COCOS_PLAY_FIELD_VIEW_H__

#include "views/PlayFieldView.h"
#include "models/GameModel.h"
#include "cocos2d.h"
#include <unordered_map>

// Cocos主牌区实现：渲染卡牌、命中测试、动画播放。
class CocosPlayFieldView : public cocos2d::Node, public PlayFieldView
{
public:
    static CocosPlayFieldView* create();

    bool init() override;

    // @param gameModel 只读模型。
    // @param controller 主牌区控制器。
    void bind(const GameModel* gameModel, const PlayFieldController* controller) override;
    void refresh() override;

    // @param callback 点牌回调，参数为cardId。
    void setOnCardClick(const std::function<void(int32_t)>& callback) override;

    // @param cardId 被匹配卡牌ID。
    // @param targetX/targetY 动画终点（顶部牌位置）。
    void playMatchAnimation(int32_t cardId, float targetX, float targetY) override;

    // @param sourceCardId 回退卡牌ID。
    // @param sourceX/sourceY 回退终点（原位）。
    // @param targetCardId 被替换目标牌ID。
    // @param targetX/targetY 回退起点（顶部牌位置）。
    void playUndoAnimation(int32_t sourceCardId,
                           float sourceX,
                           float sourceY,
                           int32_t targetCardId,
                           float targetX,
                           float targetY) override;

private:
    void _initTouchListener();

    // @param worldPoint 世界坐标触点。
    // @return 命中的cardId，未命中返回-1。
    int32_t _hitTestCardId(const cocos2d::Vec2& worldPoint) const;

    // @param card 待渲染卡牌数据。
    cocos2d::Node* _createCardNode(const CardData& card) const;

private:
    const GameModel* _gameModel;
    const PlayFieldController* _controller;
    std::function<void(int32_t)> _onCardClick;
    std::unordered_map<int32_t, cocos2d::Node*> _cardNodes;
    std::unordered_map<cocos2d::Node*, int32_t> _nodeToCardId;
};

#endif // __COCOS_PLAY_FIELD_VIEW_H__
