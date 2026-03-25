#include "AppDelegate.h"
#include "cocos2d.h"

/**
 * @brief 程序入口
 */
int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    AppDelegate app;
    return cocos2d::Application::getInstance()->run();
}
