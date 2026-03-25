#ifndef __LEVEL_CONFIG_LOADER_H__
#define __LEVEL_CONFIG_LOADER_H__

#include <cstdint>
#include <string>
#include "LevelConfig.h"

// 关卡加载器：从JSON文件/字符串解析到LevelConfig。
class LevelConfigLoader
{
public:
    LevelConfigLoader();
    ~LevelConfigLoader();

    // @param filePath 关卡JSON文件路径。
    // @param outConfig 输出配置对象（成功时被填充）。
    bool loadFromFile(const std::string& filePath, LevelConfig& outConfig);

    // @param jsonString 关卡JSON文本。
    // @param outConfig 输出配置对象（成功时被填充）。
    bool loadFromString(const std::string& jsonString, LevelConfig& outConfig);

private:
    // @param jsonString JSON文本。
    // @param outConfig 输出配置对象。
    bool _parsePlayFieldCards(const std::string& jsonString, LevelConfig& outConfig);

    // @param jsonString JSON文本。
    // @param outConfig 输出配置对象。
    bool _parseStackCards(const std::string& jsonString, LevelConfig& outConfig);
};

#endif // __LEVEL_CONFIG_LOADER_H__
