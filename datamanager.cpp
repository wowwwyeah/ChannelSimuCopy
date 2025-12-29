#include "datamanager.h"

DataManager* DataManager::m_instance = nullptr;

DataManager* DataManager::instance()
{
    if (!m_instance) {
        m_instance = new DataManager();
    }
    return m_instance;
}

DataManager::DataManager(QObject *parent) : QObject(parent)
{
}

void DataManager::setPageData(const QString &pageName, const QVariantMap &data)
{
    m_pageData[pageName] = data;
}

QVariantMap DataManager::getAllData() const
{
    return m_pageData;
}

void DataManager::clearData()
{
    m_pageData.clear();
}
