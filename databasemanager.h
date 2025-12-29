#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "channelparaconifg.h"

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool openDatabase(const QString &dbName);
    bool createTable();
    bool insertParaConfig(const ModelParaSetting &config);
    QVector<ModelParaSetting> getAllConfigs();
    bool updateParaConfig(const QString &name, ModelParaSetting &config);
    bool deleteParaConfig(const QString &name);
    void closeDatabase();

private:
    QSqlDatabase m_database;

    QList<MultiPathType> parseJsonArray(QString &jsonArrayString);
    QString buildJsonArray(QList<MultiPathType> multiParas);
};

#endif // DATABASEMANAGER_H
