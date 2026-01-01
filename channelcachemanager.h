#ifndef CHANNELCACHEMANAGER_H
#define CHANNELCACHEMANAGER_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QReadWriteLock>
#include <QList>
#include "channelparaconifg.h"

// 信道缓存管理使用的结构体
typedef struct ChannelSetting
{
    int channelNum;//信道编号
    double signalAnt;             // 信号衰减
    int filterNum;                // 滤波器编号
    QList<MultiPathType> multipathType; // 多径类型列表
    bool switchFlag = false;      // 开关状态
    bool isChange = false;        // 是否改变
}ChannelSetting;

class ChannelCacheManager : public QObject
{
    Q_OBJECT

public:
    // 获取单例实例
    static ChannelCacheManager* instance();

    // 更新信道设置（除开关外的参数）
    void updateChannelParameters(int channelKey, const ChannelSetting& setting);

    // 更新开关状态
    void updateChannelSwitch(int channelKey, bool switchFlag);

    // 获取信道设置
    ChannelSetting getChannelSetting(int channelKey);

    // 获取所有信道设置
    QMap<int, ChannelSetting> getAllChannelSettings();

    // 根据key获取值（额外接口）
    ChannelSetting getValue(int key);

    // 更新isChange为false
    void setChannelNotChanged(int channelKey);

signals:
    // 开关状态改变信号
    void switchStateChanged(int channelKey, bool switchFlag);

    // 参数改变信号
    void parameterChanged(int channelKey, const ChannelSetting& newSetting);

private:
    // 构造函数私有化
    explicit ChannelCacheManager(QObject *parent = nullptr);

    // 初始化信道缓存
    void initCache();

    // 单例实例
    static ChannelCacheManager* m_instance;
    static QMutex m_mutex;

    // 读写锁，用于线程安全
    QReadWriteLock m_rwLock;

    // 信道设置缓存
    QMap<int, ChannelSetting> m_channelCache;
};

#endif // CHANNELCACHEMANAGER_H
