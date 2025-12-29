#ifndef SWIPESTACKEDWIDGET_H
#define SWIPESTACKEDWIDGET_H

#include <QTouchEvent>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QPainter>
#include <QElapsedTimer>

class SwipeStackedWidget : public QStackedWidget
{
    Q_OBJECT
    Q_PROPERTY(int swipeDistance READ swipeDistance WRITE setSwipeDistance NOTIFY swipeDistanceChanged)

public:
    explicit SwipeStackedWidget(QWidget *parent = nullptr);
    ~SwipeStackedWidget();

    // 滑动功能控制
    void enableSwipe(bool enable);
    bool isSwipeEnabled() const;

    // 动画设置
    void setAnimationDuration(int duration);
    int animationDuration() const;

    // 滑动参数设置
    void setSwipeThreshold(int threshold);
    int swipeThreshold() const;

    void setEdgeMargin(int margin);
    int edgeMargin() const;

    // 编程控制切换（带动画）
    void setCurrentIndexWithAnimation(int index);
    void nextPageWithAnimation();
    void previousPageWithAnimation();

    // 属性访问
    int swipeDistance() const;
    void setSwipeDistance(int distance);

signals:
    void swipeStarted();
    void swipeFinished();
    void pageChanged(int index);
    void swipeDistanceChanged(int distance);
    void swipeProgressChanged(qreal progress);

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onAnimationFinished();

private:
    enum class SwipeDirection { None, Left, Right };
    enum class SwipeState { Idle, Tracking, Swiping, Animating };

    // 触摸事件处理
    bool handleTouchBegin(QTouchEvent *event);
    bool handleTouchUpdate(QTouchEvent *event);
    bool handleTouchEnd(QTouchEvent *event);

    // 兼容不同Qt版本的触摸点获取
    QPointF getTouchPoint(QTouchEvent *event);

    // 交互控件检测
    bool isInteractiveWidget(const QPoint &pos) const;
    bool isInEdgeArea(const QPoint &pos) const;

    // 滑动逻辑
    void startSwipe();
    void updateSwipe();
    void completeSwipe();
    void cancelSwipe();

    // 动画控制
    void animateToIndex(int index);
    void animateReturn();
    void updateSwipePreview();

    // 工具函数
    int calculateTargetIndex() const;
    qreal calculateSwipeProgress() const;
    bool shouldCompleteSwipe() const;

    // 绘制函数
    void drawSwipeIndicator(QPainter *painter);
    void drawEdgeAreas(QPainter *painter);

    // 成员变量
    QPropertyAnimation *m_animation;
    bool m_swipeEnabled;
    SwipeState m_swipeState;  // 添加这行
    int m_animationDuration;
    int m_swipeThreshold;
    int m_edgeMargin;
    int m_swipeDistance;
    int m_targetIndex;
    SwipeDirection m_swipeDirection;

    // 触摸相关
    QPoint m_startPos;
    QPoint m_currentPos;
    QPoint m_velocityStartPos;
    QElapsedTimer m_velocityTimer;
    qreal m_swipeVelocity;
    bool m_touchOnInteractiveWidget;

    // 视觉效果
    bool m_showEdgeAreas;
    bool m_showSwipeIndicator;
    QColor m_edgeAreaColor;
    QColor m_indicatorColor;

    // 循环滑动相关
    bool m_circularEnabled;

    // 添加新的循环滑动函数
    int getCircularNextIndex() const;
    int getCircularPrevIndex() const;
    bool shouldUseCircularNavigation() const;
};

#endif // SWIPESTACKEDWIDGET_H
