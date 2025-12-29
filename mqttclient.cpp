#include "mqttclient.h"
#include <QDebug>

MqttClient::MqttClient(QObject *parent) : QObject(parent)
    , m_parser(nullptr)
{
    m_client = new QMqttClient(this);

    // 创建默认解析器
    m_parser = new MqttMessageParser(this);

    m_timer = new QTimer();

    initConnections();

    connect(m_timer, &QTimer::timeout, this, &MqttClient::onHeartBeats);
    m_timer->start(5000);
}

MqttClient::~MqttClient()
{
    disconnectFromBroker();
}

void MqttClient::initConnections()
{
    // 连接状态变化信号
    connect(m_client, &QMqttClient::stateChanged, this, &MqttClient::onStateChanged);

    // 连接原始消息信号到解析槽
    connect(m_client, &QMqttClient::messageReceived, this, &MqttClient::onMessageReceived);

    // 错误处理
    connect(m_client, &QMqttClient::errorChanged, this, [this](QMqttClient::ClientError error) {
        switch (error) {
        case QMqttClient::NoError:
            errorString = "无错误";
            break;
        case QMqttClient::InvalidProtocolVersion:
            errorString = "无效的协议版本";
            break;
        case QMqttClient::IdRejected:
            errorString = "客户端ID被拒绝";
            break;
        case QMqttClient::ServerUnavailable:
            errorString = "服务器不可用";
            break;
        case QMqttClient::BadUsernameOrPassword:
            errorString = "用户名或密码错误";
            break;
        case QMqttClient::NotAuthorized:
            errorString = "未授权";
            break;
        // 其他错误类型...
        default:
            errorString = QString("未知错误 (代码: %1)").arg(static_cast<int>(error));
        }
        emit errorOccurred(errorString);
    });
}

void MqttClient::setMessageParser(MqttMessageParser *parser)
{
    if (m_parser && m_parser != parser) {
        m_parser->deleteLater();
    }
    m_parser = parser;
    if (m_parser && m_parser->parent() != this) {
        m_parser->setParent(this);
    }
}

MqttMessageParser* MqttClient::messageParser() const
{
    return m_parser;
}

void MqttClient::connectToBroker(const QString &hostname, quint16 port,
                                 const QString &username, const QString &password)
{
    m_client->setHostname(hostname);
    m_client->setPort(port);

    if (!username.isEmpty()) {
        m_client->setUsername(username);
    }

    if (!password.isEmpty()) {
        m_client->setPassword(password);
    }

    m_client->connectToHost();
}

void MqttClient::disconnectFromBroker()
{
    m_client->disconnectFromHost();
}

void MqttClient::subscribeToTopic(const QString &topic, quint8 qos)
{
    if (m_client->state() == QMqttClient::Connected) {
        // 已连接：直接订阅
        doSubscribe(topic, qos);
    } else {
        // 未连接：缓存到列表
        m_pendingSubscriptions.append({topic, qos});
        qDebug() << "客户端未连接，缓存订阅请求：" << topic << "（QoS：" << qos << "）";
    }
}

void MqttClient::unsubscribe(const QString &topic)
{
    m_client->unsubscribe(topic);
}

void MqttClient::publish(const QString &topic, const QByteArray &message, quint8 qos, bool retain)
{
    if (m_client->publish(topic, message, qos, retain) == -1) {
        emit errorOccurred("发布消息失败: " + topic);
    }
}

bool MqttClient::isConnected() const
{
    return m_client->state() == QMqttClient::Connected;
}

void MqttClient::onStateChanged(QMqttClient::ClientState state)
{
    m_connected = (state == QMqttClient::Connected);
    emit connectionStatusChanged(m_connected);

    switch (state) {
    case QMqttClient::Disconnected:
        emit errorOccurred("与MQTT服务器断开连接");
        break;
    case QMqttClient::Connecting:
        qDebug() << "正在连接到MQTT服务器...";
        break;
    case QMqttClient::Connected:
        qDebug() << "已成功连接到MQTT服务器";
        // 连接成功后，处理所有缓存的订阅请求
        processPendingSubscriptions();
        break;
    }
}

// 实际执行订阅的内部函数
void MqttClient::doSubscribe(const QString &topic, quint8 qos) {
    QMqttTopicFilter filter(topic);
    auto subscription = m_client->subscribe(filter, qos);
    if (!subscription) {
        qWarning() << "订阅失败：" << topic;
    } else {
        qDebug() << "发起订阅：" << topic << "（QoS：" << qos << "）";
    }
}

// 处理缓存的订阅请求
void MqttClient::processPendingSubscriptions() {
    if (m_pendingSubscriptions.isEmpty()) {
        return;
    }
    qDebug() << "开始处理缓存的" << m_pendingSubscriptions.size() << "个订阅请求";
    // 遍历缓存列表，逐个订阅
    for (const auto &item : m_pendingSubscriptions) {
        doSubscribe(item.first, item.second);
    }
    // 清空缓存（避免重复订阅）
    m_pendingSubscriptions.clear();
}

void MqttClient::onMessageReceived(const QByteArray &message, const QMqttTopicName &topicName)
{
    QString topic = topicName.name();
    qDebug() << "Topic:" << topic;
    if(topic == EXAM_START_TOPIC)
    {
        m_parser->parseExamStartJson(topic, message);
    }
    else if(topic == CHANNEL_SIMU_PARAM)
    {
        m_parser->parseChannelParamJson(topic, message);
        //emit paramMessageReceived(topic, message);
    }
    else if(topic == EXAM_END_TOPIC)
    {
        m_parser->parseExamEndJson(topic, message);
    }
    else
    {
        qDebug() << "未知消息";
    }
}

void MqttClient::onHeartBeats()
{
    if(!m_connected) return;

    QVariantMap HeartbeatParam;
    QByteArray payload;
    HeartbeatParam["state"] = 0x00;
    payload = m_parser->createJsonMessage(HeartbeatParam);
    publish(CHANNEL_HERAT_BEAT, payload, 0, false);

    QVariantMap ChannelParam;
    QByteArray payload1;
    ChannelParam["ExamID"] = 12345;
    ChannelParam["channelNum"] = 1;
    ChannelParam["BitErrorRate"] = "10%";
    ChannelParam["PacketLossRate"] = "5%";
    ChannelParam["DataRate"] = "100";
    payload1 = m_parser->createJsonMessage(ChannelParam);
    publish(CHANNEL_STATUS_REPORT, payload1, 0, false);

    QByteArray payload2;
    payload2 = m_parser->createChannelParamJsonMessage();
    publish(CHANNEL_PARAM_REPORT, payload2, 0, false);

}
