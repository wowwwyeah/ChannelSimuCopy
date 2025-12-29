// swipestackedwidget.cpp
#include "swipestackedwidget.h"
#include <QPropertyAnimation>

SwipeStackedWidget::SwipeStackedWidget(QWidget *parent)
    : QStackedWidget(parent)
    , m_animationDuration(300)
    , m_targetIndex(-1)
    , m_circularEnabled(true)
{
    m_animation = new QPropertyAnimation(this, "currentIndex");
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->setDuration(m_animationDuration);

    connect(m_animation, &QPropertyAnimation::finished,
            this, &SwipeStackedWidget::onAnimationFinished);
}

SwipeStackedWidget::~SwipeStackedWidget()
{
    delete m_animation;
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

void SwipeStackedWidget::setCircularEnabled(bool enabled)
{
    m_circularEnabled = enabled;
}

bool SwipeStackedWidget::isCircularEnabled() const
{
    return m_circularEnabled;
}

void SwipeStackedWidget::onAnimationFinished()
{
    if (m_targetIndex != -1) {
        emit pageChanged(m_targetIndex);
        m_targetIndex = -1;
    }
}

void SwipeStackedWidget::animateToIndex(int index)
{
    m_animation->setStartValue(currentIndex());
    m_animation->setEndValue(index);
    m_animation->start();
}

int SwipeStackedWidget::getCircularNextIndex() const
{
    int current = currentIndex();
    int total = count();

    if (total <= 1) return current;

    if (m_circularEnabled) {
        return (current + 1) % total;
    } else {
        return qMin(current + 1, total - 1);
    }
}

int SwipeStackedWidget::getCircularPrevIndex() const
{
    int current = currentIndex();
    int total = count();

    if (total <= 1) return current;

    if (m_circularEnabled) {
        return (current - 1 + total) % total;
    } else {
        return qMax(current - 1, 0);
    }
}
