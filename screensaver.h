#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <QObject>
#include <QTimer>
#include <QWidget>
#include <QPoint>
#include <QColor>

class ScreenSaver : public QObject
{
    Q_OBJECT

public:
    explicit ScreenSaver(QObject *parent = nullptr, int idleTimeoutMs = 5 * 60 * 1000);
    ~ScreenSaver();
    void resetTimer();
    void testTurnOffScreen();
    bool isScreenOff() const { return m_screenOff; }

private slots:
    void turnOffScreen();
    void turnOnScreen();
    // 新增：用于隐藏解锁UI的计时器超时槽函数
    void hideUnlockUI();

private:
    void initBlankWindow();
    bool eventFilter(QObject *obj, QEvent *event) override;
    void startGesture(const QPoint& pos);
    void updateGesture(const QPoint& pos);
    void endGesture();
    void paintUnlockUI(QPainter &painter);

    QTimer *m_idleTimer;
    QWidget *m_blankWindow;
    bool m_screenOff;
    int m_idleTimeoutMs;

    bool m_isGesturing;
    QPoint m_gestureStartPos;
    QPoint m_gestureCurrentPos;
    bool m_unlockSuccessful;

    QRectF m_sliderRect;
    QRectF m_thumbRect;
    qreal m_thumbPosition;

    // 新增：控制解锁UI显示/隐藏的变量和计时器
    bool m_showUnlockUI;
    QTimer *m_unlockUITimer;
};

class GlobalEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit GlobalEventFilter(ScreenSaver *saver, QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    ScreenSaver *m_screensaver;
};

#endif // SCREENSAVER_H
