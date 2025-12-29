#include "parasetting.h"
#include <QLabel>
#include <QVBoxLayout>

ParaSetting::ParaSetting(QWidget *parent)
    : QWidget{parent}
{
    initUI();
}

void ParaSetting::initUI()
{
    // 设置背景色
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#336666"));
    setPalette(pal);
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("参数设置");
    title->setAlignment(Qt::AlignmentFlag::AlignCenter);
    title->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");
    layout->addWidget(title);

#if 1
    QWidget *nosieContainer = new QWidget(this);
    nosieContainer->setObjectName("nosieContainer");
    QVBoxLayout *noiseContainerLayout = new QVBoxLayout(nosieContainer);
    noiseContainerLayout->setContentsMargins(6, 0, 6, 0); // 表格左右边距
    noiseContainerLayout->setSpacing(0);

    // 创建Label
    QLabel *label1 = new QLabel("噪声功率", this);
    label1->setMinimumWidth(80);

    // 创建SpinBox
    m_noisePowerSB = new QSpinBox(this);
    m_noisePowerSB->setRange(0, 100);
    m_noisePowerSB->setValue(10);
    m_noisePowerSB->setMinimumWidth(150);

    noiseContainerLayout->addWidget(label1);
    noiseContainerLayout->addWidget(m_noisePowerSB);

    QWidget *powerContainer = new QWidget(this);
    powerContainer->setObjectName("powerContainer");
    QVBoxLayout *powerContainerLayout = new QVBoxLayout(powerContainer);
    powerContainerLayout->setContentsMargins(6, 0, 6, 0); // 表格左右边距
    powerContainerLayout->setSpacing(0);

    // 创建Label
    QLabel *label2 = new QLabel("衰减功率", this);
    label2->setMinimumWidth(80);

    // 创建SpinBox
    m_antPowerSB = new QSpinBox(this);
    m_antPowerSB->setRange(0, 100);
    m_antPowerSB->setValue(10);
    m_antPowerSB->setMinimumWidth(150);

    noiseContainerLayout->addWidget(label2);
    noiseContainerLayout->addWidget(m_antPowerSB);

    QWidget *distanceContainer = new QWidget(this);
    distanceContainer->setObjectName("distanceContainer");
    QVBoxLayout *distanceContainerLayout = new QVBoxLayout(distanceContainer);
    distanceContainerLayout->setContentsMargins(6, 0, 6, 0); // 表格左右边距
    distanceContainerLayout->setSpacing(0);
    // 创建Label
    QLabel *label3 = new QLabel("通信距离", this);
    label2->setMinimumWidth(80);

    // 创建SpinBox
    m_distanceSB = new QSpinBox(this);
    m_distanceSB->setRange(0, 100);
    m_distanceSB->setValue(10);
    m_distanceSB->setMinimumWidth(150);

    noiseContainerLayout->addWidget(label3);
    noiseContainerLayout->addWidget(m_distanceSB);

    layout->addWidget(nosieContainer);
    layout->addWidget(powerContainer);
    layout->addWidget(distanceContainer);

#endif
    layout->addStretch();
    setLayout(layout);
}
