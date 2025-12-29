#include "configmanager.h"
#include <QDebug>
#include <QMap>


QMutex globalMutex;
QMap<QString, ModelParaSetting>globalParaMap; // 定义全局 Map
QMap<int, RadioStatus>globalStatusMap; // 定义全局 Map

ConfigManager::ConfigManager(QObject *parent)
    : QObject{parent}
{}


// 添加配置到 Map
void ConfigManager::addConfigToMap(const QString& key, const ModelParaSetting& config)
{
    if (globalParaMap.contains(key)) {
        qWarning() << "Key already exists:" << key;
        return;
    }
    globalParaMap.insert(key, config);
    qDebug() << "Added config:" << key;
}

// 从 Map 中移除配置
bool ConfigManager::removeConfigFromMap(const QString& key)
{
    if (globalParaMap.remove(key) > 0) {
        qDebug() << "Removed config:" << key;
        return true;
    }
    qWarning() << "Config not found:" << key;
    return false;
}

// 获取配置
ModelParaSetting ConfigManager::getConfigFromMap(const QString& key)
{
    if (!globalParaMap.contains(key)) {
        qWarning() << "Config not found:" << key;
    }
    return globalParaMap[key];
}

// 更新配置
bool ConfigManager::updateConfigInMap(const QString& key, const ModelParaSetting& config)
{
    if (globalParaMap.contains(key)) {
        globalParaMap[key] = config;
        qDebug() << "Updated config:" << key;
        return true;
    }
    qWarning() << "Config not found for update:" << key;
    return false;
}

// 获取所有配置键
QList<QString> ConfigManager::getAllConfigKeys()
{
    return globalParaMap.keys();
}

// 清空 Map
void ConfigManager::clearGlobalMap()
{
    globalParaMap.clear();
    qDebug() << "Global map cleared";
}
