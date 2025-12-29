#include "screensaver.h"
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QScreen>
#include <QPainter>
#include <QTouchEvent>
#include <QtWidgets>

// =============================================================================
// ScreenSaver 类实现
// =============================================================================

ScreenSaver::ScreenSaver(QObject *parent, int idleTimeoutMs)
    : QObject(parent),
    m_idleTimer(new QTimer(this)),
    m_blankWindow(nullptr),
    m_screenOff(false),
    m_idleTimeoutMs(idleTimeoutMs),
    m_isGesturing(false),
    m_unlockSuccessful(false),
    m_thumbPosition(0.0),
    m_showUnlockUI(false) // 初始化为不显示
{
    qDebug() << "ScreenSaver initialized with timeout:" << m_idleTimeoutMs / 1000 << "seconds.";
    m_idleTimer->setSingleShot(true);
    connect(m_idleTimer, &QTimer::timeout, this, &ScreenSaver::turnOffScreen);

    // 初始化解锁UI的隐藏计时器
    m_unlockUITimer = new QTimer(this);
    m_unlockUITimer->setSingleShot(true);
    m_unlockUITimer->setInterval(3000); // 3秒后隐藏
    connect(m_unlockUITimer, &QTimer::timeout, this, &ScreenSaver::hideUnlockUI);

    initBlankWindow();
    m_idleTimer->start(m_idleTimeoutMs);
}

ScreenSaver::~ScreenSaver()
{
    if (m_screenOff) {
        turnOnScreen();
    }
    delete m_blankWindow;
    qDebug() << "ScreenSaver destroyed.";
}

void ScreenSaver::resetTimer()
{
    if (m_screenOff) {
        return;
    }
    if (m_idleTimer->isActive()) {
        m_idleTimer->stop();
    }
    m_idleTimer->start(m_idleTimeoutMs);
}

void ScreenSaver::turnOffScreen()
{
    if (!m_blankWindow) {
        qWarning() << "Blank window not initialized. Cannot turn off screen.";
        return;
    }
    // 重置所有状态
    m_unlockSuccessful = false;
    m_thumbPosition = 0.0;
    m_showUnlockUI = false; // 初始不显示UI
    m_isGesturing = false;        // 重置手势状态

    m_blankWindow->raise();
    m_blankWindow->showFullScreen();
    m_blankWindow->setCursor(Qt::BlankCursor);
    m_screenOff = true;
    qDebug() << "Screen saver activated: Screen locked.";
}

void ScreenSaver::turnOnScreen()
{
    if (!m_blankWindow || !m_screenOff) {
        return;
    }
    m_blankWindow->hide();
    m_blankWindow->setCursor(Qt::ArrowCursor);
    m_screenOff = false;
    m_idleTimer->start(m_idleTimeoutMs);
    qDebug() << "Screen saver deactivated: Screen unlocked.";
}

// 新增：隐藏解锁UI
void ScreenSaver::hideUnlockUI()
{
    if (m_screenOff && !m_isGesturing && !m_unlockSuccessful) {
        m_showUnlockUI = false;
        m_blankWindow->update(); // 触发重绘，清除UI
        qDebug() << "Unlock UI hidden due to inactivity.";
    }
}

void ScreenSaver::initBlankWindow()
{
    if (m_blankWindow) {
        delete m_blankWindow;
    }
    m_blankWindow = new QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_blankWindow->setAttribute(Qt::WA_TranslucentBackground, false);
    m_blankWindow->setStyleSheet("background-color: black;");

    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    m_blankWindow->setGeometry(screenGeometry);

    int sliderHeight = 80;
    int sliderWidth = screenGeometry.width() * 0.8;
    int thumbRadius = sliderHeight / 2 + 10;
    m_sliderRect = QRectF(
        (screenGeometry.width() - sliderWidth) / 2,
        screenGeometry.height() - sliderHeight * 2,
        sliderWidth,
        sliderHeight / 4
        );
    m_thumbRect = QRectF(0, 0, thumbRadius * 2, thumbRadius * 2);

    m_blankWindow->installEventFilter(this);
    m_blankWindow->setMouseTracking(true);
    qDebug() << "Blank window created with unlock UI.";
}

// =============================================================================
// 手势检测和界面绘制
// =============================================================================
bool ScreenSaver::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != m_blankWindow || !m_screenOff) {
        return QObject::eventFilter(obj, event);
    }

    if (m_unlockSuccessful) {
        return true;
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::TouchBegin: {
        QPoint pos;
        if (event->type() == QEvent::TouchBegin) {
            pos = static_cast<QTouchEvent*>(event)->touchPoints().first().pos().toPoint();
        } else {
            pos = static_cast<QMouseEvent*>(event)->pos();
        }

        m_showUnlockUI = true; // 显示解锁UI
        m_blankWindow->update(); // 触发重绘

        // 检查是否点击在滑块上
        QPointF thumbCenter(m_sliderRect.left() + m_thumbPosition * m_sliderRect.width(), m_sliderRect.center().y());
        if (QLineF(thumbCenter, pos).length() < m_thumbRect.width() / 2) {
            startGesture(pos);
        }
        // 重启UI隐藏计时器
        m_unlockUITimer->start();
        return true;
    }
    case QEvent::MouseMove:
    case QEvent::TouchUpdate: {
        if (!m_isGesturing) break;

        QPoint pos;
        if (event->type() == QEvent::TouchUpdate) {
            pos = static_cast<QTouchEvent*>(event)->touchPoints().first().pos().toPoint();
        } else {
            pos = static_cast<QMouseEvent*>(event)->pos();
        }
        updateGesture(pos);
        // 重启UI隐藏计时器
        m_unlockUITimer->start();
        return true;
    }
    case QEvent::MouseButtonRelease:
    case QEvent::TouchEnd:
        endGesture();
        // 操作结束后，重启UI隐藏计时器
        m_unlockUITimer->start();
        return true;
    case QEvent::Paint: {
        QPainter painter(m_blankWindow);
        painter.fillRect(m_blankWindow->rect(), Qt::black); // 确保背景是黑色

        // 只有当 m_showUnlockUI 为 true 时才绘制UI
        if (m_showUnlockUI) {
            paintUnlockUI(painter);
        }
        return true;
    }
    default:
        break;
    }
    return QObject::eventFilter(obj, event);
}

void ScreenSaver::startGesture(const QPoint &pos)
{
    m_isGesturing = true;
    m_gestureStartPos = pos;
    m_gestureCurrentPos = pos;
    qDebug() << "Gesture started at:" << pos;
}

void ScreenSaver::updateGesture(const QPoint &pos)
{
    if (!m_isGesturing) return;

    m_gestureCurrentPos = pos;

    qreal delta = m_gestureCurrentPos.x() - m_gestureStartPos.x();
    qreal newPosition = m_thumbPosition + (delta / m_sliderRect.width());
    m_thumbPosition = qBound(0.0, newPosition, 1.0);

    if (m_thumbPosition > 0.9) {
        m_unlockSuccessful = true;
        m_blankWindow->update();
        QTimer::singleShot(300, this, &ScreenSaver::turnOnScreen);
    } else {
        m_blankWindow->update();
    }
}

void ScreenSaver::endGesture()
{
    if (!m_unlockSuccessful) {
        m_thumbPosition = 0.0;
        m_blankWindow->update();
    }
    m_isGesturing = false;
    m_gestureStartPos = QPoint();
    m_gestureCurrentPos = QPoint();
    qDebug() << "Gesture ended. Unlock successful:" << m_unlockSuccessful;
}

void ScreenSaver::paintUnlockUI(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRectF sliderBgRect = m_sliderRect.adjusted(-20, -20, 20, 20);
    QColor bgColor(50, 50, 50, 200);
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(sliderBgRect, sliderBgRect.height()/2, sliderBgRect.height()/2);

    QColor trackColor(100, 100, 100, 200);
    painter.setBrush(trackColor);
    painter.drawRoundedRect(m_sliderRect, m_sliderRect.height()/2, m_sliderRect.height()/2);

    if (m_thumbPosition > 0.0) {
        QRectF filledRect = m_sliderRect;
        filledRect.setWidth(m_sliderRect.width() * m_thumbPosition);
        QColor fillColor = m_unlockSuccessful ? QColor(0, 255, 0, 200) : QColor(150, 150, 150, 200);
        painter.setBrush(fillColor);
        painter.drawRoundedRect(filledRect, filledRect.height()/2, filledRect.height()/2);
    }

    QPointF thumbCenter(m_sliderRect.left() + m_thumbPosition * m_sliderRect.width(), m_sliderRect.center().y());
    m_thumbRect.moveCenter(thumbCenter);

    QColor thumbColor = m_unlockSuccessful ? Qt::green : Qt::white;
    painter.setBrush(thumbColor);
    painter.setPen(QColor(200, 200, 200));
    painter.drawEllipse(m_thumbRect);

    if (!m_unlockSuccessful) {
        QString text = "滑动解锁";
        QFont font("Microsoft YaHei", 24, QFont::Bold);
        painter.setFont(font);
        painter.setPen(Qt::white);
        QRectF textRect(0, m_sliderRect.top() - 80, m_blankWindow->width(), 60);
        painter.drawText(textRect, Qt::AlignCenter, text);
    } else {
        QString text = "解锁成功!";
        QFont font("Microsoft YaHei", 24, QFont::Bold);
        painter.setFont(font);
        painter.setPen(Qt::green);
        QRectF textRect(0, m_sliderRect.top() - 80, m_blankWindow->width(), 60);
        painter.drawText(textRect, Qt::AlignCenter, text);
    }
}

// =============================================================================
// GlobalEventFilter 类实现
// =============================================================================

GlobalEventFilter::GlobalEventFilter(ScreenSaver *saver, QObject *parent)
    : QObject(parent), m_screensaver(saver)
{
}

bool GlobalEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_screensaver) {
        return QObject::eventFilter(obj, event);
    }

    if (!m_screensaver->isScreenOff()) {
        switch (event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::TouchBegin:
        case QEvent::KeyPress:
            m_screensaver->resetTimer();
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(obj, event);
}

void ScreenSaver::testTurnOffScreen()
{
    turnOffScreen();
}
