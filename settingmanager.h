#ifndef SettingManager_H
#define SettingManager_H

#include <QObject>
#include <QSettings>
#include <QString>

class SettingManager : public QObject
{
    Q_OBJECT
public:
    explicit SettingManager(const QString &configPath, QObject *parent = nullptr);

    // 读取配置参数
    QString getDomain() const;
    QString getPort() const;
    QString getDeviceAddr() const;
    QString getPath() const;
    QString getFormat() const;

    // 设置配置参数
    void setDomain(const QString &domain);
    void setPort(const QString &port);
    void setDeviceAddr(const QString &addr);
    void setPath(const QString &path);
    void setFormat(const QString &format);

    // 保存配置
    bool saveConfig();

    // 加载配置
    bool loadConfig();

private:
    QSettings *m_settings;
    QString m_domain;
    QString m_port;
    QString m_addr;
    QString m_path;
    QString m_format;
};

#endif // SettingManager_H
