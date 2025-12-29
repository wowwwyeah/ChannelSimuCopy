#include "pageindicator.h"
#include <QPainter>

PageIndicator::PageIndicator(int count, QWidget *parent)
    : QWidget(parent), m_count(count), m_currentIndex(0),
    m_dotSize(10), m_spacing(15),
    m_activeColor(Qt::blue), m_inactiveColor(Qt::gray)
{
    setFixedHeight(30);
}

void PageIndicator::setCurrentIndex(int index)
{
    if (index != m_currentIndex && index >= 0 && index < m_count) {
        m_currentIndex = index;
        update();
    }
}

int PageIndicator::currentIndex() const
{
    return m_currentIndex;
}

void PageIndicator::setCount(int count)
{
    if (count != m_count && count > 0) {
        m_count = count;
        if (m_currentIndex >= m_count) {
            m_currentIndex = m_count - 1;
        }
        update();
    }
}

int PageIndicator::count() const
{
    return m_count;
}

void PageIndicator::setDotSize(int size)
{
    if (size != m_dotSize && size > 0) {
        m_dotSize = size;
        update();
    }
}

int PageIndicator::dotSize() const
{
    return m_dotSize;
}

void PageIndicator::setSpacing(int spacing)
{
    if (spacing != m_spacing && spacing >= 0) {
        m_spacing = spacing;
        update();
    }
}

int PageIndicator::spacing() const
{
    return m_spacing;
}

void PageIndicator::setActiveColor(const QColor &color)
{
    m_activeColor = color;
    update();
}

QColor PageIndicator::activeColor() const
{
    return m_activeColor;
}

void PageIndicator::setInactiveColor(const QColor &color)
{
    m_inactiveColor = color;
    update();
}

QColor PageIndicator::inactiveColor() const
{
    return m_inactiveColor;
}

void PageIndicator::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int totalWidth = m_count * m_dotSize + (m_count - 1) * m_spacing;
    int startX = (width() - totalWidth) / 2;

    for (int i = 0; i < m_count; ++i) {
        if (i == m_currentIndex) {
            painter.setBrush(m_activeColor);
        } else {
            painter.setBrush(m_inactiveColor);
        }
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(startX + i * (m_dotSize + m_spacing),
                            (height() - m_dotSize) / 2,
                            m_dotSize, m_dotSize);
    }
}
