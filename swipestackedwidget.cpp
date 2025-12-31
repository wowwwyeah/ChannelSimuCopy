#include "swipestackedwidget.h"
#include <QPushButton>
#include <QToolButton>
#include <QSlider>
#include <QScrollBar>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDebug>

SwipeStackedWidget::SwipeStackedWidget(QWidget *parent)
    : QStackedWidget(parent)
    , m_swipeEnabled(true)
    , m_animationDuration(300)
    , m_swipeThreshold(20)
    , m_edgeMargin(30)
    , m_swipeDistance(0)
    , m_targetIndex(-1)
    , m_swipeVelocity(0)
    , m_touchOnInteractiveWidget(false)
    , m_showEdgeAreas(false)
    , m_showSwipeIndicator(true)
    , m_edgeAreaColor(QColor(0, 100, 200, 50))
    , m_indicatorColor(QColor(0, 100, 200, 150))
    , m_circularEnabled(true)
{
    // 初始化枚举成员变量
    m_swipeState = SwipeState::Idle;
    m_swipeDirection = SwipeDirection::None;

    m_animation = new QPropertyAnimation(this, "swipeDistance");
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->setDuration(m_animationDuration);

    connect(m_animation, &QPropertyAnimation::finished,
            this, &SwipeStackedWidget::onAnimationFinished);

    // 启用触摸跟踪
    setAttribute(Qt::WA_AcceptTouchEvents);
}

SwipeStackedWidget::~SwipeStackedWidget()
{
    delete m_animation;
}

void SwipeStackedWidget::enableSwipe(bool enable)
{
    m_swipeEnabled = enable;
}

bool SwipeStackedWidget::isSwipeEnabled() const
{
    return m_swipeEnabled;
}

void SwipeStackedWidget::setAnimationDuration(int duration)
{
    m_animationDuration = duration;
    m_animation->setDuration(duration);
}

int SwipeStackedWidget::animationDuration() const
{
    return m_animationDuration;
}

void SwipeStackedWidget::setSwipeThreshold(int threshold)
{
    m_swipeThreshold = threshold;
}

int SwipeStackedWidget::swipeThreshold() const
{
    return m_swipeThreshold;
}

void SwipeStackedWidget::setEdgeMargin(int margin)
{
    m_edgeMargin = margin;
}

int SwipeStackedWidget::edgeMargin() const
{
    return m_edgeMargin;
}

void SwipeStackedWidget::setCurrentIndexWithAnimation(int index)
{
    if (index == currentIndex() || index < 0 || index >= count()) {
        return;
    }

    m_targetIndex = index;
    animateToIndex(index);
}

void SwipeStackedWidget::nextPageWithAnimation()
{
    if (count() <= 1) return;

    int nextIndex = getCircularNextIndex();
    setCurrentIndexWithAnimation(nextIndex);
}

void SwipeStackedWidget::previousPageWithAnimation()
{
    if (count() <= 1) return;

    int prevIndex = getCircularPrevIndex();
    setCurrentIndexWithAnimation(prevIndex);
}

int SwipeStackedWidget::swipeDistance() const
{
    return m_swipeDistance;
}

void SwipeStackedWidget::setSwipeDistance(int distance)
{
    if (m_swipeDistance != distance) {
        m_swipeDistance = distance;
        update();
        emit swipeDistanceChanged(distance);
        emit swipeProgressChanged(calculateSwipeProgress());
    }
}

bool SwipeStackedWidget::event(QEvent *event)
{
    if (!m_swipeEnabled || m_swipeState == SwipeState::Animating) {
        return QStackedWidget::event(event);
    }

    switch (event->type()) {
    case QEvent::TouchBegin:
        return handleTouchBegin(static_cast<QTouchEvent*>(event));

    case QEvent::TouchUpdate:
        return handleTouchUpdate(static_cast<QTouchEvent*>(event));

    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        return handleTouchEnd(static_cast<QTouchEvent*>(event));

    default:
        break;
    }

    return QStackedWidget::event(event);
}

void SwipeStackedWidget::paintEvent(QPaintEvent *event)
{
    QStackedWidget::paintEvent(event);

    if (m_showEdgeAreas) {
        QPainter painter(this);
        drawEdgeAreas(&painter);
    }

    if (m_showSwipeIndicator && m_swipeState == SwipeState::Swiping) {
        QPainter painter(this);
        drawSwipeIndicator(&painter);
    }
}

void SwipeStackedWidget::resizeEvent(QResizeEvent *event)
{
    QStackedWidget::resizeEvent(event);
    // 重置滑动状态
    if (m_swipeState == SwipeState::Swiping) {
        cancelSwipe();
    }
}

void SwipeStackedWidget::onAnimationFinished()
{
    if (m_targetIndex != -1 && m_targetIndex != currentIndex()) {
        QStackedWidget::setCurrentIndex(m_targetIndex);
        emit pageChanged(m_targetIndex);
    }

    setSwipeDistance(0);
    m_swipeState = SwipeState::Idle;
    m_targetIndex = -1;
    m_swipeDirection = SwipeDirection::None;
    emit swipeFinished();
}

// 兼容不同Qt版本的触摸点获取函数
QPointF SwipeStackedWidget::getTouchPoint(QTouchEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6 使用 points()
    if (!event->points().isEmpty()) {
        return event->points().first().position();
    }
#else
    // Qt5 使用 touchPoints()
    if (!event->touchPoints().isEmpty()) {
        return event->touchPoints().first().pos();
    }
#endif
    return QPointF();
}

bool SwipeStackedWidget::handleTouchBegin(QTouchEvent *event)
{
    QPointF touchPos = getTouchPoint(event);
    if (touchPos.isNull()) {
        return false;
    }

    m_startPos = touchPos.toPoint();
    m_currentPos = m_startPos;
    m_velocityStartPos = m_startPos;
    m_velocityTimer.start();
    m_swipeVelocity = 0;

    // 检查是否在交互控件上
    if (isInteractiveWidget(m_startPos)) {
        m_touchOnInteractiveWidget = true;
        return false; // 让交互控件处理事件
    }

    m_touchOnInteractiveWidget = false;
    m_swipeState = SwipeState::Tracking;
    return true;
}

bool SwipeStackedWidget::handleTouchUpdate(QTouchEvent *event)
{
    if (m_touchOnInteractiveWidget) {
        return false;
    }

    QPointF touchPos = getTouchPoint(event);
    if (touchPos.isNull()) {
        return false;
    }

    m_currentPos = touchPos.toPoint();

    int deltaX = m_currentPos.x() - m_startPos.x();

    // 计算速度
    qreal elapsed = m_velocityTimer.elapsed();
    if (elapsed > 0) {
        int deltaVelocityX = m_currentPos.x() - m_velocityStartPos.x();
        m_swipeVelocity = deltaVelocityX / elapsed;
        if (elapsed > 50) { // 每50ms更新一次速度参考点
            m_velocityStartPos = m_currentPos;
            m_velocityTimer.restart();
        }
    }

    switch (m_swipeState) {
    case SwipeState::Tracking:
        if (qAbs(deltaX) > m_swipeThreshold) {
            startSwipe();
        }
        break;

    case SwipeState::Swiping:
        updateSwipe();
        break;

    default:
        break;
    }

    return m_swipeState == SwipeState::Swiping;
}

bool SwipeStackedWidget::handleTouchEnd(QTouchEvent *event)
{
    Q_UNUSED(event)

    if (m_touchOnInteractiveWidget) {
        m_touchOnInteractiveWidget = false;
        m_swipeState = SwipeState::Idle;
        return false;
    }

    if (m_swipeState == SwipeState::Swiping) {
        completeSwipe();
    } else {
        cancelSwipe();
    }

    return true;
}

bool SwipeStackedWidget::isInteractiveWidget(const QPoint &pos) const
{
    QWidget *child = childAt(pos);
    while (child && child != this) {
        // 检查是否是各种交互控件
        if (qobject_cast<QAbstractButton*>(child) ||
            qobject_cast<QSlider*>(child) ||
            qobject_cast<QScrollBar*>(child) ||
            qobject_cast<QLineEdit*>(child) ||
            qobject_cast<QTextEdit*>(child) ||
            qobject_cast<QComboBox*>(child)) {
            return true;
        }

        // 检查控件是否可交互
        if (child->focusPolicy() != Qt::NoFocus ||
            child->testAttribute(Qt::WA_Hover)) {
            return true;
        }

        child = child->parentWidget();
    }

    return false;
}

bool SwipeStackedWidget::isInEdgeArea(const QPoint &pos) const
{
    return pos.x() < m_edgeMargin || pos.x() > width() - m_edgeMargin;
}

void SwipeStackedWidget::startSwipe()
{
    m_swipeState = SwipeState::Swiping;
    m_swipeDirection = (m_currentPos.x() - m_startPos.x() > 0) ?
                           SwipeDirection::Right : SwipeDirection::Left;
    emit swipeStarted();
    update();
}

void SwipeStackedWidget::updateSwipe()
{
    int deltaX = m_currentPos.x() - m_startPos.x();

    // 限制滑动范围
    int maxSwipeDistance = width() / 2;
    if (qAbs(deltaX) > maxSwipeDistance) {
        deltaX = (deltaX > 0) ? maxSwipeDistance : -maxSwipeDistance;
    }

    setSwipeDistance(deltaX);
    updateSwipePreview();
}

void SwipeStackedWidget::completeSwipe()
{
    if (shouldCompleteSwipe()) {
        m_targetIndex = calculateTargetIndex();
        animateToIndex(m_targetIndex);
    } else {
        animateReturn();
    }

    m_swipeState = SwipeState::Animating;
}

void SwipeStackedWidget::cancelSwipe()
{
    if (m_swipeState == SwipeState::Swiping) {
        animateReturn();
        m_swipeState = SwipeState::Animating;
    } else {
        m_swipeState = SwipeState::Idle;
        setSwipeDistance(0);
    }
}

void SwipeStackedWidget::animateToIndex(int index)
{
    int current = currentIndex();
    int total = count();

    if (total <= 1) return;

    int direction;

    // 计算滑动方向和距离（考虑循环）
    if (m_circularEnabled) {
        // 计算最短路径（正向或反向循环）
        int forwardDistance = (index - current + total) % total;
        int backwardDistance = (current - index + total) % total;

        if (forwardDistance <= backwardDistance) {
            direction = (index > current) ? -1 : 1;
        } else {
            direction = (index < current) ? 1 : -1;
        }
    } else {
        direction = (index > current) ? -1 : 1;
    }

    int targetDistance = direction * width();

    m_animation->setStartValue(m_swipeDistance);
    m_animation->setEndValue(targetDistance);
    m_animation->start();
}

void SwipeStackedWidget::animateReturn()
{
    m_animation->setStartValue(m_swipeDistance);
    m_animation->setEndValue(0);
    m_animation->start();
}

void SwipeStackedWidget::updateSwipePreview()
{
    // 可以在这里实现滑动时的预览效果
    // 例如改变下一页的透明度或缩放
}

int SwipeStackedWidget::calculateTargetIndex() const
{
    int current = currentIndex();
    int total = count();

    if (total <= 1) return current;

    int direction = (m_swipeDistance > 0) ? -1 : 1;
    int target = current + direction;

    // 如果启用循环滑动
    if (m_circularEnabled && shouldUseCircularNavigation()) {
        // 循环处理边界
        if (target < 0) {
            target = total - 1;  // 从第一页向左滑动到最后一页
        } else if (target >= total) {
            target = 0;  // 从最后一页向右滑动到第一页
        }
    } else {
        // 非循环模式，保持原有边界检查
        if (target < 0) target = 0;
        if (target >= total) target = total - 1;
    }

    return target;
}

qreal SwipeStackedWidget::calculateSwipeProgress() const
{
    if (m_swipeDistance == 0) return 0.0;

    qreal progress = qAbs(m_swipeDistance) / qreal(width());
    return qMin(progress, 1.0);
}

// 添加循环滑动辅助函数
int SwipeStackedWidget::getCircularNextIndex() const
{
    int current = currentIndex();
    int total = count();

    if (total <= 1) return current;

    if (m_circularEnabled) {
        return (current + 1) % total;  // 循环到下一个
    } else {
        return qMin(current + 1, total - 1);  // 不循环，限制在范围内
    }
}

int SwipeStackedWidget::getCircularPrevIndex() const
{
    int current = currentIndex();
    int total = count();

    if (total <= 1) return current;

    if (m_circularEnabled) {
        return (current - 1 + total) % total;  // 循环到上一个
    } else {
        return qMax(current - 1, 0);  // 不循环，限制在范围内
    }
}

bool SwipeStackedWidget::shouldUseCircularNavigation() const
{
    return m_circularEnabled && count() > 1;
}

bool SwipeStackedWidget::shouldCompleteSwipe() const
{
    // 基于距离和速度判断是否完成滑动
    qreal progress = calculateSwipeProgress();
    bool byDistance = progress > 0.3; // 30%阈值
    bool byVelocity = qAbs(m_swipeVelocity) > 0.5; // 速度阈值

    return byDistance || byVelocity;
}

void SwipeStackedWidget::drawSwipeIndicator(QPainter *painter)
{
    if (m_swipeDistance == 0) return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 绘制滑动进度条
    int indicatorHeight = 4;
    int indicatorY = height() - indicatorHeight - 10;

    QRect backgroundRect(10, indicatorY, width() - 20, indicatorHeight);
    painter->setBrush(QColor(200, 200, 200, 100));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(backgroundRect, 2, 2);

    qreal progress = calculateSwipeProgress();
    int progressWidth = (width() - 20) * progress;

    QRect progressRect(10, indicatorY, progressWidth, indicatorHeight);
    painter->setBrush(m_indicatorColor);
    painter->drawRoundedRect(progressRect, 2, 2);

    painter->restore();
}

void SwipeStackedWidget::drawEdgeAreas(QPainter *painter)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 绘制左侧边缘区域
    QRect leftRect(0, 0, m_edgeMargin, height());
    painter->fillRect(leftRect, m_edgeAreaColor);

    // 绘制右侧边缘区域
    QRect rightRect(width() - m_edgeMargin, 0, m_edgeMargin, height());
    painter->fillRect(rightRect, m_edgeAreaColor);

    painter->restore();
}
