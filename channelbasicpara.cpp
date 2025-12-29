#include "channelbasicpara.h"
#include <QFrame>

ChannelBasicPara::ChannelBasicPara(QWidget *parent)
    : QWidget(parent)
    , m_noisePower(-60.0)
    , m_attenuationPower(50.0)
    , m_communicationDistance(100.0)
{
    setupUI();
}

void ChannelBasicPara::setupUI()
{
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

        /* 噪声功率滚动条样式 - 蓝色渐变 */
        QSlider#noisePowerSlider::groove:horizontal {
            border: 1px solid #5AA469;
            height: 8px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3D7248, stop:0.5 #5AA469, stop:1 #7CD68B);
            border-radius: 4px;
        }

        QSlider#noisePowerSlider::handle:horizontal {
            background: qradialgradient(cx:0.5, cy:0.5, radius:0.8,
                stop:0 #7CD68B, stop:0.8 #5AA469, stop:1 #3D7248);
            border: 2px solid #5AA469;
            width: 20px;
            margin: -8px 0;
            border-radius: 10px;
        }

        QSlider#noisePowerSlider::handle:horizontal:hover {
            background: qradialgradient(cx:0.5, cy:0.5, radius:0.8,
                stop:0 #9EF5AD, stop:0.8 #7CD68B, stop:1 #5AA469);
            border: 2px solid #7CD68B;
        }

        QSlider#noisePowerSlider::sub-page:horizontal {
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

    // 创建主水平布局来包含箭头和内容
    QHBoxLayout *outerHorizontalLayout = new QHBoxLayout();

    // 左侧箭头（静态提示）
    QLabel *leftArrow = new QLabel("◀");
    leftArrow->setAlignment(Qt::AlignCenter);
    leftArrow->setStyleSheet(
        "QLabel {"
        "   color: rgba(255, 255, 255, 80);"  // 半透明白色
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   background-color: rgba(255, 255, 255, 20);"
        "   border-radius: 15px;"
        "   padding: 5px;"
        "   margin: 10px;"
        "}"
        );
    leftArrow->setFixedSize(80, 80);
    leftArrow->setCursor(Qt::ArrowCursor);  // 普通箭头光标，不可点击
    outerHorizontalLayout->addWidget(leftArrow);

    // 中央内容区域
    QVBoxLayout *contentLayout = new QVBoxLayout();

    // 创建标题
    QLabel *titleLabel = new QLabel("参数设置");
    titleLabel->setStyleSheet(R"(
    QLabel {
        color: #CCEEEE;
        font-family: "Microsoft YaHei";
        font-size: 18px;
        font-weight: bold;
        padding: 10px;
        background-color: #225555;
        border-radius: 8px;
        qproperty-alignment: AlignCenter;
    }
)");
    //contentLayout->addWidget(titleLabel);

    // 创建功率参数组（噪声功率 + 衰减功率）
    createPowerParametersGroup();
    //createCommunicationDistanceGroup();

    contentLayout->addWidget(m_powerGroup);
    //contentLayout->addWidget(m_distanceGroup);

    // 添加按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_applyButton = new QPushButton("应用设置");
    m_resetButton = new QPushButton("重置参数");

    connect(m_applyButton, &QPushButton::clicked, this, &ChannelBasicPara::onApplyClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &ChannelBasicPara::onResetClicked);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addWidget(m_applyButton);

    //ontentLayout->addLayout(buttonLayout);

    // 将内容布局添加到外层水平布局
    outerHorizontalLayout->addLayout(contentLayout);

    // 右侧箭头（静态提示）
    QLabel *rightArrow = new QLabel("▶");
    rightArrow->setAlignment(Qt::AlignCenter);
    rightArrow->setStyleSheet(
        "QLabel {"
        "   color: rgba(255, 255, 255, 80);"  // 半透明白色
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   background-color: rgba(255, 255, 255, 20);"
        "   border-radius: 15px;"
        "   padding: 5px;"
        "   margin: 10px;"
        "}"
        );
    rightArrow->setFixedSize(80, 80);
    rightArrow->setCursor(Qt::ArrowCursor);  // 普通箭头光标，不可点击
    outerHorizontalLayout->addWidget(rightArrow);

    // 设置外层布局的拉伸因子
    outerHorizontalLayout->setStretchFactor(leftArrow, 0);        // 左箭头不拉伸
    outerHorizontalLayout->setStretchFactor(contentLayout, 1);    // 内容区域拉伸
    outerHorizontalLayout->setStretchFactor(rightArrow, 0);       // 右箭头不拉伸

    mainLayout->addLayout(outerHorizontalLayout);
}

void ChannelBasicPara::createPowerParametersGroup()
{
    m_powerGroup = new QGroupBox("功率参数设置");

    // 使用GridLayout布局
    QGridLayout *gridLayout = new QGridLayout(m_powerGroup);

    // === 第一行：标题 ===
    QLabel *noiseTitle = new QLabel("噪声功率 (dBm)");
    QLabel *attenuationTitle = new QLabel("衰减功率 (dB)");

    gridLayout->addWidget(noiseTitle, 0, 0, Qt::AlignCenter);
    gridLayout->addWidget(attenuationTitle, 0, 2, Qt::AlignCenter);

    // === 第二行：当前值显示 ===
    QLabel *noiseCurrentLabel = new QLabel("当前值:");
    m_noisePowerValueLabel = new QLabel(QString::number(m_noisePower, 'f', 1));
    m_noisePowerValueLabel->setObjectName("valueLabel");

    QLabel *attenuationCurrentLabel = new QLabel("当前值:");
    m_attenuationPowerValueLabel = new QLabel(QString::number(m_attenuationPower, 'f', 1));
    m_attenuationPowerValueLabel->setObjectName("valueLabel");

    QHBoxLayout *noiseValueLayout = new QHBoxLayout();
    noiseValueLayout->addWidget(noiseCurrentLabel);
    noiseValueLayout->addWidget(m_noisePowerValueLabel);
    noiseValueLayout->addStretch();

    QHBoxLayout *attenuationValueLayout = new QHBoxLayout();
    attenuationValueLayout->addWidget(attenuationCurrentLabel);
    attenuationValueLayout->addWidget(m_attenuationPowerValueLabel);
    attenuationValueLayout->addStretch();

    gridLayout->addLayout(noiseValueLayout, 1, 0);
    gridLayout->addLayout(attenuationValueLayout, 1, 2);

    // === 第三行：滚动条 ===
    // 噪声功率滚动条
    QHBoxLayout *noiseSliderLayout = new QHBoxLayout();
    QLabel *noiseMinLabel = new QLabel(QString::number(NOISE_POWER_MIN, 'f', 0));
    QLabel *noiseMaxLabel = new QLabel(QString::number(NOISE_POWER_MAX, 'f', 0));

    // 设置标签颜色以匹配滚动条
    noiseMinLabel->setStyleSheet("color: #7CD68B;");
    noiseMaxLabel->setStyleSheet("color: #7CD68B;");

    m_noisePowerSlider = new QSlider(Qt::Horizontal);
    m_noisePowerSlider->setObjectName("noisePowerSlider"); // 设置对象名用于样式表
    m_noisePowerSlider->setRange(NOISE_POWER_MIN * 10, NOISE_POWER_MAX * 10);
    m_noisePowerSlider->setValue(m_noisePower * 10);
    m_noisePowerSlider->setTickPosition(QSlider::TicksBelow);
    m_noisePowerSlider->setTickInterval(100);

    connect(m_noisePowerSlider, &QSlider::valueChanged, this, &ChannelBasicPara::onNoisePowerSliderChanged);

    noiseSliderLayout->addWidget(noiseMinLabel);
    noiseSliderLayout->addWidget(m_noisePowerSlider);
    noiseSliderLayout->addWidget(noiseMaxLabel);

    // 衰减功率滚动条
    QHBoxLayout *attenuationSliderLayout = new QHBoxLayout();
    QLabel *attenuationMinLabel = new QLabel(QString::number(ATTENUATION_POWER_MIN, 'f', 0));
    QLabel *attenuationMaxLabel = new QLabel(QString::number(ATTENUATION_POWER_MAX, 'f', 0));

    // 设置标签颜色以匹配滚动条
    attenuationMinLabel->setStyleSheet("color: #FFD700;");
    attenuationMaxLabel->setStyleSheet("color: #FFD700;");

    m_attenuationPowerSlider = new QSlider(Qt::Horizontal);
    m_attenuationPowerSlider->setObjectName("attenuationPowerSlider"); // 设置对象名用于样式表
    m_attenuationPowerSlider->setRange(ATTENUATION_POWER_MIN * 10, ATTENUATION_POWER_MAX * 10);
    m_attenuationPowerSlider->setValue(m_attenuationPower * 10);
    m_attenuationPowerSlider->setTickPosition(QSlider::TicksBelow);
    m_attenuationPowerSlider->setTickInterval(100);

    connect(m_attenuationPowerSlider, &QSlider::valueChanged, this, &ChannelBasicPara::onAttenuationPowerSliderChanged);

    attenuationSliderLayout->addWidget(attenuationMinLabel);
    attenuationSliderLayout->addWidget(m_attenuationPowerSlider);
    attenuationSliderLayout->addWidget(attenuationMaxLabel);

    gridLayout->addLayout(noiseSliderLayout, 2, 0);
    gridLayout->addLayout(attenuationSliderLayout, 2, 2);

    // === 第四行：圆盘和数字输入 ===
    // 噪声功率控制区域
    QHBoxLayout *noiseControlLayout = new QHBoxLayout();
    m_noisePowerDial = new QDial();
    m_noisePowerDial->setRange(NOISE_POWER_MIN * 10, NOISE_POWER_MAX * 10);
    m_noisePowerDial->setValue(m_noisePower * 10);
    m_noisePowerDial->setNotchesVisible(true);
    m_noisePowerDial->setNotchTarget(10.0);

    connect(m_noisePowerDial, &QDial::valueChanged, this, &ChannelBasicPara::onNoisePowerDialChanged);

    QVBoxLayout *noiseSpinBoxLayout = new QVBoxLayout();
    QLabel *noiseInputLabel = new QLabel("精确设置:");
    m_noisePowerSpinBox = new QDoubleSpinBox();
    m_noisePowerSpinBox->setRange(NOISE_POWER_MIN, NOISE_POWER_MAX);
    m_noisePowerSpinBox->setValue(m_noisePower);
    m_noisePowerSpinBox->setDecimals(1);
    m_noisePowerSpinBox->setSingleStep(0.1);

    connect(m_noisePowerSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ChannelBasicPara::onNoisePowerSpinBoxChanged);

    noiseSpinBoxLayout->addWidget(noiseInputLabel);
    noiseSpinBoxLayout->addWidget(m_noisePowerSpinBox);
    noiseSpinBoxLayout->addStretch();

    noiseControlLayout->addWidget(m_noisePowerDial, 0, Qt::AlignCenter);
    noiseControlLayout->addLayout(noiseSpinBoxLayout);

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

    QLabel *distanceInputLabel = new QLabel("通信距离:");
    m_distanceSpinBox = new QDoubleSpinBox();
    m_distanceSpinBox->setRange(DISTANCE_MIN, DISTANCE_MAX);
    m_distanceSpinBox->setValue(m_communicationDistance);
    m_distanceSpinBox->setDecimals(1);
    m_distanceSpinBox->setSingleStep(1.0);

    connect(m_distanceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ChannelBasicPara::onAttenuationPowerSpinBoxChanged);

    attenuationSpinBoxLayout->addWidget(attenuationInputLabel);
    attenuationSpinBoxLayout->addWidget(m_attenuationPowerSpinBox);
    attenuationSpinBoxLayout->addWidget(distanceInputLabel);
    attenuationSpinBoxLayout->addWidget(m_distanceSpinBox);
    attenuationSpinBoxLayout->addStretch();

    attenuationControlLayout->addWidget(m_attenuationPowerDial, 0, Qt::AlignCenter);
    attenuationControlLayout->addLayout(attenuationSpinBoxLayout);

    gridLayout->addLayout(noiseControlLayout, 3, 0);
    gridLayout->addLayout(attenuationControlLayout, 3, 2);

    // === 添加垂直分隔线 ===
    QFrame *verticalLine = new QFrame();
    verticalLine->setFrameShape(QFrame::VLine);
    verticalLine->setFrameShadow(QFrame::Sunken);
    verticalLine->setStyleSheet(R"(
        QFrame {
            color: #559999;
            background-color: #559999;
            border: none;
            min-width: 1px;
            max-width: 1px;
        }
    )");

    // 将分隔线添加到第1列（中间列）
    gridLayout->addWidget(verticalLine, 0, 1, 4, 1); // 跨4行，第1列

    // 设置列比例
    gridLayout->setColumnStretch(0, 1);  // 噪声功率列
    gridLayout->setColumnStretch(1, 0);  // 分隔线列（不拉伸）
    gridLayout->setColumnStretch(2, 1);  // 衰减功率列

    // 设置行间距
    gridLayout->setVerticalSpacing(10);
    gridLayout->setHorizontalSpacing(15);
}

void ChannelBasicPara::createCommunicationDistanceGroup()
{
    m_distanceGroup = new QGroupBox("通信距离设置 (米)");

    QVBoxLayout *layout = new QVBoxLayout(m_distanceGroup);

    // 值显示标签
    QHBoxLayout *valueLayout = new QHBoxLayout();
    QLabel *currentLabel = new QLabel("当前值:");
    m_distanceValueLabel = new QLabel(QString::number(m_communicationDistance, 'f', 1));
    m_distanceValueLabel->setObjectName("valueLabel");

    valueLayout->addWidget(currentLabel);
    valueLayout->addWidget(m_distanceValueLabel);
    valueLayout->addStretch();
    layout->addLayout(valueLayout);

    // 滚动条
    QHBoxLayout *sliderLayout = new QHBoxLayout();
    QLabel *minLabel = new QLabel(QString::number(DISTANCE_MIN, 'f', 0));
    QLabel *maxLabel = new QLabel(QString::number(DISTANCE_MAX, 'f', 0));

    m_distanceSlider = new QSlider(Qt::Horizontal);
    m_distanceSlider->setRange(DISTANCE_MIN, DISTANCE_MAX);
    m_distanceSlider->setValue(m_communicationDistance);
    m_distanceSlider->setTickPosition(QSlider::TicksBelow);
    m_distanceSlider->setTickInterval(100);

    connect(m_distanceSlider, &QSlider::valueChanged, this, &ChannelBasicPara::onDistanceSliderChanged);

    sliderLayout->addWidget(minLabel);
    sliderLayout->addWidget(m_distanceSlider);
    sliderLayout->addWidget(maxLabel);
    layout->addLayout(sliderLayout);

    // 圆盘和数字输入
    QHBoxLayout *controlLayout = new QHBoxLayout();

    // 圆盘
    m_distanceDial = new QDial();
    m_distanceDial->setRange(DISTANCE_MIN, DISTANCE_MAX);
    m_distanceDial->setValue(m_communicationDistance);
    m_distanceDial->setNotchesVisible(true);
    m_distanceDial->setNotchTarget(50.0);

    connect(m_distanceDial, &QDial::valueChanged, this, &ChannelBasicPara::onDistanceDialChanged);

    // 数字输入
    QVBoxLayout *spinBoxLayout = new QVBoxLayout();
    QLabel *inputLabel = new QLabel("精确设置:");
    m_distanceSpinBox = new QDoubleSpinBox();
    m_distanceSpinBox->setRange(DISTANCE_MIN, DISTANCE_MAX);
    m_distanceSpinBox->setValue(m_communicationDistance);
    m_distanceSpinBox->setDecimals(1);
    m_distanceSpinBox->setSingleStep(1.0);

    connect(m_distanceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ChannelBasicPara::onDistanceSpinBoxChanged);

    spinBoxLayout->addWidget(inputLabel);
    spinBoxLayout->addWidget(m_distanceSpinBox);
    spinBoxLayout->addStretch();

    controlLayout->addWidget(m_distanceDial, 0, Qt::AlignCenter);
    controlLayout->addLayout(spinBoxLayout);
    layout->addLayout(controlLayout);
}

// 噪声功率相关槽函数
void ChannelBasicPara::onNoisePowerSliderChanged(int value)
{
    updateNoisePower(value / 10.0);
}

void ChannelBasicPara::onNoisePowerDialChanged(int value)
{
    updateNoisePower(value / 10.0);
}

void ChannelBasicPara::onNoisePowerSpinBoxChanged(double value)
{
    updateNoisePower(value);
}

void ChannelBasicPara::updateNoisePower(double value)
{
    m_noisePower = value;
    m_noisePowerValueLabel->setText(QString::number(value, 'f', 1));

    // 更新所有控件（避免循环触发）
    m_noisePowerSlider->blockSignals(true);
    m_noisePowerSlider->setValue(value * 10);
    m_noisePowerSlider->blockSignals(false);

    m_noisePowerDial->blockSignals(true);
    m_noisePowerDial->setValue(value * 10);
    m_noisePowerDial->blockSignals(false);

    m_noisePowerSpinBox->blockSignals(true);
    m_noisePowerSpinBox->setValue(value);
    m_noisePowerSpinBox->blockSignals(false);

    // 发出参数变化信号
    emit parametersChanged(m_noisePower, m_attenuationPower, m_communicationDistance);
}

// 衰减功率相关槽函数
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

void ChannelBasicPara::updateAttenuationPower(double value)
{
    m_attenuationPower = value;
    m_attenuationPowerValueLabel->setText(QString::number(value, 'f', 1));

    m_attenuationPowerSlider->blockSignals(true);
    m_attenuationPowerSlider->setValue(value * 10);
    m_attenuationPowerSlider->blockSignals(false);

    m_attenuationPowerDial->blockSignals(true);
    m_attenuationPowerDial->setValue(value * 10);
    m_attenuationPowerDial->blockSignals(false);

    m_attenuationPowerSpinBox->blockSignals(true);
    m_attenuationPowerSpinBox->setValue(value);
    m_attenuationPowerSpinBox->blockSignals(false);

    // 发出参数变化信号
    emit parametersChanged(m_noisePower, m_attenuationPower, m_communicationDistance);
}

// 通信距离相关槽函数
void ChannelBasicPara::onDistanceSliderChanged(int value)
{
    updateCommunicationDistance(value);
}

void ChannelBasicPara::onDistanceDialChanged(int value)
{
    updateCommunicationDistance(value);
}

void ChannelBasicPara::onDistanceSpinBoxChanged(double value)
{
    updateCommunicationDistance(value);
}

void ChannelBasicPara::updateCommunicationDistance(double value)
{
    m_communicationDistance = value;
    m_distanceValueLabel->setText(QString::number(value, 'f', 1));

    m_distanceSlider->blockSignals(true);
    m_distanceSlider->setValue(value);
    m_distanceSlider->blockSignals(false);

    m_distanceDial->blockSignals(true);
    m_distanceDial->setValue(value);
    m_distanceDial->blockSignals(false);

    m_distanceSpinBox->blockSignals(true);
    m_distanceSpinBox->setValue(value);
    m_distanceSpinBox->blockSignals(false);

    // 发出参数变化信号
    emit parametersChanged(m_noisePower, m_attenuationPower, m_communicationDistance);
}

void ChannelBasicPara::onApplyClicked()
{
    emit settingsApplied();
    emit parametersChanged(m_noisePower, m_attenuationPower, m_communicationDistance);
}

void ChannelBasicPara::onResetClicked()
{
    resetToDefaults();
}

void ChannelBasicPara::resetToDefaults()
{
    updateNoisePower(-60.0);
    updateAttenuationPower(50.0);
    updateCommunicationDistance(100.0);
}
