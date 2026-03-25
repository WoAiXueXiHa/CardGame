#ifndef __APP_DELEGATE_H__
#define __APP_DELEGATE_H__

#include "cocos2d.h"

// 应用入口代理：负责启动、前后台切换。
class AppDelegate : private cocos2d::Application
{
public:
    AppDelegate();
    virtual ~AppDelegate();

    // 启动完成回调：创建GLView并启动首场景。
    virtual bool applicationDidFinishLaunching() override;

    // 进入后台回调：暂停渲染/动画。
    virtual void applicationDidEnterBackground() override;

    // 回到前台回调：恢复渲染/动画。
    virtual void applicationWillEnterForeground() override;

private:
    // 初始化OpenGL上下文属性（颜色/深度/模板位数）。
    void initGLContextAttrs();
};

#endif // __APP_DELEGATE_H__
