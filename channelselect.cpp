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
    title->setStyleSheet("color: #2196F3; font-family: 微软雅黑; font-size: 18px; font-weight: bold; margin: 8px;");
    layout->addWidget(title);

    // 创建水平布局来包含箭头和表格
    QHBoxLayout *horizontalLayout = new QHBoxLayout();

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

    horizontalLayout->addWidget(leftArrow);

    // 表格控件
    m_matrixWidget = new MatrixWidget();
    horizontalLayout->addWidget(m_matrixWidget);

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

    horizontalLayout->addWidget(rightArrow);

    layout->addLayout(horizontalLayout);

    // 设置布局拉伸因子
    layout->setStretchFactor(title, 0);           // 标签不拉伸
    layout->setStretchFactor(horizontalLayout, 1); // 水平布局拉伸

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

