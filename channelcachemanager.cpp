#include "channelcachemanager.h"

// 初始化静态成员变量
ChannelCacheManager* ChannelCacheManager::m_instance = nullptr;
QMutex ChannelCacheManager::m_mutex;

ChannelCacheManager::ChannelCacheManager(QObject *parent)
    : QObject(parent)
{
    // 初始化缓存
    initCache();
}

ChannelCacheManager* ChannelCacheManager::instance()
{
    // 双重检查锁定模式，确保线程安全的单例实例创建
    if (!m_instance) {
        QMutexLocker locker(&m_mutex);
        if (!m_instance) {
            m_instance = new ChannelCacheManager();
        }
    }
    return m_instance;
}

void ChannelCacheManager::initCache()
{
    // 初始化键从1到15的缓存
    for (int i = 1; i <= 15; ++i) {
        ChannelSetting setting;
        setting.channelNum=i;
        setting.signalAnt = 0.0;
        setting.filterNum = 0;
        // 初始化一个 MultiPathType 实例并添加到列表中
        MultiPathType multiPath;
        multiPath.pathNum = 0;
        multiPath.relativDelay = 0;
        multiPath.antPower = 0;
        multiPath.freShift = 0;
        multiPath.freSpread = 0;
        multiPath.dopplerType = 0;
        setting.multipathType.append(multiPath);
        setting.switchFlag = false;
        setting.isChange = false;

        // 使用写锁保护缓存更新
        QWriteLocker locker(&m_rwLock);
        m_channelCache[i] = setting;
    }
}

void ChannelCacheManager::updateChannelParameters(int channelKey, const ChannelSetting& newSetting)
{
    // 检查信道键是否在有效范围内（对负key取绝对值）
    int absKey = abs(channelKey);
    if (absKey < 1 || absKey > 15) {
        return;
    }

    // 使用写锁保护缓存更新
    QWriteLocker locker(&m_rwLock);

    // 检查是否存在该信道
    if (!m_channelCache.contains(absKey)) {
        return;
    }

    // 获取旧设置的开关状态
    bool oldSwitchFlag = m_channelCache[absKey].switchFlag;

    // 创建一个新的设置副本，保留旧的开关状态
    ChannelSetting updatedSetting = newSetting;
    updatedSetting.switchFlag = oldSwitchFlag;
    updatedSetting.isChange = true;

    // 更新缓存
    m_channelCache[absKey] = updatedSetting;

    // 触发参数改变信号
    emit parameterChanged(absKey, updatedSetting);
}

void ChannelCacheManager::updateChannelSwitch(int channelKey, bool switchFlag)
{
    // 检查信道键是否在有效范围内（对负key取绝对值）
    int absKey = abs(channelKey);
    if (absKey < 1 || absKey > 15) {
        return;
    }

    // 使用写锁保护缓存更新
    QWriteLocker locker(&m_rwLock);

    // 检查是否存在该信道
    if (!m_channelCache.contains(absKey)) {
        return;
    }

    // 获取旧设置
    ChannelSetting oldSetting = m_channelCache[absKey];

    // 检查开关状态是否改变
    if (oldSetting.switchFlag != switchFlag) {
        // 更新开关状态
        ChannelSetting updatedSetting = oldSetting;
        updatedSetting.switchFlag = switchFlag;
        updatedSetting.isChange = true;

        m_channelCache[absKey] = updatedSetting;

        // 触发开关状态改变信号
        emit switchStateChanged(absKey, switchFlag);
    }
}

ChannelSetting ChannelCacheManager::getChannelSetting(int channelKey)
{
    // 使用读锁保护缓存访问
    QReadLocker locker(&m_rwLock);

    // 对负key取绝对值
    int absKey = abs(channelKey);

    // 检查信道键是否存在
    if (m_channelCache.contains(absKey)) {
        return m_channelCache[absKey];
    }

    // 如果不存在，返回默认设置
    ChannelSetting defaultSetting;
    defaultSetting.signalAnt = 0.0;
    defaultSetting.filterNum = 0;

    // 初始化一个 MultiPathType 实例并添加到列表中
    MultiPathType multiPath;
    multiPath.pathNum = 0;
    multiPath.relativDelay = 0;
    multiPath.antPower = 0;
    multiPath.freShift = 0;
    multiPath.freSpread = 0;
    multiPath.dopplerType = 0;
    defaultSetting.multipathType.append(multiPath);

    defaultSetting.switchFlag = false;
    defaultSetting.isChange = false;
    return defaultSetting;
}

QMap<int, ChannelSetting> ChannelCacheManager::getAllChannelSettings()
{
    // 使用读锁保护缓存访问
    QReadLocker locker(&m_rwLock);
    return m_channelCache;
}

ChannelSetting ChannelCacheManager::getValue(int key)
{
    // 直接调用已有的 getChannelSetting 方法，避免代码重复
    return getChannelSetting(key);
}

void ChannelCacheManager::setChannelNotChanged(int channelKey)
{
    // 检查信道键是否在有效范围内（对负key取绝对值）
    int absKey = abs(channelKey);
    if (absKey < 1 || absKey > 15) {
        return;
    }

    // 使用写锁保护缓存更新
    QWriteLocker locker(&m_rwLock);

    // 检查是否存在该信道
    if (m_channelCache.contains(absKey)) {
        // 获取当前设置
        ChannelSetting currentSetting = m_channelCache[absKey];

        // 只在isChange为true时更新，避免不必要的修改
        if (currentSetting.isChange) {
            currentSetting.isChange = false;
            m_channelCache[absKey] = currentSetting;
        }
    }
}
