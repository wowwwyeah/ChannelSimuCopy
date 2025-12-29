#include "mqttmessageparser.h"
#include <QDebug>
#include <QRegularExpression>

MqttMessageParser::MqttMessageParser(QObject *parent) : QObject(parent)
{
    // 预定义一些常见的主题模式 - 使用 QRegularExpression
    m_topicPatterns << QRegularExpression("^sensor/(.+)/temperature$")
                    << QRegularExpression("^sensor/(.+)/humidity$")
                    << QRegularExpression("^device/(.+)/status$")
                    << QRegularExpression("^device/(.+)/control$")
                    << QRegularExpression("^(.+)/data$");
}

ParsedMessage MqttMessageParser::parseMessage(const QString &topic, const QByteArray &payload)
{
    ParsedMessage result;
    result.topic = topic;
    result.timestamp = QDateTime::currentDateTime();

    // 检测消息格式
    result.format = detectFormat(payload);

    switch (result.format) {
    case MessageFormat::JSON:
        result.data = parseJson(payload, result.errorString);
        result.isValid = !result.data.isNull();
        break;

    case MessageFormat::PlainText:
        result.data = parseText(payload);
        result.isValid = true;
        break;

    case MessageFormat::Binary:
        result.data = parseBinary(payload);
        result.isValid = true;
        break;

    case MessageFormat::Unknown:
        result.errorString = "未知的消息格式";
        result.isValid = false;
        break;
    }

    // 如果解析失败且是JSON格式，尝试作为文本处理
    if (!result.isValid && result.format == MessageFormat::JSON) {
        result.format = MessageFormat::PlainText;
        result.data = parseText(payload);
        result.isValid = true;
        result.errorString.clear();
    }

    emit messageParsed(topic, result);

    if (!result.isValid) {
        emit parseError(topic, result.errorString);
    }

    return result;
}

MessageFormat MqttMessageParser::detectFormat(const QByteArray &payload)
{
    if (payload.isEmpty()) {
        return MessageFormat::PlainText;
    }

    // 尝试检测JSON
    QByteArray trimmed = payload.trimmed();
    if ((trimmed.startsWith('{') && trimmed.endsWith('}')) ||
        (trimmed.startsWith('[') && trimmed.endsWith(']'))) {
        return MessageFormat::JSON;
    }

    // 检测文本（主要是可打印字符）
    bool isText = true;
    for (char c : payload) {
        if (c != '\n' && c != '\r' && c != '\t' &&
            (c < 32 || c > 126)) {
            isText = false;
            break;
        }
    }

    if (isText) {
        return MessageFormat::PlainText;
    }

    return MessageFormat::Binary;
}

QVariant MqttMessageParser::parseJson(const QByteArray &payload, QString &error)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        error = parseError.errorString();
        return QVariant();
    }

    if (doc.isObject()) {
        return doc.object().toVariantMap();
    } else if (doc.isArray()) {
        return doc.array().toVariantList();
    }

    error = "无效的JSON文档";
    return QVariant();
}

QVariant MqttMessageParser::parseText(const QByteArray &payload)
{
    return QString::fromUtf8(payload);
}

QVariant MqttMessageParser::parseBinary(const QByteArray &payload)
{
    return payload;
}

void MqttMessageParser::parseExamStartJson(const QString& topic, const QByteArray& payload)
{
    // 解析JSON
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(payload, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON解析失败：" << parseError.errorString();
        return;
    }

    if (!jsonDoc.isObject()) {
        qWarning() << "消息不是JSON对象";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    // 提取字段（根据实际需求选择需要的字段）
    uint64_t examID = jsonObj["ExamID"].toInt(); // 注意：大整数可能被转为字符串处理
    QString examName = jsonObj["ExamName"].toString();
    uint64_t userID = jsonObj["UserId"].toInt();
    QString username = jsonObj["username"].toString();
    QString deviceId = jsonObj["device_Id"].toString();
    QString deviceName = jsonObj["device_name"].toString();
    QString deviceModel = jsonObj["device_model"].toString();
    int op = jsonObj["op"].toInt(-1); // 默认值-1表示解析失败

    // 打印解析结果（或进行后续处理）
    qDebug() << "解析结果：";
    qDebug() << "ExamID:" << examID;
    qDebug() << "ExamName:" << examName;
    qDebug() << "UserId:" << userID;
    qDebug() << "username:" << username;
    qDebug() << "device_Id:" << deviceId;
    qDebug() << "device_name:" << deviceName;
    qDebug() << "device_model:" << deviceModel;
    qDebug() << "op:" << op;
}

void MqttMessageParser::parseExamEndJson(const QString& topic, const QByteArray& payload)
{
    // 解析JSON
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(payload, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON解析失败：" << parseError.errorString();
        return;
    }

    if (!jsonDoc.isObject()) {
        qWarning() << "消息不是JSON对象";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    // 提取字段（仅保留当前payload中的字段）
    uint64_t examID = jsonObj["ExamID"].toInt(); // 大整数转字符串处理
    QString username = jsonObj["username"].toString();
    int op = jsonObj["op"].toInt(-1); // 默认值-1表示解析失败

    // 输出解析结果
    qDebug() << "解析结果：";
    qDebug() << "ExamID:" << examID;
    qDebug() << "username:" << username;
    qDebug() << "op:" << op;

    // 可根据op的值进行业务处理（例如op=0可能代表某种特定操作）
    if (op == 0) {
        qDebug() << "检测到op=0，执行对应逻辑...";
        // 这里添加op=0时的处理代码
    }
}

void MqttMessageParser::parseChannelParamJson(const QString& topic, const QByteArray& payload)
{
    // 解析顶层JSON
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON解析失败：" << parseError.errorString();
        return;
    }
    if (!jsonDoc.isObject()) {
        qWarning() << "消息不是JSON对象";
        return;
    }
    QJsonObject rootObj = jsonDoc.object();

    // 提取顶层字段
    QString examID = rootObj["ExamID"].toString(); // 大整数转字符串
    qDebug() << "ExamID:" << examID;

    // 解析第一层嵌套：ModelParaSetting
    if (rootObj.contains("ModelParaSetting") && rootObj["ModelParaSetting"].isObject()) {
        QJsonObject modelObj = rootObj["ModelParaSetting"].toObject();

        // 提取ModelParaSetting的字段
        QString modelType = modelObj["modelType"].toString();
        QString modelName = modelObj["modelName"].toString();
        QString channelNum = modelObj["channelNum"].toString();
        QString channelID = modelObj["channelID"].toString();
        QString noisePower = modelObj["noisePower"].toString();
        QString signalAnt = modelObj["signalAnt"].toString();
        QString comDistance = modelObj["comDistance"].toString();
        QString filterNum = modelObj["filterNum"].toString();
        QString multipathNum = modelObj["multipathNum"].toString();

        // 打印第一层嵌套字段
        qDebug() << "\nModelParaSetting:";
        qDebug() << "modelType:" << modelType;
        qDebug() << "modelName:" << modelName;
        qDebug() << "channelNum:" << channelNum;
        qDebug() << "channelID:" << channelID;
        qDebug() << "noisePower:" << noisePower;
        qDebug() << "signalAnt:" << signalAnt;
        qDebug() << "comDistance:" << comDistance;
        qDebug() << "filterNum:" << filterNum;
        qDebug() << "multipathNum:" << multipathNum;

        // 解析第二层嵌套：MultiPath（在ModelParaSetting内部）
        if (modelObj.contains("MultiPath") && modelObj["MultiPath"].isObject()) {
            QJsonObject multiPathObj = modelObj["MultiPath"].toObject();

            // 提取MultiPath的字段
            QString pathNum = multiPathObj["pathNum"].toString();
            QString relativDelay = multiPathObj["relativDelay"].toString();
            QString antPower = multiPathObj["antPower"].toString();
            QString freShift = multiPathObj["freShift"].toString();
            QString freSpread = multiPathObj["freSpread"].toString();

            // 打印第二层嵌套字段
            qDebug() << "\nMultiPath:";
            qDebug() << "pathNum:" << pathNum;
            qDebug() << "relativDelay:" << relativDelay;
            qDebug() << "antPower:" << antPower;
            qDebug() << "freShift:" << freShift;
            qDebug() << "freSpread:" << freSpread;
        } else {
            qWarning() << "ModelParaSetting中不包含有效的MultiPath对象";
        }
    } else {
        qWarning() << "消息中不包含有效的ModelParaSetting对象";
    }
}

QVariantMap MqttMessageParser::extractTopicInfo(const QString &topic)
{
    QVariantMap info;
    info["full_topic"] = topic;

    // 分割主题路径
    QStringList parts = topic.split('/');
    info["path_parts"] = parts;
    info["depth"] = parts.size();

    // 尝试匹配预定义模式 - 使用 QRegularExpression
    MqttMessageParser parser;
    for (const QRegularExpression &pattern : parser.m_topicPatterns) {
        QRegularExpressionMatch match = pattern.match(topic);
        if (match.hasMatch()) {
            info["pattern"] = pattern.pattern();
            if (match.capturedTexts().size() > 1) {
                info["device_id"] = match.captured(1);
            }
            break;
        }
    }

    return info;
}

QByteArray MqttMessageParser::createJsonMessage(const QVariantMap &data)
{
    QJsonObject jsonObj = QJsonObject::fromVariantMap(data);
    QJsonDocument doc(jsonObj);
    return doc.toJson(QJsonDocument::Compact);
}

QByteArray MqttMessageParser::createChannelParamJsonMessage()
{
    // 构建根对象
    QJsonObject rootObj;
    rootObj["ExamID"] = QString::number(1943573142583222273);

    // 设备列表数组（可根据实际设备数量动态生成）
    QJsonArray channelConnectArray;
    channelConnectArray.append(1);
    channelConnectArray.append(3);
    channelConnectArray.append(5);
    rootObj["channelConnect"] = channelConnectArray;

    // --------------------------
    // 核心：循环创建多个信道
    // --------------------------
    QJsonArray modelParaArray; // 存储所有信道的数组
    int channelCount = 2; // 总信道数量（可从业务逻辑获取，例如用户配置的信道数）

    for (int channelIdx = 0; channelIdx < channelCount; ++channelIdx) {
        // 1. 创建当前信道的基础参数对象
        QJsonObject modelObj;
        // 信道类型/名称可根据索引动态区分（示例逻辑）
        modelObj["modelType"] = QString::number(3 + channelIdx); // 3,4,5...
        modelObj["modelName"] = (channelIdx == 0) ? "海洋场景" : "城市场景"; // 不同信道不同场景
        modelObj["channelNum"] = QString::number(2 + channelIdx); // 2,3,4...
        modelObj["channelID"] = QString("channel-%1-id").arg(channelIdx + 1); // 动态生成ID
        // 其他基础参数（示例值，实际可从配置获取）
        modelObj["noisePower"] = QString::number(2000 - channelIdx * 500);
        modelObj["signalAnt"] = QString::number(1000 - channelIdx * 200);
        modelObj["comDistance"] = QString::number(3000 + channelIdx * 2000);
        modelObj["filterNum"] = QString::number(3 - channelIdx);

        // 2. 为当前信道动态生成多径（嵌套循环）
        int multipathCount = 2 + channelIdx; // 每个信道的多径数量（示例：2,3,4...）
        modelObj["multipathNum"] = QString::number(multipathCount); // 多径数量与数组长度一致

        QJsonArray multiPathArray;
        for (int pathIdx = 0; pathIdx < multipathCount; ++pathIdx) {
            QJsonObject pathObj;
            pathObj["pathNum"] = QString::number(pathIdx + 1); // 多径编号从1开始
            // 多径参数（示例：根据信道和多径索引动态生成）
            pathObj["relativDelay"] = QString::number(2 + pathIdx * 2 + channelIdx * 1);
            pathObj["antPower"] = QString::number(2000 - pathIdx * 200 - channelIdx * 300);
            pathObj["freShift"] = QString::number(20 - pathIdx * 5 - channelIdx * 2);
            pathObj["freSpread"] = QString::number(30 - pathIdx * 5 - channelIdx * 3);
            multiPathArray.append(pathObj);
        }
        modelObj["MultiPath"] = multiPathArray; // 将多径数组添加到当前信道

        // 3. 将当前信道对象添加到信道数组
        modelParaArray.append(modelObj);
    }

    // 将所有信道组成的数组添加到根对象
    rootObj["ModelParaSetting"] = modelParaArray;

    // 转换为JSON字符串
    QJsonDocument jsonDoc(rootObj);
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Indented);

    //qDebug() << "循环创建的完整JSON:\n" << jsonData;

    return jsonData;
}

QByteArray MqttMessageParser::createTextMessage(const QString &text)
{
    return text.toUtf8();
}

QByteArray MqttMessageParser::createBinaryMessage(const QByteArray &data)
{
    return data;
}
