#ifndef MATRIXWIDGET_H
#define MATRIXWIDGET_H

#include <QTableWidget>
#include <QTimer>
#include <QTouchEvent>
#include <QHeaderView>

class MatrixWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit MatrixWidget(QWidget *parent = nullptr);
    ~MatrixWidget();

    int getSelectedChannel();
    int getPressedChannel();
    bool isUpperTriangle(int row, int col) const;

public slots:
    void changeCellColor(int row, int column);

protected:
    bool event(QEvent *event) override;

private slots:
    void onLongPressTimeout();

private:
    void initHeaders();
    void initCells();
    void handleTouchEvent(QTouchEvent *event);
    void handleTouchClick(int row, int column);
    void handleTouchLongPress(int row, int column);
    void createRaisedCellWidget(int row, int column, QTableWidgetItem *item);
    void removeCellWidget(int row, int column);

    QTimer *m_longPressTimer;

    // 凸起效果相关
    int m_highlightedRow;
    int m_highlightedCol;
    int m_selectedChannel;

    // 触摸事件相关
    QTableWidgetItem *m_pressedItem;
    QPoint m_touchStartPos;
    bool m_isLongPressTriggered;
    int m_pressedChannel;
};

#endif // MATRIXWIDGET_H
