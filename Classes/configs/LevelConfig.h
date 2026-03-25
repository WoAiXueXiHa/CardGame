#ifndef __LEVEL_CONFIG_H__
#define __LEVEL_CONFIG_H__

#include <vector>
#include <cstdint>
#include <string>

// 单张卡牌的关卡配置。
struct CardConfig
{
    int32_t cardFace; // 点数：0=A, 1=2, ... 12=K
    int32_t cardSuit; // 花色：0=club, 1=diamond, 2=heart, 3=spade
    float posX;       // 初始X坐标（场景坐标）
    float posY;       // 初始Y坐标（场景坐标）

    CardConfig()
        : cardFace(-1), cardSuit(-1), posX(0.0f), posY(0.0f) {}

    CardConfig(int32_t face, int32_t suit, float x, float y)
        : cardFace(face), cardSuit(suit), posX(x), posY(y) {}
};

// 关卡配置容器：主牌区、手牌堆、初始顶部牌。
class LevelConfig
{
public:
    LevelConfig();
    ~LevelConfig();

    // @param levelId 关卡ID。
    void setLevelId(int32_t levelId);
    int32_t getLevelId() const;

    // @param cardConfig 要加入主牌区的卡牌配置。
    void addPlayFieldCard(const CardConfig& cardConfig);
    const std::vector<CardConfig>& getPlayFieldCards() const;
    int32_t getPlayFieldCardCount() const;

    // @param cardConfig 要加入手牌堆的卡牌配置。
    void addStackCard(const CardConfig& cardConfig);
    const std::vector<CardConfig>& getStackCards() const;
    int32_t getStackCardCount() const;

    // @param cardConfig 初始顶部牌（底牌）配置。
    void setInitialTrayCard(const CardConfig& cardConfig);
    const CardConfig& getInitialTrayCard() const;

    // 清空关卡配置（用于重新加载）。
    void clear();

private:
    int32_t _levelId;
    std::vector<CardConfig> _playFieldCards;
    std::vector<CardConfig> _stackCards;
    CardConfig _initialTrayCard;
};

#endif // __LEVEL_CONFIG_H__
