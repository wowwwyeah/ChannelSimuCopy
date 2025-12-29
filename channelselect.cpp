#include "channelselect.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

ChannelSelect::ChannelSelect(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}


void ChannelSelect::initUI()
{
    // 设置背景色
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#336666"));
    setPalette(pal);
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("通道选择");
    title->setAlignment(Qt::AlignmentFlag::AlignCenter);
    title->setStyleSheet("color: #2196F3; font-family: 微软雅黑; font-size: 22px; font-weight: bold; margin: 12px;");  // 18px -> 22px, 8px -> 12px
    layout->addWidget(title);

    // 表格控件
    m_matrixWidget = new MatrixWidget();
    layout->addWidget(m_matrixWidget);

    // 设置布局拉伸因子
    layout->setStretchFactor(title, 0);           // 标签不拉伸
    layout->setStretchFactor(m_matrixWidget, 1); // 水平布局拉伸

    layout->addStretch();
    setLayout(layout);
}

int ChannelSelect::getSimuChannel()
{
    return m_matrixWidget->getSelectedChannel();
}

int ChannelSelect::getSwitchChannel()
{
    return m_matrixWidget->getPressedChannel();
}

