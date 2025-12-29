#ifndef PAGEINDICATOR_H
#define PAGEINDICATOR_H

#include <QWidget>

class PageIndicator : public QWidget
{
    Q_OBJECT
public:
    explicit PageIndicator(int count, QWidget *parent = nullptr);

    void setCurrentIndex(int index);
    int currentIndex() const;

    void setCount(int count);
    int count() const;

    void setDotSize(int size);
    int dotSize() const;

    void setSpacing(int spacing);
    int spacing() const;

    void setActiveColor(const QColor &color);
    QColor activeColor() const;

    void setInactiveColor(const QColor &color);
    QColor inactiveColor() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_count;
    int m_currentIndex;
    int m_dotSize;
    int m_spacing;
    QColor m_activeColor;
    QColor m_inactiveColor;
};

#endif // PAGEINDICATOR_H
