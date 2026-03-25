#ifndef __GAME_MODEL_H__
#define __GAME_MODEL_H__

#include <cstdint>
#include <vector>
#include <string>

// 运行时卡牌数据。
struct CardData
{
    int32_t cardId;   // 唯一ID
    int32_t cardFace; // 点数：0=A ... 12=K
    int32_t cardSuit; // 花色：0=club,1=diamond,2=heart,3=spade
    float posX;       // 场景X坐标
    float posY;       // 场景Y坐标
    bool isFlipped;   // 是否可见
    bool isMatched;   // 是否已被匹配移除

    CardData()
        : cardId(-1), cardFace(-1), cardSuit(-1), posX(0.0f), posY(0.0f), isFlipped(false), isMatched(false)
    {
    }

    CardData(int32_t id, int32_t face, int32_t suit, float x, float y, bool flipped = true)
        : cardId(id), cardFace(face), cardSuit(suit), posX(x), posY(y), isFlipped(flipped), isMatched(false)
    {
    }
};

// 游戏运行时数据模型。
class GameModel
{
public:
    GameModel();
    ~GameModel();

    // @param card 加入主牌区的卡。
    void addPlayFieldCard(const CardData& card);
    const std::vector<CardData>& getPlayFieldCards() const;
    std::vector<CardData>& getPlayFieldCardsMutable();

    // @param cardId 目标卡ID。
    CardData* getPlayFieldCard(int32_t cardId);
    bool removePlayFieldCard(int32_t cardId);

    // @param card 设置新的顶部牌。
    void setTrayTopCard(const CardData& card);
    const CardData& getTrayTopCard() const;
    CardData* getTrayTopCardMutable();

    // @param card 压入手牌堆的卡。
    void addStackCard(const CardData& card);
    const std::vector<CardData>& getStackCards() const;

    // 弹出手牌堆顶部卡。
    CardData popStackCard();
    int32_t getStackCardCount() const;

    // 导出运行时状态（用于存档）。
    std::string toJsonString() const;

    // 从JSON字符串恢复运行时状态（用于读档）。
    bool loadFromJsonString(const std::string& jsonString);

    void clear();
    bool isGameOver() const;

    // @param over 游戏结束状态。
    void setGameOver(bool over);

private:
    std::vector<CardData> _playFieldCards;
    CardData _trayTopCard;
    std::vector<CardData> _stackCards;
    bool _isGameOver;
};

#endif // __GAME_MODEL_H__
