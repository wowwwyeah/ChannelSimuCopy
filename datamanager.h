#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QVariantMap>

class DataManager : public QObject
{
    Q_OBJECT
public:
public:
    static DataManager* instance();

    void setPageData(const QString &pageName, const QVariantMap &data);
    QVariantMap getAllData() const;

    void clearData();

signals:
    void allDataReady(const QVariantMap &data);

private:
    explicit DataManager(QObject *parent = nullptr);
    static DataManager *m_instance;
    QVariantMap m_pageData;
};

#endif // DATAMANAGER_H
