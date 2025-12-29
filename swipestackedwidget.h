// swipestackedwidget.h
#ifndef SWIPESTACKEDWIDGET_H
#define SWIPESTACKEDWIDGET_H

#include <QStackedWidget>

class SwipeStackedWidget : public QStackedWidget
{
    Q_OBJECT

public:
    explicit SwipeStackedWidget(QWidget *parent = nullptr);
    ~SwipeStackedWidget();

    // 编程控制切换（带动画）
    void setCurrentIndexWithAnimation(int index);
    void nextPageWithAnimation();
    void previousPageWithAnimation();

    // 设置循环滑动
    void setCircularEnabled(bool enabled);
    bool isCircularEnabled() const;

signals:
    void pageChanged(int index);

private slots:
    void onAnimationFinished();

private:
    // 动画相关
    class QPropertyAnimation *m_animation;
    int m_animationDuration;
    int m_targetIndex;

    // 循环滑动
    bool m_circularEnabled;

    // 动画控制
    void animateToIndex(int index);

    // 循环滑动辅助函数
    int getCircularNextIndex() const;
    int getCircularPrevIndex() const;
};

#endif // SWIPESTACKEDWIDGET_H
