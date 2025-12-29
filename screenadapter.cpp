// ScreenAdapter.cpp
#include "screenadapter.h"
#include <QFont>

void ScreenAdapter::initialize() {
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    // 基准设计尺寸
    const int baseWidth = 1920;
    const int baseHeight = 1080;

    // 计算缩放因子
    qreal scaleX = (qreal)screenGeometry.width() / baseWidth;
    qreal scaleY = (qreal)screenGeometry.height() / baseHeight;
    m_scaleFactor = qMin(scaleX, scaleY);

    // 限制缩放范围
    m_scaleFactor = qBound(0.5, m_scaleFactor, 3.0);

    // 设置全局字体
    QFont font = qApp->font();
    font.setPointSizeF(font.pointSizeF() * m_scaleFactor);
    qApp->setFont(font);
}
