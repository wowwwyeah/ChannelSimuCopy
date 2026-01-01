#include "channelbasicpara.h"
#include <QFrame>
#include <cmath>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPalette>
#include <QDebug>
ChannelBasicPara::ChannelBasicPara(QWidget *parent)
    : QWidget(parent)
    , m_attenuationPower(0)
    , m_communicationDistance(10.0)  // 默认值改为10.0（匹配重置逻辑）
{
    setupUI();
}

void ChannelBasicPara::setupUI()
{
    // 设置背景色
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#336666"));
    setPalette(pal);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 设置样式表
    this->setStyleSheet(R"(
        ChannelBasicPara {
            background-color: #336666;
        }

        QGroupBox {
            color: #CCEEEE;
            font-family: "Microsoft YaHei";
            font-size: 14px;
            font-weight: bold;
            border: 2px solid #559999;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 2px;
            background-color: #225555;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 0 8px;
            background-color: #448888;
            border-radius: 4px;
        }

        QLabel {
            color: #99CCCC;
            font-family: "Microsoft YaHei";
            font-size: 12px;
            font-weight: bold;
        }

        QLabel#valueLabel {
            color: #CCEEEE;
            font-size: 16px;
            font-weight: bold;
            background-color: #224444;
            border-radius: 4px;
            padding: 4px 8px;
            min-width: 60px;
            qproperty-alignment: AlignCenter;
        }

        /* 通信距离滚动条样式 - 绿色渐变 */
        QSlider#distanceSlider::groove:horizontal {
            border: 1px solid #5AA469;
            height: 8px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3D7248, stop:0.5 #5AA469, stop:1 #7CD68B);
            border-radius: 4px;
        }

        QSlider#distanceSlider::handle:horizontal {
            background: qradialgradient(cx:0.5, cy:0.5, radius:0.8,
                stop:0 #7CD68B, stop:0.8 #5AA469, stop:1 #3D7248);
            border: 2px solid #5AA469;
            width: 20px;
            margin: -8px 0;
            border-radius: 10px;
        }

        QSlider#distanceSlider::handle:horizontal:hover {
            background: qradialgradient(cx:0.5, cy:0.5, radius:0.8,
                stop:0 #9EF5AD, stop:0.8 #7CD68B, stop:1 #5AA469);
            border: 2px solid #7CD68B;
        }

        QSlider#distanceSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3D7248, stop:0.5 #5AA469, stop:1 #7CD68B);
            border: 1px solid #5AA469;
            border-radius: 4px;
        }



        /* 衰减功率滚动条样式 - 橙色渐变 */
        QSlider#attenuationPowerSlider::groove:horizontal {
            border: 1px solid #D4A15F;
            height: 8px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #8B6B3D, stop:0.5 #D4A15F, stop:1 #FFD700);
            border-radius: 4px;
        }

        QSlider#attenuationPowerSlider::handle:horizontal {
            background: qradialgradient(cx:0.5, cy:0.5, radius:0.8,
                stop:0 #FFD700, stop:0.8 #D4A15F, stop:1 #8B6B3D);
            border: 2px solid #D4A15F;
            width: 20px;
            margin: -8px 0;
            border-radius: 10px;
        }

        QSlider#attenuationPowerSlider::handle:horizontal:hover {
            background: qradialgradient(cx:0.5, cy:0.5, radius:0.8,
                stop:0 #FFE55C, stop:0.8 #FFD700, stop:1 #D4A15F);
            border: 2px solid #FFD700;
        }

        QSlider#attenuationPowerSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #8B6B3D, stop:0.5 #D4A15F, stop:1 #FFD700);
            border: 1px solid #D4A15F;
            border-radius: 4px;
        }

        QDial {
            background-color: #224444;
            border: 1px solid #559999;
            border-radius: 4px;
        }

        QDial::groove {
            background: qconicalgradient(cx:0.5, cy:0.5, angle:0, stop:0 #225555, stop:1 #448888);
        }

        QDial::handle {
            background: #99CCCC;
            border: 2px solid #559999;
            border-radius: 8px;
            width: 16px;
            height: 16px;
        }

        QDoubleSpinBox {
            padding: 6px;
            border: 1px solid #559999;
            border-radius: 4px;
            background-color: #224444;
            color: #CCEEEE;
            font-family: "Microsoft YaHei";
            font-size: 12px;
            min-width: 80px;
        }

        QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
            background-color: #448888;
            border: 1px solid #559999;
            border-radius: 2px;
            width: 16px;
        }

        QPushButton {
            background-color: #448888;
            color: #CCEEEE;
            border: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-family: "Microsoft YaHei";
            font-size: 14px;
            font-weight: bold;
            min-width: 100px;
        }

        QPushButton:hover {
            background-color: #559999;
        }

        QPushButton:pressed {
            background-color: #66AAAA;
        }
    )");

    // 创建功率参数组
    createPowerParametersGroup();
    mainLayout->addWidget(m_powerGroup);

    // 添加按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_applyButton = new QPushButton("应用设置");
    m_resetButton = new QPushButton("重置参数");

    connect(m_applyButton, &QPushButton::clicked, this, &ChannelBasicPara::onApplyClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &ChannelBasicPara::onResetClicked);

    //buttonLayout->addStretch();
    //buttonLayout->addWidget(m_resetButton);
    //buttonLayout->addWidget(m_applyButton);
    //mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

void ChannelBasicPara::createPowerParametersGroup()
{
    m_powerGroup = new QGroupBox("功率参数设置");
    QGridLayout *gridLayout = new QGridLayout(m_powerGroup);

    // === 第一行：标题 ===
    QLabel *distanceTitle = new QLabel("通信距离 (km)");
    QLabel *attenuationTitle = new QLabel("衰减功率 (dB)");
    gridLayout->addWidget(distanceTitle, 0, 0, Qt::AlignCenter);
    gridLayout->addWidget(attenuationTitle, 0, 2, Qt::AlignCenter);

    // === 第二行：当前值显示 ===
    QLabel *distanceCurrentLabel = new QLabel("当前值:");
    m_distanceValueLabel = new QLabel(QString::number(m_communicationDistance, 'f', 1));
    m_distanceValueLabel->setObjectName("valueLabel");

    QLabel *attenuationCurrentLabel = new QLabel("当前值:");
    m_attenuationPowerValueLabel = new QLabel(QString::number(m_attenuationPower, 'f', 1));
    m_attenuationPowerValueLabel->setObjectName("valueLabel");

    QHBoxLayout *distanceValueLayout = new QHBoxLayout();
    distanceValueLayout->addWidget(distanceCurrentLabel);
    distanceValueLayout->addWidget(m_distanceValueLabel);
    distanceValueLayout->addStretch();

    QHBoxLayout *attenuationValueLayout = new QHBoxLayout();
    attenuationValueLayout->addWidget(attenuationCurrentLabel);
    attenuationValueLayout->addWidget(m_attenuationPowerValueLabel);
    attenuationValueLayout->addStretch();

    gridLayout->addLayout(distanceValueLayout, 1, 0);
    gridLayout->addLayout(attenuationValueLayout, 1, 2);

    // === 第三行：滚动条 ===
    // 通信距离滚动条
    // DISTANCE_MIN=calculateDistanceFromAttenuation(60);
    // DISTANCE_MAX=calculateDistanceFromAttenuation(ATTENUATION_POWER_MIN);

    QHBoxLayout *distanceSliderLayout = new QHBoxLayout();
    QLabel *distanceMinLabel = new QLabel(QString::number(DISTANCE_MIN, 'f', 0));
    QLabel *distanceMaxLabel = new QLabel(QString::number(DISTANCE_MAX, 'f', 0));
    distanceMinLabel->setStyleSheet("color: #7CD68B;");
    distanceMaxLabel->setStyleSheet("color: #7CD68B;");

    m_distanceSlider = new QSlider(Qt::Horizontal);
    m_distanceSlider->setObjectName("distanceSlider");
    m_distanceSlider->setRange(DISTANCE_MIN * 10, DISTANCE_MAX * 10);
    m_distanceSlider->setValue(m_communicationDistance * 10);
    m_distanceSlider->setTickPosition(QSlider::TicksBelow);
    m_distanceSlider->setTickInterval(100);
    connect(m_distanceSlider, &QSlider::valueChanged, this, &ChannelBasicPara::onDistanceSliderChanged);

    distanceSliderLayout->addWidget(distanceMinLabel);
    distanceSliderLayout->addWidget(m_distanceSlider);
    distanceSliderLayout->addWidget(distanceMaxLabel);

    // 衰减功率滚动条
    QHBoxLayout *attenuationSliderLayout = new QHBoxLayout();
    QLabel *attenuationMinLabel = new QLabel(QString::number(ATTENUATION_POWER_MIN, 'f', 0));
    QLabel *attenuationMaxLabel = new QLabel(QString::number(ATTENUATION_POWER_MAX, 'f', 0));
    attenuationMinLabel->setStyleSheet("color: #FFD700;");
    attenuationMaxLabel->setStyleSheet("color: #FFD700;");

    m_attenuationPowerSlider = new QSlider(Qt::Horizontal);
    m_attenuationPowerSlider->setObjectName("attenuationPowerSlider");
    m_attenuationPowerSlider->setRange(ATTENUATION_POWER_MIN * 10, ATTENUATION_POWER_MAX * 10);
    m_attenuationPowerSlider->setValue(m_attenuationPower * 10);
    m_attenuationPowerSlider->setTickPosition(QSlider::TicksBelow);
    m_attenuationPowerSlider->setTickInterval(100);
    connect(m_attenuationPowerSlider, &QSlider::valueChanged, this, &ChannelBasicPara::onAttenuationPowerSliderChanged);

    attenuationSliderLayout->addWidget(attenuationMinLabel);
    attenuationSliderLayout->addWidget(m_attenuationPowerSlider);
    attenuationSliderLayout->addWidget(attenuationMaxLabel);

    gridLayout->addLayout(distanceSliderLayout, 2, 0);
    gridLayout->addLayout(attenuationSliderLayout, 2, 2);

    // === 第四行：圆盘和数字输入 ===
    // 通信距离控制区域
    QHBoxLayout *distanceControlLayout = new QHBoxLayout();
    m_distanceDial = new QDial();
    m_distanceDial->setRange(DISTANCE_MIN * 10, DISTANCE_MAX * 10);
    m_distanceDial->setValue(m_communicationDistance * 10);
    m_distanceDial->setNotchesVisible(true);
    m_distanceDial->setNotchTarget(10.0);
    connect(m_distanceDial, &QDial::valueChanged, this, &ChannelBasicPara::onDistanceDialChanged);

    QVBoxLayout *distanceSpinBoxLayout = new QVBoxLayout();
    QLabel *distanceInputLabel = new QLabel("精确设置:");
    m_distanceSpinBox = new QDoubleSpinBox();
    m_distanceSpinBox->setRange(DISTANCE_MIN, DISTANCE_MAX);
    m_distanceSpinBox->setValue(m_communicationDistance);
    m_distanceSpinBox->setDecimals(1);
    m_distanceSpinBox->setSingleStep(0.1);
    connect(m_distanceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ChannelBasicPara::onDistanceSpinBoxChanged);

    distanceSpinBoxLayout->addWidget(distanceInputLabel);
    distanceSpinBoxLayout->addWidget(m_distanceSpinBox);
    distanceSpinBoxLayout->addStretch();

    distanceControlLayout->addWidget(m_distanceDial, 0, Qt::AlignCenter);
    distanceControlLayout->addLayout(distanceSpinBoxLayout);

    // 衰减功率控制区域
    QHBoxLayout *attenuationControlLayout = new QHBoxLayout();
    m_attenuationPowerDial = new QDial();
    m_attenuationPowerDial->setRange(ATTENUATION_POWER_MIN * 10, ATTENUATION_POWER_MAX * 10);
    m_attenuationPowerDial->setValue(m_attenuationPower * 10);
    m_attenuationPowerDial->setNotchesVisible(true);
    m_attenuationPowerDial->setNotchTarget(10.0);
    connect(m_attenuationPowerDial, &QDial::valueChanged, this, &ChannelBasicPara::onAttenuationPowerDialChanged);

    QVBoxLayout *attenuationSpinBoxLayout = new QVBoxLayout();
    QLabel *attenuationInputLabel = new QLabel("精确设置:");
    m_attenuationPowerSpinBox = new QDoubleSpinBox();
    m_attenuationPowerSpinBox->setRange(ATTENUATION_POWER_MIN, ATTENUATION_POWER_MAX);
    m_attenuationPowerSpinBox->setValue(m_attenuationPower);
    m_attenuationPowerSpinBox->setDecimals(1);
    m_attenuationPowerSpinBox->setSingleStep(0.1);
    connect(m_attenuationPowerSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ChannelBasicPara::onAttenuationPowerSpinBoxChanged);

    attenuationSpinBoxLayout->addWidget(attenuationInputLabel);
    attenuationSpinBoxLayout->addWidget(m_attenuationPowerSpinBox);
    attenuationSpinBoxLayout->addStretch();

    attenuationControlLayout->addWidget(m_attenuationPowerDial, 0, Qt::AlignCenter);
    attenuationControlLayout->addLayout(attenuationSpinBoxLayout);

    gridLayout->addLayout(distanceControlLayout, 3, 0);
    gridLayout->addLayout(attenuationControlLayout, 3, 2);

    // === 信号频率设置（只保留spinbox，放在左下角） ===
    QHBoxLayout *frequencyLayout = new QHBoxLayout();
    QLabel *frequencyLabel = new QLabel("信号频率 (MHz):");
    m_frequencySpinBox = new QDoubleSpinBox();
    m_frequencySpinBox->setRange(FREQUENCY_MIN, FREQUENCY_MAX);
    m_frequencySpinBox->setValue(m_radioParams.frequency);
    m_frequencySpinBox->setDecimals(1);
    m_frequencySpinBox->setSingleStep(0.1);
    connect(m_frequencySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ChannelBasicPara::onFrequencySpinBoxChanged);

    frequencyLayout->addWidget(frequencyLabel);
    frequencyLayout->addWidget(m_frequencySpinBox);
    frequencyLayout->addStretch();

    gridLayout->addLayout(frequencyLayout, 4, 0);

    // 布局比例
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(2, 1);
    gridLayout->setVerticalSpacing(10);
    gridLayout->setHorizontalSpacing(15);
}

// 衰减功率更新（含自动计算通信距离）
void ChannelBasicPara::updateAttenuationPower(double value)
{
    if (m_isAttenuationUpdating) return;
    m_isAttenuationUpdating = true;

    // 约束范围+保留1位小数
    double attenuation = qBound(ATTENUATION_POWER_MIN, value, ATTENUATION_POWER_MAX);
    attenuation = static_cast<double>(qRound(attenuation * 10)) / 10;

    // 更新数值和UI
    m_attenuationPower = attenuation;
    m_attenuationPowerValueLabel->setText(QString::number(attenuation, 'f', 1));

    m_attenuationPowerSlider->blockSignals(true);
    m_attenuationPowerSlider->setValue(static_cast<int>(attenuation * 10));
    m_attenuationPowerSlider->blockSignals(false);

    m_attenuationPowerDial->blockSignals(true);
    m_attenuationPowerDial->setValue(static_cast<int>(attenuation * 10));
    m_attenuationPowerDial->blockSignals(false);

    m_attenuationPowerSpinBox->blockSignals(true);
    m_attenuationPowerSpinBox->setValue(attenuation);
    m_attenuationPowerSpinBox->blockSignals(false);

    // 反向计算通信距离
    double newDistance = calculateDistanceFromAttenuation(attenuation);
    updateCommunicationDistance(newDistance, false); // 禁止回算衰减功率

    // 发送信号
    emit parametersChanged(m_attenuationPower, m_communicationDistance);
    m_isAttenuationUpdating = false;
}

// 信号频率更新（含自动计算衰减功率）
void ChannelBasicPara::updateFrequency(double value)
{
    if (m_isFrequencyUpdating) return;
    m_isFrequencyUpdating = true;

    // 约束范围+保留1位小数
    double frequency = qBound(FREQUENCY_MIN, value, FREQUENCY_MAX);
    frequency = static_cast<double>(qRound(frequency * 10)) / 10;

    // 更新数值和UI
    m_radioParams.frequency = frequency;

    m_frequencySpinBox->blockSignals(true);
    m_frequencySpinBox->setValue(frequency);
    m_frequencySpinBox->blockSignals(false);

    // 重新计算衰减功率
    double newAttenuation = calculateAttenuationFromDistance(m_communicationDistance);
    updateAttenuationPower(newAttenuation);

    m_isFrequencyUpdating = false;
}

// 通信距离更新（重载：isCalculateAtten=true时自动计算衰减功率）
void ChannelBasicPara::updateCommunicationDistance(double value, bool isCalculateAtten)
{
    if (m_isDistanceUpdating) return;
    m_isDistanceUpdating = true;

    // 约束范围+保留1位小数
    double distance = qBound(DISTANCE_MIN, value, DISTANCE_MAX);
    distance = static_cast<double>(qRound(distance * 10)) / 10;

    // 更新数值和UI
    m_communicationDistance = distance;
    m_distanceValueLabel->setText(QString::number(distance, 'f', 1));

    m_distanceSlider->blockSignals(true);
    m_distanceSlider->setValue(static_cast<int>(distance * 10));
    m_distanceSlider->blockSignals(false);

    m_distanceDial->blockSignals(true);
    m_distanceDial->setValue(static_cast<int>(distance * 10));
    m_distanceDial->blockSignals(false);

    m_distanceSpinBox->blockSignals(true);
    m_distanceSpinBox->setValue(distance);
    m_distanceSpinBox->blockSignals(false);

    // 自动计算衰减功率（仅主动修改距离时）
    if (isCalculateAtten) {
        double newAttenuation = calculateAttenuationFromDistance(distance);
        updateAttenuationPower(newAttenuation);
    }

    // 发送信号
    emit parametersChanged(m_attenuationPower, m_communicationDistance);
    m_isDistanceUpdating = false;
}

// 通信距离更新（默认：自动计算衰减功率）
void ChannelBasicPara::updateCommunicationDistance(double value)
{
    updateCommunicationDistance(value, true);
}

// 衰减功率槽函数
void ChannelBasicPara::onAttenuationPowerSliderChanged(int value)
{
    updateAttenuationPower(value / 10.0);
}

void ChannelBasicPara::onAttenuationPowerDialChanged(int value)
{
    updateAttenuationPower(value / 10.0);
}

void ChannelBasicPara::onAttenuationPowerSpinBoxChanged(double value)
{
    updateAttenuationPower(value);
}

// 通信距离槽函数
void ChannelBasicPara::onDistanceSliderChanged(int value)
{
    updateCommunicationDistance(value / 10.0);
}

void ChannelBasicPara::onDistanceDialChanged(int value)
{
    updateCommunicationDistance(value / 10.0);
}

void ChannelBasicPara::onDistanceSpinBoxChanged(double value)
{
    updateCommunicationDistance(value);
}

// 信号频率输入框更新
void ChannelBasicPara::onFrequencySpinBoxChanged(double value)
{
    updateFrequency(value);
}

// 按钮槽函数
void ChannelBasicPara::onApplyClicked()
{
    emit settingsApplied();
    emit parametersChanged(m_attenuationPower, m_communicationDistance);
}

void ChannelBasicPara::onResetClicked()
{
    resetToDefaults();
}

void ChannelBasicPara::resetToDefaults()
{
    m_isDistanceUpdating = true;
    m_isAttenuationUpdating = true;

    updateAttenuationPower(50.0);
    updateCommunicationDistance(10.0, false); // 不自动计算衰减功率
    updateFrequency(14.0); // 重置为默认频率14MHz

    m_isDistanceUpdating = false;
    m_isAttenuationUpdating = false;
}

// 计算：通信距离 → 衰减功率
double ChannelBasicPara::calculateAttenuationFromDistance(double distanceKm)
{
    // 计算自由空间路径损耗（距离单位：km，频率单位：MHz）
    double pathLoss = 32.44 + 20 * log10(distanceKm) + 20 * log10(m_radioParams.frequency);

    // 计算接收功率
    double rxPower = m_radioParams.txPower + m_radioParams.txGain + m_radioParams.rxGain
                     - pathLoss - m_radioParams.otherLoss;

    qDebug()<<"通信距离 "<<distanceKm<<"衰减功率"<<rxPower;
    return rxPower;
}

// 计算：衰减功率 → 通信距离
double ChannelBasicPara::calculateDistanceFromAttenuation(double attenuationDb)
{
    // 计算最大允许的路径损耗
    double maxPathLoss = m_radioParams.txPower + m_radioParams.txGain + m_radioParams.rxGain
                         - m_radioParams.rxSensitivity - m_radioParams.otherLoss;

    // 从路径损耗反推距离
    // Lp = 32.44 + 20log10(d) + 20log10(f)
    // 20log10(d) = Lp - 32.44 - 20log10(f)
    double term = maxPathLoss - 32.44 - 20 * log10(m_radioParams.frequency);
    double distance_km = pow(10, term / 20.0);
    qDebug()<<"衰减功率"<<attenuationDb<<"通信距离 "<<distance_km;
    return distance_km;
}
