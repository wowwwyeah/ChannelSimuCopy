#ifndef MQTTMESSAGEPARSER_H
#define MQTTMESSAGEPARSER_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QVariant>
#include <QDateTime>
#include <QRegularExpression>

// 消息格式枚举
enum class MessageFormat {
    JSON,       // JSON格式
    XML,        // XML格式（需要额外处理）
    PlainText,  // 纯文本
    Binary,     // 二进制数据
    Unknown     // 未知格式
};

// 解析结果结构体
struct ParsedMessage {
    bool isValid;           // 是否有效
    MessageFormat format;   // 消息格式
    QVariant data;          // 解析后的数据
    QString errorString;    // 错误信息
    QDateTime timestamp;    // 时间戳
    QString topic;          // 主题

    ParsedMessage() : isValid(false), format(MessageFormat::Unknown) {}
};

class MqttMessageParser : public QObject
{
    Q_OBJECT

public:
    explicit MqttMessageParser(QObject *parent = nullptr);

    // 解析MQTT消息
    ParsedMessage parseMessage(const QString &topic, const QByteArray &payload);

    // 自动检测消息格式
    static MessageFormat detectFormat(const QByteArray &payload);

    // 各种格式的解析方法
    static QVariant parseJson(const QByteArray &payload, QString &error);
    static QVariant parseText(const QByteArray &payload);
    static QVariant parseBinary(const QByteArray &payload);

    void parseExamStartJson(const QString& topic, const QByteArray& payload);
    void parseExamEndJson(const QString& topic, const QByteArray& payload);
    void parseChannelParamJson(const QString& topic, const QByteArray& payload);

    // 工具方法：从主题中提取信息
    static QVariantMap extractTopicInfo(const QString &topic);

    // 创建标准化的发布消息
    static QByteArray createJsonMessage(const QVariantMap &data);
    static QByteArray createChannelParamJsonMessage();
    static QByteArray createTextMessage(const QString &text);
    static QByteArray createBinaryMessage(const QByteArray &data);

signals:
    void messageParsed(const QString &topic, const ParsedMessage &result);
    void parseError(const QString &topic, const QString &error);

private:
    // 主题模式匹配（用于智能解析）
    QList<QRegularExpression> m_topicPatterns;
};

#endif // MQTTMESSAGEPARSER_H
