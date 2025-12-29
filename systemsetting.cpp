#include "systemsetting.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

SystemSetting::SystemSetting(QWidget *parent)
    : QWidget{parent}
    , m_mqttClient(new MqttClient(this))
    , m_mqttParser(new MqttMessageParser(this))
{
    initUI();
    loadSettings();
    initMqttClient();
}

SystemSetting::~SystemSetting()
{
    saveSettings();
}

void SystemSetting::initUI()
{
    // 设置背景色
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#336666"));
    setPalette(pal);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("系统设置");
    title->setAlignment(Qt::AlignmentFlag::AlignCenter);
    title->setStyleSheet("color: #2196F3; font-family: 微软雅黑; font-size: 22px; font-weight: bold; margin: 12px;");  // 18px -> 22px, 8px -> 12px
    layout->addWidget(title);

    // 中央内容区域 - 使用垂直布局
    QVBoxLayout *contentLayout = new QVBoxLayout();

    // 网络设置组
    QGroupBox *networkGroup = new QGroupBox(tr("网络设置"));
    networkGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 18px; }");  // 添加字体大小
    networkGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QGridLayout *networkLayout = new QGridLayout(networkGroup);
    networkLayout->setVerticalSpacing(12);  // 增加垂直间距
    networkLayout->setHorizontalSpacing(12); // 增加水平间距

    // 第一行：MQTT地址和端口
    QLabel *mqttAddressLabel = new QLabel("MQTT地址:");
    mqttAddressLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mqttAddressLabel->setStyleSheet("font-size: 16px;");  // 添加字体大小

    networkDomainEdit = new QLineEdit;
    networkDomainEdit->setPlaceholderText("例如: mqtt.broker.com");
    networkDomainEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    networkDomainEdit->setMinimumHeight(36);  // 设置最小高度
    networkDomainEdit->setStyleSheet("font-size: 14px; padding: 8px 12px;");  // 添加样式

    QLabel *mqttPortLabel = new QLabel("端口:");
    mqttPortLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mqttPortLabel->setStyleSheet("font-size: 16px;");  // 添加字体大小

    networkPortEdit = new QLineEdit;
    networkPortEdit->setPlaceholderText("例如: 1883");
    networkPortEdit->setMaximumWidth(120);  // 100px -> 120px
    networkPortEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    networkPortEdit->setMinimumHeight(36);  // 设置最小高度
    networkPortEdit->setStyleSheet("font-size: 14px; padding: 8px 12px;");  // 添加样式

    // 将地址和端口放在同一行
    networkLayout->addWidget(mqttAddressLabel, 0, 0);
    networkLayout->addWidget(networkDomainEdit, 0, 1);
    networkLayout->addWidget(mqttPortLabel, 0, 2);
    networkLayout->addWidget(networkPortEdit, 0, 3);

    // 第二行：设备地址
    QLabel *deviceAddressLabel = new QLabel("设备地址:");
    deviceAddressLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    deviceAddressLabel->setStyleSheet("font-size: 16px;");  // 添加字体大小

    networkAddrEdit = new QLineEdit;
    networkAddrEdit->setPlaceholderText("例如: 192.168.1.1");
    networkAddrEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    networkAddrEdit->setMinimumHeight(36);  // 设置最小高度
    networkAddrEdit->setStyleSheet("font-size: 14px; padding: 8px 12px;");  // 添加样式

    networkLayout->addWidget(deviceAddressLabel, 1, 0);
    networkLayout->addWidget(networkAddrEdit, 1, 1, 1, 3); // 跨3列

    // 设置列比例
    networkLayout->setColumnStretch(0, 0);  // 标签列不拉伸
    networkLayout->setColumnStretch(1, 3);  // 地址编辑框主要拉伸
    networkLayout->setColumnStretch(2, 0);  // 端口标签不拉伸
    networkLayout->setColumnStretch(3, 1);  // 端口输入框次要拉伸

    // 设置行比例
    networkLayout->setRowStretch(0, 1);     // 第一行可拉伸
    networkLayout->setRowStretch(1, 1);     // 第二行可拉伸
    networkLayout->setRowStretch(2, 1);     // 预留行可拉伸

    contentLayout->addWidget(networkGroup);

    // 导出设置组
    QGroupBox *exportGroup = new QGroupBox(tr("导出设置"));
    exportGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 18px; }");  // 添加字体大小
    exportGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QFormLayout *exportLayout = new QFormLayout(exportGroup);
    exportLayout->setVerticalSpacing(12);  // 增加垂直间距
    exportLayout->setHorizontalSpacing(12); // 增加水平间距

    QLabel *exportPathLabel = new QLabel("导出路径:");
    exportPathLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    exportPathLabel->setStyleSheet("font-size: 16px;");  // 添加字体大小

    QHBoxLayout *pathLayout = new QHBoxLayout;
    exportPathEdit = new QLineEdit;
    exportPathEdit->setPlaceholderText(tr("请选择导出文件保存路径"));
    exportPathEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    exportPathEdit->setMinimumHeight(36);  // 设置最小高度
    exportPathEdit->setStyleSheet("font-size: 14px; padding: 8px 12px;");  // 添加样式

    QPushButton *browseBtn = new QPushButton(tr("浏览..."));
    browseBtn->setObjectName("browseBtn");
    browseBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    browseBtn->setMinimumHeight(36);  // 设置最小高度
    browseBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #448888;"
        "   color: #CCEEEE;"
        "   border: none;"
        "   padding: 8px 16px;"
        "   border-radius: 6px;"
        "   font-family: 'Microsoft YaHei';"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #559999;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #66AAAA;"
        "}"
        );

    pathLayout->addWidget(exportPathEdit);
    pathLayout->addWidget(browseBtn);

    exportLayout->addRow(exportPathLabel, pathLayout);

    // 导出格式
    QLabel *formatLabel = new QLabel(tr("默认格式:"));
    formatLabel->setStyleSheet("font-size: 16px;");  // 添加字体大小
    formatLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    exportFormatCombo = new QComboBox;
    exportFormatCombo->addItem("CSV文件", "CSV");
    exportFormatCombo->addItem("JSON文件", "JSON");
    exportFormatCombo->addItem("XML文件", "XML");
    exportFormatCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    exportFormatCombo->setMinimumHeight(36);  // 设置最小高度
    exportFormatCombo->setStyleSheet("font-size: 14px; padding: 8px 12px;");  // 添加样式

    exportLayout->addRow(formatLabel, exportFormatCombo);

    // 设置表单布局的字段增长策略
    exportLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    contentLayout->addWidget(exportGroup);

    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(16);  // 10px -> 16px

    QPushButton *saveBtn = new QPushButton(tr("💾 保存设置"));
    saveBtn->setObjectName("saveSettingsBtn");
    saveBtn->setStyleSheet(
        "QPushButton {"
        "   padding: 12px 24px;"  // 8px -> 12px 24px
        "   background-color: #28a745;"
        "   color: white;"
        "   border-radius: 8px;"  // 添加圆角
        "   font-family: 'Microsoft YaHei';"
        "   font-size: 16px;"  // 添加字体大小
        "   font-weight: bold;"
        "   min-width: 120px;"  // 添加最小宽度
        "   min-height: 44px;"  // 添加最小高度
        "}"
        "QPushButton:hover {"
        "   background-color: #34ce57;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1e7e34;"
        "}"
        );
    saveBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QPushButton *defaultsBtn = new QPushButton(tr("🔄 恢复默认"));
    defaultsBtn->setObjectName("restoreDefaultsBtn");
    defaultsBtn->setStyleSheet(
        "QPushButton {"
        "   padding: 12px 24px;"
        "   background-color: #6c757d;"
        "   color: white;"
        "   border-radius: 8px;"
        "   font-family: 'Microsoft YaHei';"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   min-width: 120px;"
        "   min-height: 44px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #5a6268;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #545b62;"
        "}"
        );
    defaultsBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QPushButton *applyBtn = new QPushButton(tr("✅ 立即应用"));
    applyBtn->setObjectName("applySettingsBtn");
    applyBtn->setStyleSheet(
        "QPushButton {"
        "   padding: 12px 24px;"
        "   background-color: #007bff;"
        "   color: white;"
        "   border-radius: 8px;"
        "   font-family: 'Microsoft YaHei';"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   min-width: 120px;"
        "   min-height: 44px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #0069d9;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #0062cc;"
        "}"
        );
    applyBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(defaultsBtn);
    buttonLayout->addWidget(applyBtn);
    buttonLayout->addStretch();

    contentLayout->addLayout(buttonLayout);
    contentLayout->addStretch();

    // 设置内容布局的拉伸因子
    contentLayout->setStretchFactor(networkGroup, 2);    // 网络设置组占2份空间
    contentLayout->setStretchFactor(exportGroup, 2);     // 导出设置组占2份空间
    contentLayout->setStretchFactor(buttonLayout, 0);    // 按钮区域不拉伸

    contentLayout->setStretch(0, 0);           // 标题不拉伸 (索引0)
    contentLayout->setStretch(1, 2);           // 网络设置组拉伸 (索引1)
    contentLayout->setStretch(2, 2);           // 导出设置组拉伸 (索引2)
    contentLayout->setStretch(3, 0);           // 按钮布局不拉伸 (索引3)

    layout->addLayout(contentLayout);

    // 设置布局拉伸因子
    layout->setStretchFactor(title, 0);              // 标题不拉伸
    layout->setStretchFactor(contentLayout, 1);      // 主要内容区域拉伸

    setLayout(layout);

    // 连接设置页面信号槽
    connect(browseBtn, &QPushButton::clicked, this, &SystemSetting::onBrowseExportPath);
    connect(saveBtn, &QPushButton::clicked, this, &SystemSetting::onSaveSettings);
    connect(defaultsBtn, &QPushButton::clicked, this, &SystemSetting::onRestoreDefaults);
    connect(applyBtn, &QPushButton::clicked, this, &SystemSetting::onApplySettings);
    connect(exportFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SystemSetting::onFormatChanged);
    connect(applyBtn, &QPushButton::clicked, [this]() {
        saveSettings();
        QMessageBox::information(this, tr("设置已应用"), tr("设置已成功应用！"));
    });

}

void SystemSetting::initMqttClient()
{
    connect(m_mqttClient, &MqttClient::connectionStatusChanged, [](bool connected) {
        qDebug() << "连接状态:" << (connected ? "已连接" : "已断开");
    });

    connect(m_mqttClient, &MqttClient::examStartMessageReceived, this, &SystemSetting::onMQTTExamStartMessageReceived);
    connect(m_mqttClient, &MqttClient::examEndMessageReceived, this, &SystemSetting::onMQTTExamEndMessageReceived);
    connect(m_mqttClient, &MqttClient::paramMessageReceived, this, &SystemSetting::onMQTTParamMessageReceived);

    connect(m_mqttClient, &MqttClient::errorOccurred, [](const QString &error) {
        qDebug() << "错误:" << error;
    });

    // 连接到MQTT服务器
    m_mqttClient->connectToBroker("10.43.15.178", 1883); // 使用公共MQTT服务器

    // 订阅消息
    m_mqttClient->subscribeToTopic(EXAM_START_TOPIC);
    m_mqttClient->subscribeToTopic(EXAM_END_TOPIC);
    m_mqttClient->subscribeToTopic(CHANNEL_SIMU_PARAM);
}

void SystemSetting::onFormatChanged(int index)
{
    if (index < 0) return;

    QString format = exportFormatCombo->currentData().toString();
    QString formatName = exportFormatCombo->currentText();

}

void SystemSetting::onBrowseExportPath()
{

    // 获取当前路径
    QString currentPath = exportPathEdit->text();
    if (currentPath.isEmpty()) {
        currentPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
        + "/Channel/Exports";
    }

    // 确保路径存在
    QDir dir(currentPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 使用静态函数，避免重复触发问题
    QString selectedDir = QFileDialog::getExistingDirectory(
        this,
        tr("选择导出路径"),
        currentPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    // 只有当用户选择了有效目录时才更新
    if (!selectedDir.isEmpty()) {
        exportPathEdit->setText(selectedDir);

        // 立即保存设置

    }
}

void SystemSetting::onSaveSettings()
{
    saveSettings();
    QMessageBox::information(this, tr("保存成功"), tr("设置已保存成功！"));
}

void SystemSetting::onRestoreDefaults()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("恢复默认设置"),
        tr("确定要恢复所有设置为默认值吗？"),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {

        // 恢复导出路径默认值
        QString defaultExportPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                                    + "/Channel/Exports";
        exportPathEdit->setText(defaultExportPath);

        // 创建默认导出目录
        QDir dir(defaultExportPath);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        QMessageBox::information(this, tr("恢复成功"), tr("已恢复所有设置为默认值！"));
    }
}

void SystemSetting::onApplySettings()
{
    if(m_mqttDomain != networkDomainEdit->text().trimmed() || m_mqttPort != networkPortEdit->text().trimmed())
    {
        m_mqttDomain = networkDomainEdit->text().trimmed();
        m_mqttPort = networkPortEdit->text().trimmed();

        // 先断开连接
        m_mqttClient->disconnectFromBroker();

        m_mqttClient->connectToBroker(m_mqttDomain, m_mqttPort.toInt());
    }

    if(m_deviceAddr != networkAddrEdit->text().trimmed())
    {
        m_deviceAddr = networkAddrEdit->text().trimmed();
        // 设置设备IP
    }

}

void SystemSetting::loadSettings()
{
    m_settingMgr = new SettingManager("ChannelSettings.ini", this);
    // 加载配置
    if (m_settingMgr->loadConfig()) {
        qDebug() << "配置加载成功";
        m_mqttDomain = m_settingMgr->getDomain();
        networkDomainEdit->setText(m_mqttDomain);
        m_mqttPort = m_settingMgr->getPort();
        networkPortEdit->setText(m_mqttPort);
        m_deviceAddr = m_settingMgr->getDeviceAddr();
        networkAddrEdit->setText(m_deviceAddr);
        m_filePath = m_settingMgr->getPath();
        exportPathEdit->setText(m_filePath);
        m_fileFormat = m_settingMgr->getFormat();
        applyFormat(m_fileFormat);
    } else {
        qDebug() << "配置加载失败";
    }

}

void SystemSetting::saveSettings()
{
    // 保存网络设置
    if (networkDomainEdit) {
        m_settingMgr->setDomain(networkDomainEdit->text());
    }

    if (networkPortEdit) {
        m_settingMgr->setPort(networkPortEdit->text());
    }

    if (networkAddrEdit) {
        m_settingMgr->setDeviceAddr(networkAddrEdit->text());
    }

    // 保存导出路径
    if (exportPathEdit) {
        m_settingMgr->setPath(exportPathEdit->text());
    }

    // 保存导出格式
    if (exportFormatCombo) {
        QString format = exportFormatCombo->currentData().toString();
        m_settingMgr->setFormat(format);
    }

    m_settingMgr->saveConfig();

    m_mqttDomain = m_settingMgr->getDomain();
    m_mqttPort = m_settingMgr->getPort();
    m_deviceAddr = m_settingMgr->getDeviceAddr();
    m_filePath = m_settingMgr->getPath();
    m_fileFormat = m_settingMgr->getFormat();
}

void SystemSetting::applyFormat(const QString &format)
{
    // 设置格式
    if (exportFormatCombo) {
        for (int i = 0; i < exportFormatCombo->count(); ++i) {
            if (exportFormatCombo->itemData(i).toString() == format) {
                exportFormatCombo->setCurrentIndex(i);
                break;
            }
        }
    }
}

QString SystemSetting::getMqttDomain() const
{
    return m_mqttDomain;
}

uint16_t SystemSetting::getMqttPort() const
{
    return m_mqttPort.toInt();
}

QString SystemSetting::getDeviceAddr() const
{
    return m_deviceAddr;
}

QString SystemSetting::getExportPath() const
{
    return m_filePath;
}

QString SystemSetting::getExportFormat() const
{
    return m_fileFormat;
}

void SystemSetting::onMQTTExamStartMessageReceived(const QString& topic, const QByteArray& payload)
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
    QString examID = jsonObj["ExamID"].toString(); // 注意：大整数可能被转为字符串处理
    QString examName = jsonObj["ExamName"].toString();
    QString userID = jsonObj["UserId"].toString();
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

void SystemSetting::onMQTTExamEndMessageReceived(const QString& topic, const QByteArray& payload)
{

}

void SystemSetting::onMQTTParamMessageReceived(const QString& topic, const QByteArray& payload)
{

}

