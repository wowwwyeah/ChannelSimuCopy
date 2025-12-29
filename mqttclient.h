#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QObject>
#include <QTimer>
#include <QtMqtt/qmqttclient.h>
#include "mqttmessageparser.h"

#define EXAM_START_TOPIC            "0000020001/0000010001/00FF/0001"
#define EXAM_END_TOPIC              "0000020001/0000010001/00FF/0003"
#define CHANNEL_SIMU_PARAM          "0000020001/0000010001/00FF/0004"
#define CHANNEL_PARAM_REPORT        "0000010001/0000020001/00FF/0005"
#define CHANNEL_STATUS_REPORT       "0000010001/0000020001/00FF/0006"
#define CHANNEL_HERAT_BEAT          "0000010001/0000020001/00FF/0002"

class MqttClient : public QObject
{
    Q_OBJECT

public:
    explicit MqttClient(QObject *parent = nullptr);
    ~MqttClient();

    // 连接MQTT服务器
    void connectToBroker(const QString &hostname, quint16 port = 1883,
                         const QString &username = QString(),
                         const QString &password = QString());

    // 断开连接
    void disconnectFromBroker();

    // 订阅主题
    void subscribeToTopic(const QString &topic, quint8 qos = 0);

    // 取消订阅
    void unsubscribe(const QString &topic);

    // 发布消息
    void publish(const QString &topic, const QByteArray &message, quint8 qos = 0, bool retain = false);

    // 检查连接状态
    bool isConnected() const;

    // 设置消息解析器
    void setMessageParser(MqttMessageParser *parser);
    MqttMessageParser* messageParser() const;

    // 发送结构化消息
    void publishJson(const QString &topic, const QVariantMap &data, quint8 qos = 0, bool retain = false);
    void publishText(const QString &topic, const QString &text, quint8 qos = 0, bool retain = false);

signals:
    // 连接状态变化信号
    void connectionStatusChanged(bool connected);

    // 收到消息信号
    void messageReceived(const QString &topic, const QByteArray &message);
    void examStartMessageReceived(const QString &topic, const QByteArray &message);
    void examEndMessageReceived(const QString &topic, const QByteArray &message);
    void paramMessageReceived(const QString &topic, const QByteArray &message);

    // 错误信号
    void errorOccurred(const QString &error);

    void parsedMessageReceived(const QString &topic, const ParsedMessage &message);

public slots:
    // 连接状态变化槽函数
    void onStateChanged(QMqttClient::ClientState state);
    void onMessageReceived(const QByteArray &message, const QMqttTopicName &topic);
    void onHeartBeats();

private:
    void initConnections();
    void doSubscribe(const QString &topic, quint8 qos);
    void processPendingSubscriptions();
    // 缓存未执行的订阅请求：QList存储<主题, QoS>对
    QList<QPair<QString, quint8>> m_pendingSubscriptions;

    QMqttClient *m_client;
    QString errorString;
    bool m_connected;
    MqttMessageParser *m_parser;
    QTimer *m_timer;
};

#endif // MQTTCLIENT_H
