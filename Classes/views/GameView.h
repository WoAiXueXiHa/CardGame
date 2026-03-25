#ifndef __GAME_VIEW_H__
#define __GAME_VIEW_H__

#include <cstdint>
#include <functional>

// 主视图事件总线：把UI输入转发给控制器。
class GameView
{
public:
    GameView();
    virtual ~GameView();

    // @param callback 主牌区点牌回调，参数是cardId。
    void setOnPlayFieldCardClick(const std::function<void(int32_t)>& callback);

    // @param callback 抽牌按钮点击回调。
    void setOnDrawCardClick(const std::function<void()>& callback);

    // @param callback 撤销按钮点击回调。
    void setOnUndoClick(const std::function<void()>& callback);

    // @param callback 重新开始游戏回调。
    void setOnRestartClick(const std::function<void()>& callback);

    // @param callback 退出游戏回调。
    void setOnQuitClick(const std::function<void()>& callback);

    // @param cardId 被点击的主牌区卡牌ID。
    void emitPlayFieldCardClick(int32_t cardId) const;
    void emitDrawCardClick() const;
    void emitUndoClick() const;
    void emitRestartClick() const;
    void emitQuitClick() const;

    // @param gameOver 是否处于结束状态（用于UI提示/输入控制）。
    virtual void setGameOverState(bool gameOver);

protected:
    std::function<void(int32_t)> _onPlayFieldCardClick;
    std::function<void()> _onDrawCardClick;
    std::function<void()> _onUndoClick;
    std::function<void()> _onRestartClick;
    std::function<void()> _onQuitClick;
};

#endif // __GAME_VIEW_H__
