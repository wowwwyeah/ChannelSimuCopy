#include "configmanager.h"
#include "databasemanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::openDatabase(const QString &dbName)
{
    // 使用SQLite数据库驱动
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbName);

    if (!m_database.open()) {
        qWarning() << "Error: Failed to open database:" << m_database.lastError().text();
        return false;
    }

    qDebug() << "Database opened successfully!";
    return true;
}

bool DatabaseManager::createTable()
{
    QSqlQuery query;
    QString createTableQuery =
        "CREATE TABLE IF NOT EXISTS configs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "channelNum INTEGER NOT NULL, "
        "modelName TEXT NOT NULL, "
        "noisePower TEXT NOT NULL, "
        "signalAnt TEXT NOT NULL, "
        "comDistance TEXT NOT NULL, "
        "multipathNum INTEGER NOT NULL, "
        "filterNum INTEGER NOT NULL, "
        "MultiPathType JSON)";

    if (!query.exec(createTableQuery)) {
        qWarning() << "Error: Failed to create table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Table created successfully!";
    return true;
}
bool DatabaseManager::insertParaConfig(const ModelParaSetting &config)
{
    QSqlQuery query;
    query.prepare("INSERT INTO configs (channelNum, modelName, noisePower, signalAnt, comDistance, multipathNum, filterNum, multiPathType) "
                  "VALUES (:channelNum, :modelName, :noisePower, :signalAnt, :comDistance, :multipathNum, :filterNum, :multiPathType)");
    query.bindValue(":channelNum", config.channelNum);
    query.bindValue(":modelName", config.modelName);
    query.bindValue(":noisePower", config.noisePower);
    query.bindValue(":signalAnt", config.signalAnt);
    query.bindValue(":comDistance", config.comDistance);
    query.bindValue(":multipathNum", config.multipathNum);
    query.bindValue(":filterNum", config.filterNum);
    query.bindValue(":multiPathType", buildJsonArray(config.multipathType));

    if (!query.exec()) {
        qWarning() << "Error: Failed to insert data:" << query.lastError().text();
        return false;
    }

    qDebug() << "Data inserted successfully!";
    return true;
}

QVector<ModelParaSetting> DatabaseManager::getAllConfigs()
{
    QVector<ModelParaSetting> ParaConfigs;
    qDebug() << "Data get!";
    QSqlQuery query("SELECT id, channelNum, modelName, noisePower, signalAnt, comDistance, multipathNum, filterNum,"
                    "multiPathType FROM configs ORDER BY id");

    while (query.next()) {
        ModelParaSetting config;
        config.channelNum = query.value(1).toInt();
        config.modelName = query.value(2).toString();
        config.noisePower = query.value(3).toDouble();
        config.signalAnt = query.value(4).toDouble();
        config.comDistance = query.value(5).toDouble();
        config.multipathNum = query.value(6).toInt();
        config.filterNum = query.value(7).toInt();
        QString JsonArrayString = query.value(8).toString();
        config.multipathType = parseJsonArray(JsonArrayString);
        ParaConfigs.append(config);
        {
            QMutexLocker locker(&globalMutex);
            globalParaMap[config.modelName] = config;
        }
    }

    return ParaConfigs;
}

bool DatabaseManager::updateParaConfig(const QString &name, ModelParaSetting &config)
{
    QSqlQuery query;
    query.prepare("UPDATE configs SET channelNum = :channelNum, modelName = :modelName, noisePower = :noisePower, "
                  "signalAnt = :signalAnt, comDistance = :comDistance, multipathNum = :multipathNum, filterNum = :filterNum, "
                  "multipathType = :multipathType WHERE modelName = :modelName");

    if (!query.exec()) {
        qWarning() << "Error: Failed to update config:" << query.lastError().text();
        return false;
    }

    query.bindValue(":channelNum", config.channelNum);
    query.bindValue(":modelName", config.modelName);
    query.bindValue(":noisePower", config.noisePower);
    query.bindValue(":signalAnt", config.signalAnt);
    query.bindValue(":comDistance", config.comDistance);
    query.bindValue(":multipathNum", config.multipathNum);
    query.bindValue(":filterNum", config.filterNum);
    query.bindValue(":multiPathType", buildJsonArray(config.multipathType));
    qDebug() << "config update successfully!";
    return true;
}

bool DatabaseManager::deleteParaConfig(const QString &name)
{
    QSqlQuery query;
    query.prepare("DELETE FROM configs WHERE modelName = :modelName");
    query.bindValue(":modelName", name);

    if (!query.exec()) {
        qWarning() << "Error: Failed to delete config:" << query.lastError().text();
        return false;
    }

    qDebug() << "config deleted successfully!";
    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
        qDebug() << "Database closed successfully!";
    }
}

QList<MultiPathType> DatabaseManager::parseJsonArray(QString &jsonArrayString)
{
    QList<MultiPathType> multiParas;
    QJsonDocument doc = QJsonDocument::fromJson(jsonArrayString.toUtf8());

    if (doc.isArray()) {
        QJsonArray array = doc.array();

        for (const QJsonValue &value : array) {
            QJsonObject obj = value.toObject();
            MultiPathType config;
            config.pathNum = obj["pathNum"].toInt();
            config.relativDelay = obj["relativDelay"].toInt();
            config.antPower = obj["antPower"].toInt();
            config.freShift = obj["freShift"].toInt();
            config.freSpread = obj["freSpread"].toInt();
            multiParas.append(config);
        }
    }
    return multiParas;
}

QString DatabaseManager::buildJsonArray(QList<MultiPathType> multiParas)
{
    QJsonArray jsonArray;
    for(const MultiPathType &config : multiParas)
    {
        QJsonObject obj;
        obj["pathNum"] = config.pathNum;
        obj["relativDelay"] = config.relativDelay;
        obj["antPower"] = config.antPower;
        obj["freShift"] = config.freShift;
        obj["freSpread"] = config.freSpread;

        jsonArray.append(obj);
    }

    QJsonDocument doc(jsonArray);
    return doc.toJson();
}
