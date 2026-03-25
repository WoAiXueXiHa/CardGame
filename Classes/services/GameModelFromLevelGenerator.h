#ifndef __GAME_MODEL_FROM_LEVEL_GENERATOR_H__
#define __GAME_MODEL_FROM_LEVEL_GENERATOR_H__

#include <cstdint>
#include "configs/LevelConfig.h"
#include "models/GameModel.h"

// 将关卡配置转换为运行时GameModel。
class GameModelFromLevelGenerator
{
public:
    GameModelFromLevelGenerator();
    ~GameModelFromLevelGenerator();

    // @param levelConfig 输入关卡配置。
    // @param outGameModel 输出运行时模型。
    bool generate(const LevelConfig& levelConfig, GameModel& outGameModel);

private:
    // @param nextCardId 自增ID引用。
    int32_t _generateCardId(int32_t& nextCardId) const;

    // @param cardConfig 输入配置卡。
    // @param cardId 生成后的唯一ID。
    // @param isFlipped 运行时是否初始可见。
    CardData _convertCardConfigToCardData(const CardConfig& cardConfig,
                                         int32_t cardId,
                                         bool isFlipped = true) const;
};

#endif // __GAME_MODEL_FROM_LEVEL_GENERATOR_H__
