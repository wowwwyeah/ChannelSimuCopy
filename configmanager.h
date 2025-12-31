#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include "channelparaconifg.h"

extern QMutex globalMutex;
extern QMap<QString, ModelParaSetting> globalParaMap; // 定义全局 Map
extern QMap<int, RadioStatus> globalStatusMap; // 定义全局 Map

class ConfigManager : public QObject
{
    Q_OBJECT

signals:
    // 配置更新信号，当updateConfigInMap函数被调用时发出
    void configUpdated(const QString& key, const ModelParaSetting& config);

public:
    explicit ConfigManager(QObject *parent = nullptr);

    // 操作函数声明
    void addConfigToMap(const QString& key, const ModelParaSetting& config);
    bool removeConfigFromMap(const QString& key);
    ModelParaSetting getConfigFromMap(const QString& key);
    bool updateConfigInMap(const QString& key, const ModelParaSetting& config);
    QList<QString> getAllConfigKeys();
    void clearGlobalMap();
};

#endif // CONFIGMANAGER_H
