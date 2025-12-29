#ifndef SYSTEMSETTING_H
#define SYSTEMSETTING_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include "settingmanager.h"
#include "mqttclient.h"
#include "mqttmessageparser.h"

class SystemSetting : public QWidget
{
    Q_OBJECT
public:
    explicit SystemSetting(QWidget *parent = nullptr);
    ~SystemSetting();

    QString getMqttDomain() const;
    uint16_t getMqttPort() const;
    QString getDeviceAddr() const;
    QString getExportPath() const;
    QString getExportFormat() const;

signals:

private slots:
    void onBrowseExportPath();
    void onFormatChanged(int index);
    void onSaveSettings();
    void onRestoreDefaults();
    void onApplySettings();

    void onMQTTExamStartMessageReceived(const QString &topic, const QByteArray& payload);
    void onMQTTExamEndMessageReceived(const QString &topic, const QByteArray& payload);
    void onMQTTParamMessageReceived(const QString &topic, const QByteArray& payload);

private:
    void initUI();
    void initMqttClient();
    void loadSettings();
    void saveSettings();
    void applyFormat(const QString &format);

    QLineEdit *networkDomainEdit;
    QLineEdit *networkPortEdit;
    QLineEdit *networkAddrEdit;
    QLineEdit *exportPathEdit;
    QComboBox *exportFormatCombo;

    SettingManager *m_settingMgr;

    QString m_mqttDomain;   //MQTT地址
    QString m_mqttPort;     //MQTT端口
    QString m_deviceAddr;   //设备地址
    QString m_filePath;     //导出路径
    QString m_fileFormat;   //导出格式


    MqttClient *m_mqttClient;
    MqttMessageParser *m_mqttParser;
};

#endif // SYSTEMSETTING_H
