#include "settingmanager.h"
#include <QDir>
#include <QDebug>

SettingManager::SettingManager(const QString &configPath, QObject *parent)
    : QObject(parent)
{
    // 确保配置文件所在目录存在
    QDir dir(QFileInfo(configPath).absolutePath());
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 创建QSettings对象，使用INI格式
    m_settings = new QSettings(configPath, QSettings::IniFormat, this);
}

QString SettingManager::getDomain() const
{
    return m_domain;
}

QString SettingManager::getPort() const
{
    return m_port;
}

QString SettingManager::getDeviceAddr() const
{
    return m_addr;
}

QString SettingManager::getPath() const
{
    return m_path;
}

QString SettingManager::getFormat() const
{
    return m_format;
}

void SettingManager::setDomain(const QString &domain)
{
    m_domain = domain;
}

void SettingManager::setPort(const QString &port)
{
    m_port = port;
}

void SettingManager::setDeviceAddr(const QString &addr)
{
    m_addr = addr;
}

void SettingManager::setPath(const QString &path)
{
    m_path = path;
}

void SettingManager::setFormat(const QString &format)
{
    m_format = format;
}

bool SettingManager::saveConfig()
{
    if (!m_settings) {
        return false;
    }

    // 将参数写入INI文件，使用"General"分组
    m_settings->beginGroup("General");
    m_settings->setValue("Domain", m_domain);
    m_settings->setValue("Port", m_port);
    m_settings->setValue("Addr", m_addr);
    m_settings->setValue("Path", m_path);
    m_settings->setValue("Format", m_format);
    m_settings->endGroup();

    // 确保写入磁盘
    m_settings->sync();

    return m_settings->status() == QSettings::NoError;
}

bool SettingManager::loadConfig()
{
    if (!m_settings) {
        return false;
    }

    // 从INI文件读取参数
    m_settings->beginGroup("General");
    m_domain = m_settings->value("Domain").toString();
    m_port = m_settings->value("Port").toString();
    m_addr = m_settings->value("Addr").toString();
    m_path = m_settings->value("Path").toString();
    m_format = m_settings->value("Format").toString();
    m_settings->endGroup();

    return m_settings->status() == QSettings::NoError;
}
